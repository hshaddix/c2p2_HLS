// Sandbox compiling thoughts: 
// 1) Adjust stream depths 
// 2) explore fixed point arithmetic (ap_fixed)
// 4) Replacing AXI with bram in INTERFACE pragmas 

// Header portion of code // 

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

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

// Function declarations
extern "C" {
    void processHits(ap_uint<64>* in, ap_uint<64>* out, unsigned int vSize);
}

bool areAdjacent(const Hit &first_hit, const Hit &second_hit, bool &error);  // Adjacent function
bool get_LAST_bit(ap_uint<64> stripLine);  // Helper function to extract the LAST bit from the strip line

// Function for reading input data from global memory
static void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize);

// End of Header portion of code // 

#include <iostream>  // For debug prints
#include <bitset>    // For binary output

#define BITS_64 64

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
    return (first_position + first_size == second_position);  // Clusters are exactly adjacent
}

// Function to get the LAST bit from a 64-bit strip line
bool get_LAST_bit(ap_uint<64> stripLine) {
    return stripLine[63] || stripLine[31];  // Check both first bits of upper and lower 32 bits
}

// Outputs an error message to the stream
void outputError(hls::stream<ap_uint<64>> &outputStream) {
    ap_uint<64> errorCluster = 0xFFFFFFFFFFFFFFFF;  // Indicate error with special value (all bits set)
    outputStream << errorCluster;
#ifdef SW_EMU
    std::cout << "[ERROR] Error detected. Sending error cluster: 0xFFFFFFFFFFFFFFFF" << std::endl;
#endif
}

// Process clusters with adjacency-based clustering and bitmask handling
static void processCluster(hls::stream<ap_uint<64>> &outputStream, hls::stream<ap_uint<32>>& processStream) {

    // Create a stream for processing hits
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");
    //// Insert directive here ////  

    // Loop over the stream
    while(!processStream.empty())
    {
        ap_uint<32> fullClusterWord = processStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);
    
        ap_uint<16> first_hit = cluster;
        ap_uint<16> third_hit = 0;

        ap_uint<12> position = cluster.range(14, 3);
        ap_uint<3> bitmask = cluster.range(2, 0);

    #ifdef SW_EMU
        std::cout << "[PROCESSING CLUSTER] Input Cluster = " << std::hex << cluster 
                << ", SPARE/ID = " << std::hex << spareID 
                << ", Position = " << position 
                << ", Bitmask = " << bitmask << std::endl;
    #endif

        // Determine size based on bitmask and adjust for potential third hit
        switch (bitmask.to_uint()) {
            case 0: //000
                first_hit.range(2, 0) = 1;  // 1 hit
                break;
            case 1: //001
                first_hit.range(2, 0) = 1;
                third_hit = first_hit;
                third_hit.range(14, 3) = position + 3;
                third_hit.range(2, 0) = 1;
                break;
            case 2:// 010
                first_hit.range(2, 0) = 1;
                third_hit = first_hit;
                third_hit.range(14, 3) = position + 2;
                third_hit.range(2, 0) = 1;
                break;
            case 3: //011
                first_hit.range(2, 0) = 1;
                third_hit = first_hit;
                third_hit.range(14, 3) = position + 2;
                third_hit.range(2, 0) = 2;
                break;
            case 4: //100
                first_hit.range(2, 0) = 2;
                break;
            case 5: //101
                first_hit.range(2, 0) = 2;
                third_hit = first_hit;
                third_hit.range(14, 3) = position + 3;
                third_hit.range(2, 0) = 1;
                break;
            case 6: // 110
                first_hit.range(2, 0) = 3;
                break;
            case 7: // 111`
                first_hit.range(2, 0) = 4;
                break;
            default:
                break;
        }

        // output the first one
        ap_uint<32> outputFirst = 0;
        outputFirst.range(31, 16) =  first_hit;
        outputFirst.range(15, 0) =  spareID;
        packingOutStream << outputFirst;

        if (third_hit.range(2, 0) != 0) {
            ap_uint<32> outputThird = 0;
            outputThird.range(31, 16) =  third_hit;
            outputThird.range(15, 0) =  spareID;
            packingOutStream << outputThird;
        }
    }

    // output the stream and pack into 64 stream
    bool isFirst32Filled = false;
    ap_uint<64> outWord = 0;
    while(!packingOutStream.empty())
    {
        ap_uint<32> fullClusterWord = packingOutStream.read();

        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);
    
        ap_uint<12> position = cluster.range(14, 3);

        // unroll the position
        ap_uint<4> chipID = position.range(11, 8);
        ap_uint<8> stripID = position.range(7, 0);
        ap_uint<12> unrolledStripID = 0;
        if(stripID >= 128) unrolledStripID = (stripID-128) + chipID*128;
        else unrolledStripID = (stripID) + chipID*128;


        // Extract and encode the output fields
        ap_uint<1> last = (packingOutStream.empty());
        ap_uint<1> row = (stripID >= 128) ? 1 : 0;
        ap_uint<2> nStrips = cluster.range(2, 0);
        ap_uint<12> stripIndex = unrolledStripID;
        ap_uint<32> encodedCluster = ((ap_uint<64>)last << 31) |
                                    ((ap_uint<64>)row << 30) |
                                    ((ap_uint<64>)nStrips << 28) |
                                    ((ap_uint<64>)stripIndex << 16) |
                                    ((ap_uint<64>)spareID << 0);
                                    // ((ap_uint<64>)0 << 0);

        #ifdef SW_EMU
                std::cout << "[OUTPUT CLUSTER] Encoded Data = " << std::hex << encodedCluster << std::endl;
                std::cout << "FIELDS:" << std::endl;
                std::cout << "LAST = " << std::hex << last 
                        << ", ROW = " << std::hex << row 
                        << ", NSTRIPS = " << std::hex << nStrips 
                        << ", STRIP_INDEX = " << std::hex << stripIndex 
                        << ", SPARE = " << std::hex << spareID << std::endl;
        #endif

        // Fill the upper half first
        if(!isFirst32Filled)
        {
            outWord.range(63, 32) = encodedCluster;
            isFirst32Filled = true;
        }
        else
        {
            // move the cluster down, and add the other output, and then clean it
            outWord.range(31, 0) = outWord.range(63, 32);
            outWord.range(63, 32) = encodedCluster;
            // output the cluster
            outputStream << outWord;

            // clean the state
            isFirst32Filled = false;
            isFirst32Filled = 0;
        }
    }
    // output the final cluster 
    if(outWord) outputStream << outWord;

}

// Read data from global memory and process
void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize) {
#ifdef SW_EMU
    std::cout << "[INPUT] Reading and Writing Header" << std::endl;
#endif

    for (int i = 0; i < HEADER_SIZE; i++) {
        ap_uint<64> header = in[i];
        outputStream << header;
#ifdef SW_EMU
        std::cout << "[HEADER] Header[" << std::dec << i << "] = " << std::hex << header << std::endl;
#endif
    }

    // Create a stream for processing hits
    hls::stream<ap_uint<32>> processStream("processStream");
    //// Insert directive here ////

    unsigned int i = HEADER_SIZE;
    while (i < vSize - FOOTER_SIZE) {
        ap_uint<64> moduleHeader = in[i++];
        outputStream << moduleHeader;
#ifdef SW_EMU
        std::cout << "-------------------------------------------------------------" << std::endl;
        std::cout << "[MODULE HEADER] Module Header = " << std::hex << moduleHeader << std::endl;
#endif

        while (i < vSize - FOOTER_SIZE) {
            ap_uint<64> stripLine = in[i];
            bool lastBit = get_LAST_bit(stripLine);

            ap_uint<32> cluster1 = stripLine.range(63, 32);
            ap_uint<32> cluster2 = stripLine.range(31, 0);

#ifdef SW_EMU
            std::cout << "[CLUSTER] Cluster 1 = " << std::hex << cluster1 << std::endl;
            std::cout << "[CLUSTER] Cluster 2 = " << std::hex << cluster2 << std::endl;
#endif

            // Process each cluster individually
            if(cluster1 != 0) processStream << cluster1; 
            if(cluster2 != 0) processStream << cluster2; 
            i++;

            if (lastBit) {
#ifdef SW_EMU
                std::cout << "[LAST BIT] Module boundary detected. Processing the module." << std::endl;
#endif
                processCluster(outputStream, processStream);
#ifdef SW_EMU
                std::cout << "-------------------------------------------------------------" << std::endl;
#endif

                break;
            }
        }
    }

#ifdef SW_EMU
    std::cout << "[INPUT] Reading and Writing Footer" << std::endl;
#endif
    for (int i = 0; i < FOOTER_SIZE; i++) {
        ap_uint<64> footer = in[vSize - FOOTER_SIZE + i];
        outputStream << footer;
#ifdef SW_EMU
        std::cout << "[FOOTER] Footer[" << std::dec << i << "] = " << std::hex << footer << std::endl;
#endif
    }
}

extern "C" {
    void processHits(ap_uint<64>* in, ap_uint<64>* out, unsigned int vSize) {
        #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=in bundle=control
        #pragma HLS INTERFACE s_axilite port=out bundle=control
        #pragma HLS INTERFACE s_axilite port=vSize bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        hls::stream<ap_uint<64>> outStream("outputStream");

        #pragma HLS STREAM variable=outStream depth=64

        read_input(in, outStream, vSize);

        while (!outStream.empty()) {
            ap_uint<64> result = outStream.read();
            *out++ = result;
        }
    }
}
