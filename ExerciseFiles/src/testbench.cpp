#include <iostream>
#include <ap_int.h>
#include <hls_stream.h>
#include "processHits.h"
#include <bitset>

#ifndef __SYNTHESIS__
// Testbench
int main() {
    // Declare the output stream to hold cluster information
    hls::stream<OutputData> outputClustersStream;

    // Define inputs for each case
    int id1 = 0, position1 = 50;
    unsigned char bitmask1 = 0b111;

    int id2 = 0, position2a = 50, position2b = 54;
    unsigned char bitmask2a = 0b110, bitmask2b = 0b100;

    int id3 = 0, position3 = 50;
    unsigned char bitmask3 = 0b011;

    int id4a = 0, position4a = 50;
    int id4b = 0, position4b = 55;
    unsigned char bitmask4a = 0b101, bitmask4b = 0b001;

    int id5a = 0, position5a = 252;
    int id5b = 1, position5b = 0;
    unsigned char bitmask5a = 0b111, bitmask5b = 0b100;

    // Case 1: ID = 0, Position = 50, Bitmask = 111
    {
        hls::stream<InputData> testDataStream;
        std::cout << "Case 1 Input: ABCStarID = " << id1 << ", Position = " << position1 << ", Bitmask = " << std::bitset<3>(bitmask1) << std::endl;
        unsigned short data1 = (id1 << 11) | (position1 << 3) | bitmask1;
        InputData inputDataCase1 = {data1, false}; // Not the last data
        testDataStream.write(inputDataCase1);

        processHits(testDataStream, outputClustersStream);

        std::cout << "Case 1 Results:" << std::endl;
        while (!outputClustersStream.empty()) {
            OutputData output = outputClustersStream.read();
            std::cout << "Cluster: Position = " << (output.data >> SIZE_BITS)
                    << ", Size = " << (output.data & ((1 << SIZE_BITS) - 1))
                    << ", Last = " << output.last << std::endl;
        }
    }

    // Case 2: Two clusters on the same ABCStar with positions 50 and 54
    {
        hls::stream<InputData> testDataStream;
        std::cout << "Case 2 Input: \n";
        std::cout << "    ABCStarID = " << id2 << ", Position = " << position2a << ", Bitmask = " << std::bitset<3>(bitmask2a) << std::endl;
        std::cout << "    ABCStarID = " << id2 << ", Position = " << position2b << ", Bitmask = " << std::bitset<3>(bitmask2b) << std::endl;
        InputData inputData1 = {(id2 << 11) | (position2a << 3) | bitmask2a, false}; // Not the last data
        InputData inputData2 = {(id2 << 11) | (position2b << 3) | bitmask2b, true};  // Last data
        testDataStream.write(inputData1);
        testDataStream.write(inputData2);

        processHits(testDataStream, outputClustersStream);

        std::cout << "Case 2 Results:" << std::endl;
        while (!outputClustersStream.empty()) {
            OutputData output = outputClustersStream.read();
            std::cout << "Cluster: Position = " << (output.data >> SIZE_BITS)
                    << ", Size = " << (output.data & ((1 << SIZE_BITS) - 1))
                    << ", Last = " << output.last << std::endl;
        }
    }

    // Case 3: ID = 0, Position = 50, Bitmask = 011
    {
        hls::stream<InputData> testDataStream;
        std::cout << "Case 3 Input: ABCStarID = " << id3 << ", Position = " << position3 << ", Bitmask = " << std::bitset<3>(bitmask3) << std::endl;
        unsigned short data3 = (id3 << 11) | (position3 << 3) | bitmask3;
        InputData inputDataCase3 = {data3, false}; // Not the last data
        testDataStream.write(inputDataCase3);

        processHits(testDataStream, outputClustersStream);

        std::cout << "Case 3 Results:" << std::endl;
        while (!outputClustersStream.empty()) {
            OutputData output = outputClustersStream.read();
            std::cout << "Cluster: Position = " << (output.data >> SIZE_BITS)
                    << ", Size = " << (output.data & ((1 << SIZE_BITS) - 1))
                    << ", Last = " << output.last << std::endl;
        }
    }

    // Case 4: ID = 0, Position = 50, Bitmask = 101 and ID = 0, Position = 55, Bitmask = 001
    {
        hls::stream<InputData> testDataStream;
        std::cout << "Case 4 Input: \n";
        std::cout << "    ABCStarID = " << id4a << ", Position = " << position4a << ", Bitmask = " << std::bitset<3>(bitmask4a) << std::endl;
        std::cout << "    ABCStarID = " << id4b << ", Position = " << position4b << ", Bitmask = " << std::bitset<3>(bitmask4b) << std::endl;
        InputData inputData1 = {(id4a << 11) | (position4a << 3) | bitmask4a, false}; // Not the last data
        InputData inputData2 = {(id4b << 11) | (position4b << 3) | bitmask4b, true};  // Last data
        testDataStream.write(inputData1);
        testDataStream.write(inputData2);

        processHits(testDataStream, outputClustersStream);

        std::cout << "Case 4 Results:" << std::endl;
        while (!outputClustersStream.empty()) {
            OutputData output = outputClustersStream.read();
            std::cout << "Cluster: Position = " << (output.data >> SIZE_BITS)
                    << ", Size = " << (output.data & ((1 << SIZE_BITS) - 1))
                    << ", Last = " << output.last << std::endl;
        }
    }

    // Case 5: ID = 0, Position = 252, Bitmask = 111 and ID = 1, Position = 0, Bitmask = 100
    {
        hls::stream<InputData> testDataStream;
        std::cout << "Case 5 Input: \n";
        std::cout << "    ABCStarID = " << id5a << ", Position = " << position5a << ", Bitmask = " << std::bitset<3>(bitmask5a) << std::endl;
        std::cout << "    ABCStarID = " << id5b << ", Position = " << position5b << ", Bitmask = " << std::bitset<3>(bitmask5b) << std::endl;
        InputData inputData1 = {(id5a << 11) | (position5a << 3) | bitmask5a, false}; // Not the last data
        InputData inputData2 = {(id5b << 11) | (position5b << 3) | bitmask5b, true};  // Last data
        testDataStream.write(inputData1);
        testDataStream.write(inputData2);

        processHits(testDataStream, outputClustersStream);

        std::cout << "Case 5 Results:" << std::endl;
        while (!outputClustersStream.empty()) {
            OutputData output = outputClustersStream.read();
            std::cout << "Cluster: Position = " << (output.data >> SIZE_BITS)
                    << ", Size = " << (output.data & ((1 << SIZE_BITS) - 1))
                    << ", Last = " << output.last << std::endl;
        }
    }

    return 0;
}
#endif
