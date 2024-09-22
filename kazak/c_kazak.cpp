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

std::string cleanAndLowercase(const std::string& word) {
    std::string cleaned;
    std::remove_copy_if(word.begin(), word.end(), std::back_inserter(cleaned), [](char c) { return std::ispunct(static_cast<unsigned char>(c)); });
    std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::tolower);
    return cleaned;
}

// Function to check if a line contains the target word
bool containsTargetWord(const std::string &line, const std::string &targetWord) {
    std::istringstream iss(line);
    std::string word;
    std::string lowerTargetWord = cleanAndLowercase(targetWord);

    while (iss >> word) {
        std::string cleanedWord = cleanAndLowercase(word);
        if (cleanedWord == lowerTargetWord) {
            return true;
        }
    }
    return false;
}

// Thread function to process a file and find sentences containing the target word
DWORD WINAPI processFileThread(LPVOID lpParam) {
    auto *fileData = reinterpret_cast<std::pair<std::pair<std::string, std::string>, std::string> *>(lpParam);
    std::string inputFilename = fileData->first.first;
    std::string outputFilename = fileData->first.second;
    std::string targetWord = fileData->second;
    delete fileData;

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
    std::vector<std::string> sentencesWithTargetWord;

    while (std::getline(inFile, line)) {
        if (containsTargetWord(line, targetWord)) {
            sentencesWithTargetWord.push_back(line);
        }
    }

    outFile << "Sentences containing the word '" << targetWord << "':\n";
    for (const auto &sentence : sentencesWithTargetWord) {
        outFile << sentence << '\n';
    }

    inFile.close();
    outFile.close();

    endFileTime = clock();
    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;

    EnterCriticalSection(&cs);
    fileProcessingTimes.push_back(processingTime);
    LeaveCriticalSection(&cs);

    std::cout << "Sentences processed and saved to '" << outputFilename << "'." << std::endl;
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

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <path_type> <target_word> <filename1> [<filename2> ... <filenameN>]" << std::endl;
        std::cerr << "<path_type> should be either '--removable' or '--harddisk'" << std::endl;
        return 1;
    }

    std::string pathType = argv[1];
    std::string targetWord = argv[2];

    if (pathType == "--removable") {
        inputDirectory = "E:\\files\\in\\"; // In
        outputDirectory = "E:\\files\\out\\"; // Out
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

    for (int i = 3; i < argc; ++i) {
        std::string inputFilename = inputDirectory + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end()) {
            std::cout << "File '" << inputFilename << "' reoccurs. Skipping." << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = outputDirectory + "out_" + argv[i];

        auto *fileData = new std::pair<std::pair<std::string, std::string>, std::string>(std::make_pair(inputFilename, outputFilename), targetWord);
        HANDLE hThread = CreateThread(NULL, 0, processFileThread, fileData, 0, NULL);
        if (hThread == NULL) {
            std::cerr << "Error creating thread for file '" << inputFilename << "'." << std::endl;
            delete fileData;
        } else {
            if (!SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL)) {
                std::cerr << "Failed to set thread priority for file '" << inputFilename << "'." << std::endl;
            }
            threadHandles.push_back(hThread);
        }
    }

    WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

    for (auto &hThread : threadHandles) {
        CloseHandle(hThread);
    }

    endTime = clock();

    writeToTextFile("file_processing_times_c.txt");

    DeleteCriticalSection(&cs);

    return 0;
}
