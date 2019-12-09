//
//  main.cpp
//  Femto2CSV
//
//  Created by Ammon Wolfert on 12/8/19.
//  Copyright © 2019 Ammon Wolfert. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

void PrintBinary(char byte) {
    for (int i = 7; i >= 0; i--) {
        if ((1 << i) & byte) {
            std::cout << '1';
        } else {
            std::cout << '0';
        }
    }
}

void PrintHex(char byte) {
    for (int i = 0; i < 5; i += 4) {
        uint8_t nibble = ((15 << i) & byte) >> i;
        switch (nibble) {
            case 0:
                std::cout << '0';
                break;
            case 1:
                std::cout << '1';
                break;
            case 2:
                std::cout << '2';
                break;
            case 3:
                std::cout << '3';
                break;
            case 4:
                std::cout << '4';
                break;
            case 5:
                std::cout << '5';
                break;
            case 6:
                std::cout << '6';
                break;
            case 7:
                std::cout << '7';
                break;
            case 8:
                std::cout << '8';
                break;
            case 9:
                std::cout << '9';
                break;
            case 10:
                std::cout << 'A';
                break;
            case 11:
                std::cout << 'B';
                break;
            case 12:
                std::cout << 'C';
                break;
            case 13:
                std::cout << 'D';
                break;
            case 14:
                std::cout << 'E';
                break;
            case 15:
                std::cout << 'F';
                break;
        }
    }
}

void PrintAscii(char byte) {
    if (byte >= 32 && byte < 127) {
        std::cout << byte;
    } else {
        std::cout << '.';
    }
}

void PrintDump(char* buffer, int fileLength) {
    int bytesPerLine = 26;
    std::cout << "Beginning Data Dump" << std::endl;
    for (int i = 0; i < fileLength; i += bytesPerLine) {
        std::cout << i << " | ";
        for (int j = 0; j < bytesPerLine; j++) {
            PrintHex(buffer[i + j]);
            std::cout << " ";
        }
        std::cout << "| ";
        for (int j = 0; j < bytesPerLine; j++) {
            PrintAscii(buffer[i + j]);
        }
        std::cout << std::endl;
    }
}

float ConvertToFloat(char* startByte, int position) {
    float* number = (float*)(&startByte[position]);
    return *number;
}

void WriteToFile(std::ofstream stream, char* buffer, int offset, char deliminator) {
    stream << ConvertToFloat(buffer, offset) << deliminator;
}

int main() {
    
    std::string fileToRead;
    std::cout << "Enter a file name to read: " << std::endl;
    getline(std::cin, fileToRead);
    
    std::ifstream datastream;
    datastream.open(fileToRead, std::ios::binary);
    if (!datastream.is_open()) {
        std::cout << "Failed to open " << fileToRead << ". Aborting execution.";
        return 1;
    } else {
        std::cout << "Reading from file " << fileToRead << "..." << std::endl;
    }
    
    datastream.seekg(0, datastream.end);
    int fileLength = (int)datastream.tellg();
    datastream.seekg(0, datastream.beg);
    
    char* buffer = new char[fileLength];
    datastream.read(buffer, fileLength);
    
    PrintDump(buffer, fileLength);
    
    std::string fileToSave;
    std::cout << "Enter a file name to save to: " << std::endl;
    getline(std::cin, fileToSave);
    
    std::ofstream datarecord;
    datarecord.open(fileToSave);
    if (!datarecord.is_open()) {
        std::cout << "Failed to open file " << fileToSave << std::endl;
        return 2;
    }
    
    std::cout << "Writing to File..." << std::endl;
    for (int i = 0; i < fileLength; i += 52) {
        datarecord << ConvertToFloat(buffer, i+5) << ',';
        datarecord << ConvertToFloat(buffer, i+9) << ',';
        datarecord << ConvertToFloat(buffer, i+13) << ',';
        datarecord << ConvertToFloat(buffer, i+17) << ',';
        datarecord << ConvertToFloat(buffer, i+21) << ',';
        
        datarecord << ConvertToFloat(buffer, i+26) << ',';
        datarecord << ConvertToFloat(buffer, i+30) << ',';
        datarecord << ConvertToFloat(buffer, i+34) << ',';
        datarecord << ConvertToFloat(buffer, i+38) << ',';
        datarecord << ConvertToFloat(buffer, i+42) << '\n';
    }
    
    std::cout << "Finished Writing. Closing files..." << std::endl;
    datastream.close();
    datarecord.close();
    
    return 0;
}
