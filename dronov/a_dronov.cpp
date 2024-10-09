#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <ctime>
#include "util.h"

struct FileProcessingTime {
    std::string filename;
    clock_t start_time;
    clock_t end_time;
    int sentence_count;  //  Sentences counter
};

std::vector<FileProcessingTime> fileProcessingTimes;
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;

// Directories initializing
std::string inputDirectory = "abstract_path\\in\\";
std::string outputDirectory = "abstract_path\\out\\";

// Function for counting sentences from file
void processFile(const std::string &inputFilename, const std::string &outputFilename) {
    FileProcessingTime processingTime;
    clock_t startFileTime, endFileTime;

    startFileTime = clock();
    processingTime.filename = inputFilename;

    std::ifstream inFile(inputFilename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Couldn't open input file '" << inputFilename << "'." << std::endl;
        return;
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Couldn't open output file '" << outputFilename << "'." << std::endl;
        inFile.close();
        return;
    }

    std::string line;
    int sentenceCount = 0;


    // Reading file content
    while (std::getline(inFile, line)) {
        for (char ch : line) {
            if (ch == '.' || ch == '?' || ch == '!') {
                sentenceCount++;
            }
        }
    }

    processingTime.sentence_count = sentenceCount;  

    outFile << "Number of sentences: " << sentenceCount << '\n';

    inFile.close();
    outFile.close();

    endFileTime = clock();
    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;
    fileProcessingTimes.push_back(processingTime);

    std::cout << "File '" << inputFilename << "' processed with " << sentenceCount << " sentences. Result saved to '" << outputFilename << "'." << std::endl;
}

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
                    << ", Elapsed Time: " << static_cast<double>(processingTime.end_time - processingTime.start_time) / CLOCKS_PER_SEC << " seconds"
                    << ", Number of Sentences: " << processingTime.sentence_count << '\n';  // Добавляем количество предложений
        }

        outFile.close();
        std::cout << "Data written to text file '" << outputFilename << "'." << std::endl;
    } else {
        std::cerr << "Error writing to file '" << outputFilename << "'." << std::endl;
    }
}

int main(int argc, char *argv[]) {
    startTime = clock();

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

    for (int i = 2; i < argc; ++i) {
        std::string inputFilename = inputDirectory + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end()) {
            std::cout << "File '" << inputFilename << "' reoccurs. Skipping." << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = outputDirectory + "out_" + argv[i];

        processFile(inputFilename, outputFilename);
    }

    endTime = clock();

    writeToTextFile("file_processing_times_a.txt");

    return 0;
}
