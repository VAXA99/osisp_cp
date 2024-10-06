#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <ctime>
#include "util.h"

// Struct to store file processing time details
struct FileProcessingTime {
    std::string filename;
    clock_t start_time;
    clock_t end_time;
};

std::vector<FileProcessingTime> fileProcessingTimes;
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;
CRITICAL_SECTION cs;

// Initializing default directories
std::string inputDirectory = "abstract_path\\in\\";
std::string outputDirectory = "abstract_path\\out\\";

// Function to trim leading and trailing whitespace
std::string trimWhitespace(const std::string &str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Function to count punctuation marks in a line
int countPunctuation(const std::string &line) {
    int count = 0;
    for (char ch : line) {
        if (ispunct(ch)) {
            count++;
        }
    }
    return count;
}

// Thread function to process a file and save only the sentence with the most punctuation
DWORD WINAPI processFileThread(LPVOID lpParam) {
    auto *filePair = reinterpret_cast<std::pair<std::string, std::string> *>(lpParam);
    std::string inputFilename = filePair->first;
    std::string outputFilename = filePair->second;
    delete filePair;

    FileProcessingTime processingTime;
    clock_t startFileTime, endFileTime;

    startFileTime = clock();
    processingTime.filename = inputFilename;

    std::ifstream inFile(inputFilename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Couldn't open input file '" << inputFilename << "'." << std::endl;
        return 1;
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Couldn't open output file '" << outputFilename << "'." << std::endl;
        inFile.close();
        return 1;
    }

    std::string line;
    std::string maxPunctuationLine;
    int maxPunctuationCount = 0;

    // Process the input file, line by line
    while (std::getline(inFile, line)) {
        line = trimWhitespace(line);

        // Count punctuation in the line
        int punctuationCount = countPunctuation(line);
        if (punctuationCount > maxPunctuationCount) {
            maxPunctuationCount = punctuationCount;
            maxPunctuationLine = line;
        }
    }

    // Write only the sentence with the most punctuation to the output file
    if (!maxPunctuationLine.empty()) {
        outFile << maxPunctuationLine << '\n';
    }

    inFile.close();
    outFile.close();

    endFileTime = clock();
    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;

    EnterCriticalSection(&cs);
    fileProcessingTimes.push_back(processingTime);
    LeaveCriticalSection(&cs);

    std::cout << "Longest sentence with the most punctuation saved to '" << outputFilename << "'." << std::endl;
    return 0;
}

// Function to write processing times to a text file
void writeToTextFile(const std::string &filename) {
    std::string outputFilename = outputDirectory + filename;

    std::ofstream outFile(outputFilename);
    if (outFile.is_open()) {
        outFile << "Program Start Time: " << static_cast<double>(startTime) / CLOCKS_PER_SEC << " seconds\n";
        outFile << "Program End Time: " << static_cast<double>(endTime) / CLOCKS_PER_SEC << " seconds\n";
        outFile << "Program Elapsed Time: " << static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC << " seconds\n";

        outFile << "\nFile Processing Times:\n";
        for (const auto &processingTime : fileProcessingTimes) {
            outFile << "File: " << processingTime.filename
                    << ", Start Time: " << static_cast<double>(processingTime.start_time) / CLOCKS_PER_SEC << " seconds"
                    << ", End Time: " << static_cast<double>(processingTime.end_time) / CLOCKS_PER_SEC << " seconds"
                    << ", Elapsed Time: " << static_cast<double>(processingTime.end_time - processingTime.start_time) / CLOCKS_PER_SEC << " seconds\n";
        }

        outFile.close();
        std::cout << "Data written to text file '" << outputFilename << "'." << std::endl;
    } else {
        std::cerr << "Error writing to file '" << outputFilename << "'." << std::endl;
    }
}

int main(int argc, char *argv[]) {
    startTime = clock();
    InitializeCriticalSection(&cs);

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_type> <filename1> [<filename2> ... <filenameN>]" << std::endl;
        std::cerr << "<path_type> should be either '--removable' or '--harddisk'" << std::endl;
        return 1;
    }

    std::string pathType = argv[1];
    if (pathType == "--removable") {
        inputDirectory = "E:\\files\\in\\";
        outputDirectory = "E:\\files\\out\\";
    } else if (pathType == "--harddisk") {
        std::wstring executableDir = getDirectoryOfExecutable();
        inputDirectory = std::string(executableDir.begin(), executableDir.end()) + "\\files\\in\\";
        outputDirectory = std::string(executableDir.begin(), executableDir.end()) + "\\files\\out\\";
    } else {
        std::cerr << "Error: Unknown path type '" << pathType << "'." << std::endl;
        std::cerr << "<path_type> should be either '--removable' or '--harddisk'" << std::endl;
        return 1;
    }

    std::vector<HANDLE> threadHandles;

    for (int i = 2; i < argc; ++i) {
        std::string inputFilename = inputDirectory + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end()) {
            std::cout << "File '" << inputFilename << "' reoccurs. Skipping." << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = outputDirectory + "out_" + argv[i];

        auto *filePair = new std::pair<std::string, std::string>(inputFilename, outputFilename);
        HANDLE hThread = CreateThread(NULL, 0, processFileThread, filePair, 0, NULL);
        if (hThread == NULL) {
            std::cerr << "Error creating thread for file '" << inputFilename << "'." << std::endl;
            delete filePair;
        } else {
            threadHandles.push_back(hThread);
        }
    }

    WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

    for (auto &hThread : threadHandles) {
        CloseHandle(hThread);
    }

    endTime = clock();

    writeToTextFile("file_processing_times_b.txt");

    DeleteCriticalSection(&cs);

    return 0;
}
