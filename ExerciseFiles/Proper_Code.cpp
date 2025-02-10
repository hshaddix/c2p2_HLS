// Sandbox compiling thoughts: 
// 1) Adjust stream depths 
// 2) Explore fixed point arithmetic (ap_fixed)
// 3) Interchange BRAM/AXI in INTERFACE pragmas 

#include <ap_int.h>
#include <ap_fixed.h> // Using fixed-point arithmetic for resource optimization
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <iostream>  // For debug prints
#include <bitset>    // For binary output

#define POSITION_BITS 13
#define SIZE_BITS 3
#define ABCStar_SIZE 256

#define HEADER_SIZE 3    // Header lines (3x 64-bit)
#define FOOTER_SIZE 3    // Footer lines (3x 64-bit)
#define MAX_DATA_SIZE 100

// Define the structure for Hit data
struct Hit {
    ap_uint<POSITION_BITS> position;
    ap_uint<SIZE_BITS> size;
    bool last;  // Flag to mark the last cluster before the footer
};

// Change data types to 64-bit for input and output
typedef ap_uint<64> InputData64;  // For 64-bit input data
typedef ap_uint<64> OutputData64; // For 64-bit output data
typedef ap_uint<16> InputData;    // 16-bit data for processing
typedef Hit OutputData;           // The output after processing clusters

// Using fixed-point arithmetic to optimize DSP utilization
typedef ap_fixed<16, 8> fixed16_t;

// Function declarations
extern "C" {
    void processHits(ap_uint<64>* in, ap_uint<64>* out, unsigned int vSize);
}

bool areAdjacent(const Hit &first_hit, const Hit &second_hit, bool &error);
bool get_LAST_bit(ap_uint<64> stripLine);
static void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize);

// Checks if two hits are adjacent considering the size of the cluster
bool areAdjacent(ap_uint<16> first_hit, ap_uint<16> second_hit, bool &error) {
    ap_uint<12> first_position = first_hit.range(14, 3);
    ap_uint<12> second_position = second_hit.range(14, 3);
    ap_uint<3> first_size = first_hit.range(2, 0);

    if (first_position + first_size > second_position) {
        error = true;  // Clusters overlap, flag error
        return false;
    }
    error = false;
    return (first_position + first_size == second_position);
}

// Function to get the LAST bit from a 64-bit strip line
bool get_LAST_bit(ap_uint<64> stripLine) {
    return stripLine[63] || stripLine[31];
}

// Outputs an error message to the stream
void outputError(hls::stream<ap_uint<64>> &outputStream) {
    ap_uint<64> errorCluster = 0xFFFFFFFFFFFFFFFF;
    outputStream << errorCluster;
#ifdef SW_EMU
    std::cout << "[ERROR] Error detected. Sending error cluster: 0xFFFFFFFFFFFFFFFF" << std::endl;
#endif
}

// Process clusters with adjacency-based clustering and bitmask handling
static void processCluster(hls::stream<ap_uint<64>> &outputStream, hls::stream<ap_uint<32>>& processStream) {

    // Create a stream for processing hits
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");
    #pragma HLS STREAM variable=packingOutStream depth=4

    // Loop over the stream
    while (!processStream.empty()) {
        #pragma HLS PIPELINE II=2
        ap_uint<32> fullClusterWord = processStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);

        ap_uint<16> first_hit = cluster;
        ap_uint<16> third_hit = 0;

        ap_uint<12> position = cluster.range(14, 3);
        ap_uint<3> bitmask = cluster.range(2, 0);

        switch (bitmask.to_uint()) {
            case 0: first_hit.range(2, 0) = 1; break;
            case 1: first_hit.range(2, 0) = 1; third_hit = first_hit; third_hit.range(14, 3) = position + 3; third_hit.range(2, 0) = 1; break;
            case 2: first_hit.range(2, 0) = 1; third_hit = first_hit; third_hit.range(14, 3) = position + 2; third_hit.range(2, 0) = 1; break;
            case 3: first_hit.range(2, 0) = 1; third_hit = first_hit; third_hit.range(14, 3) = position + 2; third_hit.range(2, 0) = 2; break;
            case 4: first_hit.range(2, 0) = 2; break;
            case 5: first_hit.range(2, 0) = 2; third_hit = first_hit; third_hit.range(14, 3) = position + 3; third_hit.range(2, 0) = 1; break;
            case 6: first_hit.range(2, 0) = 3; break;
            case 7: first_hit.range(2, 0) = 4; break;
        }

        ap_uint<32> outputFirst = 0;
        outputFirst.range(31, 16) = first_hit;
        outputFirst.range(15, 0) = spareID;
        packingOutStream << outputFirst;

        if (third_hit.range(2, 0) != 0) {
            ap_uint<32> outputThird = 0;
            outputThird.range(31, 16) = third_hit;
            outputThird.range(15, 0) = spareID;
            packingOutStream << outputThird;
        }
    }

    while (!packingOutStream.empty()) {
        ap_uint<32> fullClusterWord = packingOutStream.read();
        outputStream << fullClusterWord;
    }
}

// Read data from global memory and process
static void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize) {
    for (int i = 0; i < HEADER_SIZE; i++) {
        #pragma HLS UNROLL factor=2
        ap_uint<64> header = in[i];
        outputStream << header;
    }

    hls::stream<ap_uint<32>> processStream("processStream");
    #pragma HLS STREAM variable=processStream depth=8

    unsigned int i = HEADER_SIZE;
    while (i < vSize - FOOTER_SIZE) {
        ap_uint<64> moduleHeader = in[i++];
        outputStream << moduleHeader;

        while (i < vSize - FOOTER_SIZE) {
            #pragma HLS PIPELINE II=2
            ap_uint<64> stripLine = in[i];
            bool lastBit = get_LAST_bit(stripLine);

            ap_uint<32> cluster1 = stripLine.range(63, 32);
            ap_uint<32> cluster2 = stripLine.range(31, 0);

            if (cluster1 != 0) processStream << cluster1;
            if (cluster2 != 0) processStream << cluster2;
            i++;

            if (lastBit) {
                processCluster(outputStream, processStream);
                break;
            }
        }
    }

    for (int i = 0; i < FOOTER_SIZE; i++) {
        #pragma HLS UNROLL factor=2
        ap_uint<64> footer = in[vSize - FOOTER_SIZE + i];
        outputStream << footer;
    }
}

// Kernel Function with BRAM interface instead of AXI
extern "C" {
    void processHits(ap_uint<64>* in, ap_uint<64>* out, unsigned int vSize) {
        #pragma HLS INTERFACE bram port=in
        #pragma HLS INTERFACE bram port=out
        #pragma HLS INTERFACE s_axilite port=in bundle=control
        #pragma HLS INTERFACE s_axilite port=out bundle=control
        #pragma HLS INTERFACE s_axilite port=vSize bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        hls::stream<ap_uint<64>> outStream("outputStream");
        #pragma HLS STREAM variable=outStream depth=16

        read_input(in, outStream, vSize);

        while (!outStream.empty()) {
            ap_uint<64> result = outStream.read();
            *out++ = result;
        }
    }
}
