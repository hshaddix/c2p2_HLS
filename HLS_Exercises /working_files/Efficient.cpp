/// Correct Read Input with Unroll, STREAM and PIPELINE pragmas 

#include "processHits.h"
#include <iostream>  // For debug prints
#include <bitset>    // For binary output

#define BITS_64 64

// Function to get the LAST bit from a 64-bit strip line
bool get_LAST_bit(ap_uint<64> stripLine) {
    return stripLine[63] || stripLine[31];  // Check both first bits of upper and lower 32 bits
}

// Process clusters with adjacency-based clustering and bitmask handling
static void processCluster(hls::stream<ap_uint<64>> &outputStream, hls::stream<ap_uint<32>> &processStream) {
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");
    #pragma HLS STREAM variable=packingOutStream depth=6

    while (!processStream.empty()) {
        #pragma HLS PIPELINE II=1
        ap_uint<32> fullClusterWord = processStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);

        // Processing logic here...

        packingOutStream << fullClusterWord;
    }

    while (!packingOutStream.empty()) {
        #pragma HLS PIPELINE II=1
        ap_uint<32> fullClusterWord = packingOutStream.read();
        outputStream << fullClusterWord;
    }
}

// Read data from global memory and process
void read_input(ap_uint<64> *in, hls::stream<ap_uint<64>> &outputStream, unsigned int vSize) {
    for (int i = 0; i < HEADER_SIZE; i++) {
        #pragma HLS UNROLL
        ap_uint<64> header = in[i];
        outputStream << header;
    }

    hls::stream<ap_uint<32>> processStream("processStream");
    #pragma HLS STREAM variable=processStream depth=6

    unsigned int i = HEADER_SIZE;
    while (i < vSize - FOOTER_SIZE) {
        ap_uint<64> moduleHeader = in[i++];
        outputStream << moduleHeader;

        while (i < vSize - FOOTER_SIZE) {
            #pragma HLS PIPELINE II=1
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
        #pragma HLS UNROLL
        ap_uint<64> footer = in[vSize - FOOTER_SIZE + i];
        outputStream << footer;
    }
}


/// Explanation of processCluster pragmas 
// #pragma HLS STREAM: Increases stream buffer depth, preventing data stalls and ensuring smooth data flow between functions
// #pragma HLS PIPELINE: Enables concurrent processing of loop iterations, allowing one cluster to be processed per clock cycle, reducing overall latency
