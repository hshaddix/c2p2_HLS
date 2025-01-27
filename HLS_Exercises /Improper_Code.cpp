// Sandbox compiling thoughts: 
// 1) Adjust stream depths 
// 2) Changing/removing INLINE pragmas from areAdjacent and get_LAST_bit
// 3) explore fixed point arithmetic (ap_fixed)
// 4) Replacing AXI with bram in INTERFACE pragmas 


#include "processHits.h"
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
static void processCluster(hls::stream<ap_uint<64>> &outputStream, hls::stream<ap_uint<32>> &processStream) {
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");

    while (!processStream.empty()) {
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

        switch (bitmask.to_uint()) {
            case 0: first_hit.range(2, 0) = 1; break;  // 1 hit
            case 1: first_hit.range(2, 0) = 1;
                    third_hit = first_hit;
                    third_hit.range(14, 3) = position + 3;
                    third_hit.range(2, 0) = 1;
                    break;
            case 2: first_hit.range(2, 0) = 1;
                    third_hit = first_hit;
                    third_hit.range(14, 3) = position + 2;
                    third_hit.range(2, 0) = 1;
                    break;
            case 3: first_hit.range(2, 0) = 1;
                    third_hit = first_hit;
                    third_hit.range(14, 3) = position + 2;
                    third_hit.range(2, 0) = 2;
                    break;
            case 4: first_hit.range(2, 0) = 2; break;
            case 5: first_hit.range(2, 0) = 2;
                    third_hit = first_hit;
                    third_hit.range(14, 3) = position + 3;
                    third_hit.range(2, 0) = 1;
                    break;
            case 6: first_hit.range(2, 0) = 3; break;
            case 7: first_hit.range(2, 0) = 4; break;
            default: break;
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

    bool isFirst32Filled = false;
    ap_uint<64> outWord = 0;

    while (!packingOutStream.empty()) {
        ap_uint<32> fullClusterWord = packingOutStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);

        ap_uint<12> position = cluster.range(14, 3);
        ap_uint<4> chipID = position.range(11, 8);
        ap_uint<8> stripID = position.range(7, 0);
        ap_uint<12> unrolledStripID = (stripID >= 128) ? (stripID - 128) + chipID * 128 : stripID + chipID * 128;

        ap_uint<1> last = packingOutStream.empty();
        ap_uint<1> row = (stripID >= 128) ? 1 : 0;
        ap_uint<2> nStrips = cluster.range(2, 0);
        ap_uint<12> stripIndex = unrolledStripID;

        ap_uint<32> encodedCluster = ((ap_uint<64>)last << 31) |
                                      ((ap_uint<64>)row << 30) |
                                      ((ap_uint<64>)nStrips << 28) |
                                      ((ap_uint<64>)stripIndex << 16) |
                                      ((ap_uint<64>)spareID << 0);

        if (!isFirst32Filled) {
            outWord.range(63, 32) = encodedCluster;
            isFirst32Filled = true;
        } else {
            outWord.range(31, 0) = outWord.range(63, 32);
            outWord.range(63, 32) = encodedCluster;
            outputStream << outWord;
            isFirst32Filled = false;
        }
    }

    if (outWord) outputStream << outWord;
}

// Read data from global memory and process
void read_input(ap_uint<64> *in, hls::stream<ap_uint<64>> &outputStream, unsigned int vSize) {
    for (int i = 0; i < HEADER_SIZE; i++) {
        ap_uint<64> header = in[i];
        outputStream << header;
    }

    hls::stream<ap_uint<32>> processStream("processStream");

    unsigned int i = HEADER_SIZE;
    while (i < vSize - FOOTER_SIZE) {
        ap_uint<64> moduleHeader = in[i++];
        outputStream << moduleHeader;

        while (i < vSize - FOOTER_SIZE) {
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
        ap_uint<64> footer = in[vSize - FOOTER_SIZE + i];
        outputStream << footer;
    }
}

extern "C" {
    void processHits(ap_uint<64> *in, ap_uint<64> *out, unsigned int vSize) {
        #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=in bundle=control
        #pragma HLS INTERFACE s_axilite port=out bundle=control
        #pragma HLS INTERFACE s_axilite port=vSize bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        hls::stream<ap_uint<64>> outStream("outputStream");

        read_input(in, outStream, vSize);

        while (!outStream.empty()) {
            ap_uint<64> result = outStream.read();
            *out++ = result;
        }
    }
}
