#include "steganography.h"
#include <fstream>
#include <iostream>
#include <bitset>

// EXPLANATION FOR VIVA:
// We use 'std::ios::binary' because audio is raw binary data, not text.
// If we open it as text, the OS might corrupt newlines.

bool StegoEngine::EncryptAudio(const std::string& inputFile, const std::string& outputFile, const std::string& message) {
    std::ifstream in(inputFile, std::ios::binary);
    std::ofstream out(outputFile, std::ios::binary);

    if (!in || !out) return false;

    // 1. Copy the WAV Header (First 44 Bytes) exactly. 
    // If we touch this, the file won't play.
    char header[44];
    in.read(header, 44);
    out.write(header, 44);

    // 2. Prepare the secret message with a "stopper" character
    std::string secret = message + "\0"; // Null terminator tells us when to stop reading
    int msgIndex = 0;
    int bitIndex = 0;

    char buffer;
    while (in.get(buffer)) {
        if (msgIndex < secret.length()) {
            // Get the specific bit from the current character
            // Example: Letter 'A' is 01000001. We grab bits one by one.
            int bit = (secret[msgIndex] >> bitIndex) & 1;

            // LSB MAGIC:
            // (buffer & 0xFE) -> Clears the last bit (makes it 0)
            // | bit           -> Sets the last bit to our secret (0 or 1)
            buffer = (buffer & 0xFE) | bit;

            bitIndex++;
            if (bitIndex == 8) { // Move to next character after 8 bits
                bitIndex = 0;
                msgIndex++;
            }
        }
        out.put(buffer);
    }
    return true;
}

std::string StegoEngine::DecryptAudio(const std::string& inputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) return "Error: File not found";

    // Skip header
    in.seekg(44);

    std::string message = "";
    char currentChar = 0;
    int bitIndex = 0;
    char buffer;

    while (in.get(buffer)) {
        // Extract LSB: (buffer & 1) gives us the last bit
        int bit = (buffer & 1);

        // Reconstruct the character bit by bit
        currentChar |= (bit << bitIndex);

        bitIndex++;
        if (bitIndex == 8) {
            if (currentChar == '\0') break; // Stop if we hit the terminator
            message += currentChar;
            currentChar = 0; // Reset for next char
            bitIndex = 0;
        }
    }
    return message;
}