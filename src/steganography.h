#pragma once
#include <string>
#include <vector>

// Define our Class to handle the secrets
class StegoEngine {
public:
    // Hides the 'message' inside 'inputFile' and saves to 'outputFile'
    bool EncryptAudio(const std::string& inputFile, const std::string& outputFile, const std::string& message);

    // Extracts hidden text from 'inputFile'
    std::string DecryptAudio(const std::string& inputFile);
};