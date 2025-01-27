/// Correct Read Input with INLINE, Unroll, and Pipeline pragmas 

void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize) {
    #pragma HLS INLINE off
    #pragma HLS PIPELINE II=1

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
        ap_uint<64> footer = in[vSize - FOOTER_SIZE + i];
        outputStream << footer;
    }
}

/// Explanation of read_input pragmas and reasoning for them 
// #pragma HLS PIPELINE: Reduces the loop initiation interval (II) to 1, allowing new iterations to start every clock cycle, significantly improving throughput.
// #pragma HLS UNROLL: Processes multiple loop iterations simultaneously, increasing parallelism and reducing latency for small, independent tasks.
// #pragma HLS INLINE: Removes function call overhead by integrating the function directly into the calling context, minimizing latency. 

/// Processhits function with stream variable and pipeline pragmas 
static void processCluster(hls::stream<ap_uint<64>>& outputStream, hls::stream<ap_uint<32>>& processStream) {
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");
    #pragma HLS STREAM variable=packingOutStream depth=6

    while (!processStream.empty()) {
        #pragma HLS PIPELINE II=1
        ap_uint<32> fullClusterWord = processStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);

        // Handle bitmask logic here

        // Push the result into packingOutStream
        packingOutStream << fullClusterWord;
    }

    // Further processing...
}

/// Explanation of processCluster pragmas 
// #pragma HLS STREAM: Increases stream buffer depth, preventing data stalls and ensuring smooth data flow between functions
// #pragma HLS PIPELINE: Enables concurrent processing of loop iterations, allowing one cluster to be processed per clock cycle, reducing overall latency