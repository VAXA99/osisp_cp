#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <ctime>
#include "util.h"

// Structure for storing file processing time
struct FileProcessingTime {
    std::string filename;
    clock_t start_time;
    clock_t end_time;
};

std::vector<FileProcessingTime> fileProcessingTimes;
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;

// Initialization of directories
std::string inputDirectory = "abstract_path\\in\\";
std::string outputDirectory = "abstract_path\\out\\";

// Global counter for sentences
size_t globalSentenceCounter = 0;

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

    // Process each line of the input file
    while (std::getline(inFile, line)) {
        // Remove leading and trailing spaces
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Split the line into sentences
        std::string delimiters = ".!?"; // Sentence delimiters
        std::vector<std::string> sentences;
        size_t pos = 0;

        // Extract sentences
        while ((pos = line.find_first_of(delimiters)) != std::string::npos) {
            std::string sentence = line.substr(0, pos + 1); // Include the delimiter
            sentences.push_back(sentence);
            line.erase(0, pos + 1); // Remove the extracted sentence from the line
        }

        // If there's remaining text, add it as the last sentence
        if (!line.empty()) {
            sentences.push_back(line);
        }

        // Numbering and writing sentences to the output file
        for (const auto &sentence : sentences) {
            std::string trimmedSentence = sentence;
            trimmedSentence.erase(0, trimmedSentence.find_first_not_of(" \t")); // Remove leading spaces
            trimmedSentence.erase(trimmedSentence.find_last_not_of(" \t") + 1); // Remove trailing spaces

            if (!trimmedSentence.empty()) {
                outFile << (++globalSentenceCounter) << ": " << trimmedSentence << std::endl; // Use global counter
            }
        }
    }

    inFile.close();
    outFile.close();

    endFileTime = clock();
    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;
    fileProcessingTimes.push_back(processingTime);

    std::cout << "Lines sorted and saved to '" << outputFilename << "'." << std::endl;
}

// Function to write processing time to a text file
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
