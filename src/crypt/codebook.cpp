#include "codebook.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

void do_something() {
    std::cout << "Hello, World!" << std::endl;
}

Codebook::Codebook(std::string keyword) {
    // These are the primitive roots of 257 - numbers that, when multiplied modulo 257, will eventually yield every number from 1 to 256
    // They are repeated for convenience
    const uint8_t roots[256] = {3, 3, 3, 3, 3, 5, 6, 7, 7, 7, 10, 10, 12, 12, 14, 14, 14, 14, 14, 19, 20, 20, 20, 20, 24, 24, 24, 27, 28, 28, 28, 28, 28, 33, 33, 33, 33, 37, 38, 39, 40, 41, 41, 43, 43, 45, 45, 47, 48, 48, 48, 51, 51, 53, 54, 55, 56, 56, 56, 56, 56, 56, 56, 63, 63, 65, 66, 66, 66, 69, 69, 71, 71, 71, 74, 75, 76, 77, 78, 78, 80, 80, 82, 83, 83, 85, 86, 87, 87, 87, 90, 91, 91, 93, 94, 94, 96, 97, 97, 97, 97, 101, 102, 103, 103, 105, 106, 107, 108, 109, 110, 110, 112, 112, 112, 115, 115, 115, 115, 119, 119, 119, 119, 119, 119, 125, 126, 127, 127, 127, 130, 131, 132, 132, 132, 132, 132, 132, 138, 138, 138, 138, 142, 142, 142, 145, 145, 147, 148, 149, 150, 151, 152, 152, 154, 155, 156, 156, 156, 156, 160, 161, 161, 163, 164, 164, 166, 167, 167, 167, 170, 171, 172, 172, 174, 175, 175, 177, 177, 179, 180, 181, 182, 183, 183, 183, 186, 186, 188, 188, 188, 191, 192, 192, 194, 194, 194, 194, 194, 194, 194, 201, 202, 203, 204, 204, 206, 206, 206, 209, 210, 210, 212, 212, 214, 214, 216, 217, 218, 219, 220, 220, 220, 220, 224, 224, 224, 224, 224, 229, 230, 230, 230, 233, 233, 233, 233, 237, 238, 238, 238, 238, 238, 243, 243, 245, 245, 247, 247, 247, 250, 251, 252, 252, 254, 254};
    uint8_t used[256] = {0}; // Which result bytes are already used
    int multiplier;
    int index;
    int c;
    uint8_t * code_ptr = codes;
    uint8_t * tortoise = codes;

    // For each character in the keyword, assign a pseudo-related byte in the codebook
    // for the first few entries
    for (char ch : keyword) {
        c = ch;
        multiplier = roots[c];
        index = (c + 1) * multiplier % 257 - 1;
        while (used[index]) index = (index + 1) * multiplier % 257 - 1;
        *code_ptr = index;
        used[index] = 1;
        ++code_ptr;
    }

    // Imagine the same for a null terminator character
    multiplier = roots[0];
    index = 2;
    while (used[index]) index = (index + 1) * 3 % 257 - 1;
    *code_ptr = index;
    used[index] = 1;
    ++code_ptr;

    // Fill out the rest of the codebook based on the first part of the codebook
    while (code_ptr < codes + 256) {
        c = *tortoise;
        multiplier = roots[c];
        index = (c + 1) * multiplier % 257 - 1;
        while (used[index]) index = (index + 1) * multiplier % 257 - 1;
        *code_ptr = index;
        used[index] = 1;
        ++code_ptr;
    }
}

void Codebook::write_to_file(std::string filename) {
    std::ofstream file(filename, std::ios::binary);
    file.write((char*)codes, 256);
    file.close();
}

// FullCodebook::FullCodebook(std::string keyword) {
//     using Codebook::Codebook(keyword);
//     for (int i = 0; i < 256; i++) {
//         codes[i] = keyword[i % keyword.length()];
//     }
// }