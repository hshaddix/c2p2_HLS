#ifndef PROCESS_HITS_H
#define PROCESS_HITS_H

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

#endif // PROCESS_HITS_H
