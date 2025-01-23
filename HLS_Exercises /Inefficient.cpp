// Read input function with removed pragmas (Unroll, INLINE, Pipeline)

void read_input(ap_uint<64>* in, hls::stream<ap_uint<64>>& outputStream, unsigned int vSize) {
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

// Process Cluster Function 

static void processCluster(hls::stream<ap_uint<64>>& outputStream, hls::stream<ap_uint<32>>& processStream) {
    hls::stream<ap_uint<32>> packingOutStream("packingOutStream");

    while (!processStream.empty()) {
        ap_uint<32> fullClusterWord = processStream.read();
        ap_uint<16> cluster = fullClusterWord.range(31, 16);
        ap_uint<16> spareID = fullClusterWord.range(15, 0);

        // Handle bitmask logic here

        // Push the result into packingOutStream
        packingOutStream << fullClusterWord;
    }

    // Further processing...
}




