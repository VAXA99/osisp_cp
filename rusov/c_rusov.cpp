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
CRITICAL_SECTION cs; // Critical section for thread safety

// Initializing default directories
std::string inputDirectory = "abstract_path\\in\\";
std::string outputDirectory = "abstract_path\\out\\";

// Function to trim leading and trailing whitespace
std::string trimWhitespace(const std::string &str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Thread function to process a file and write sentences to an output file
DWORD WINAPI processFileThread(LPVOID lpParam) {
    auto *filePair = reinterpret_cast<std::pair<std::string, std::string> *>(lpParam);
    std::string inputFilename = filePair->first;
    std::string outputFilename = filePair->second;
    delete filePair; // Free memory

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
    size_t sentenceCounter = 0; // Local counter for sentences

    // Process each line of the input file
    while (std::getline(inFile, line)) {
        line = trimWhitespace(line); // Trim spaces
        std::string delimiters = ".!?"; // Sentence delimiters
        size_t pos = 0;

        // Split the line into sentences
        while ((pos = line.find_first_of(delimiters)) != std::string::npos) {
            std::string sentence = line.substr(0, pos + 1); // Include delimiter
            outFile << (++sentenceCounter) << ": " << trimWhitespace(sentence) << std::endl; // Write sentence
            line.erase(0, pos + 1); // Remove the extracted sentence
        }

        // Write any remaining text as the last sentence
        if (!line.empty()) {
            outFile << (++sentenceCounter) << ": " << trimWhitespace(line) << std::endl;
        }
    }

    inFile.close();
    outFile.close();

    endFileTime = clock();
    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;

    EnterCriticalSection(&cs); // Enter critical section for thread safety
    fileProcessingTimes.push_back(processingTime); // Save processing time
    LeaveCriticalSection(&cs); // Leave critical section

    std::cout << "Lines processed and saved to '" << outputFilename << "'." << std::endl;
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
    InitializeCriticalSection(&cs); // Initialize critical section

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_type> <filename1> [<filename2> ... <filenameN>]" << std::endl;
        std::cerr << "<path_type> should be either '--removable' or '--harddisk'" << std::endl;
        return 1;
    }

    // Determine input and output directories based on path type
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

    // Create threads for each input file
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
            delete filePair; // Clean up memory in case of thread creation failure
        } else {
            if (!SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL)) {
                std::cerr << "Failed to set thread priority for file '" << inputFilename << "'." << std::endl;
            }  
            threadHandles.push_back(hThread); // Store thread handle
        }
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

    // Close thread handles
    for (auto &hThread : threadHandles) {
        CloseHandle(hThread);
    }

    endTime = clock();

    writeToTextFile("file_processing_times_c.txt");

    DeleteCriticalSection(&cs); // Clean up critical section
    return 0;
}
