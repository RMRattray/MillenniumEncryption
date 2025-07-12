#include "codebook.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cassert>

byte_code to_byte_code(full_code code) {
    byte_code result = 0;
    uint8_t ones = 0, digit = 0, count = 0;
    uint8_t * ch = (uint8_t*)&code[0];
    uint8_t * end = ch + code.size();
    while (*ch == 1) {
        ++ones; ++ch;
    }
    *ch = *ch >> 1;
    while (*ch) {
        ++digit; *ch = (*ch) >> 1;
    }
    ++ch;
    while (ch < end) {
        ++count; ++ch;
    }
    return (ones << 6) | (digit << 3) | count;
}

// byte_code to_byte_code(readable_code code) {
//     byte_code result = 0;
//     uint8_t ones = 0, digit = 0, count = 0;
//     char * ch = &code[0];
//     while (*ch == '1') {
//         ++ones; ++ch;
//     }
//     digit = (*ch == '0' ? 0 : (*ch - '1'));
//     ++ch;
//     while (*ch) {
//         ++count; ++ch;
//     }
//     return (ones << 6) | (digit << 3) | count;
// }

full_code to_full_code(byte_code code) {
    full_code result = "";
    uint8_t ones = code >> 6;
    uint8_t digit = (code >> 3) & 7;
    if (digit) digit = (1 << digit);
    uint8_t count = code & 7;

    while (ones) {
        result += (char)1;
        --ones;
    }
    result += (char)digit;
    while (count) {
        result += (char)digit;
        --count;
    }
    return result;
}

full_code to_full_code(readable_code code) {
    full_code result = code;
    char * ch = &result[0];
    while (*ch) {
        switch (*ch) {
            case '0': *ch = 0; break;
            case '1': *ch = 1; break;
            case '2': *ch = 2; break;
            case '3': *ch = 4; break;
            case '4': *ch = 8; break;
            case '5': *ch = 16; break;
            case '6': *ch = 32; break;
            case '7': *ch = 64; break;
            case '8': *ch = 128; break;
            default:
            assert(false);
        }
        ++ch;
    }
    return result;
}

readable_code to_readable_code(byte_code code) {
    readable_code result = "";
    uint8_t ones = code >> 6;
    uint8_t digit = (code >> 3) & 7;
    digit = digit ? ('1' + digit) : '0';
    uint8_t count = code & 7;

    while (ones) {
        result += '1';
        --ones;
    }
    result += digit;
    while (count) {
        result += digit;
        --count;
    }
    return result;
}

readable_code to_readable_code(full_code code) {
    readable_code result = code;
    char * ch = &result[0];
    char * end = ch + result.size();
    while (ch < end) {
        switch (*ch) {
            case 0: *ch = '0'; break;
            case 1: *ch = '1'; break;
            case 2: *ch = '2'; break;
            case 4: *ch = '3'; break;
            case 8: *ch = '4'; break;
            case 16: *ch = '5'; break;
            case 32: *ch = '6'; break;
            case 64: *ch = '7'; break;
            case -128: *ch = '8'; break;
            default:
            assert(false);
        }
        ++ch;
    }
    return result;
}

Codebook::Codebook(std::string keyword) {
    // These are the primitive roots of 257 - numbers that, when multiplied modulo 257, will eventually yield every number from 1 to 256
    // They are repeated for convenience
    const uint8_t roots[256] = {3, 3, 3, 3, 5, 6, 7, 7, 7, 10, 10, 12, 12, 14, 14, 14, 14, 14, 19, 20, 20, 20, 20, 24, 24, 24, 27, 28, 28, 28, 28, 28, 33, 33, 33, 33, 37, 38, 39, 40, 41, 41, 43, 43, 45, 45, 47, 48, 48, 48, 51, 51, 53, 54, 55, 56, 56, 56, 56, 56, 56, 56, 63, 63, 65, 66, 66, 66, 69, 69, 71, 71, 71, 74, 75, 76, 77, 78, 78, 80, 80, 82, 83, 83, 85, 86, 87, 87, 87, 90, 91, 91, 93, 94, 94, 96, 97, 97, 97, 97, 101, 102, 103, 103, 105, 106, 107, 108, 109, 110, 110, 112, 112, 112, 115, 115, 115, 115, 119, 119, 119, 119, 119, 119, 125, 126, 127, 127, 127, 130, 131, 132, 132, 132, 132, 132, 132, 138, 138, 138, 138, 142, 142, 142, 145, 145, 147, 148, 149, 150, 151, 152, 152, 154, 155, 156, 156, 156, 156, 160, 161, 161, 163, 164, 164, 166, 167, 167, 167, 170, 171, 172, 172, 174, 175, 175, 177, 177, 179, 180, 181, 182, 183, 183, 183, 186, 186, 188, 188, 188, 191, 192, 192, 194, 194, 194, 194, 194, 194, 194, 201, 202, 203, 204, 204, 206, 206, 206, 209, 210, 210, 212, 212, 214, 214, 216, 217, 218, 219, 220, 220, 220, 220, 224, 224, 224, 224, 224, 229, 230, 230, 230, 233, 233, 233, 233, 237, 238, 238, 238, 238, 238, 243, 243, 245, 245, 247, 247, 247, 250, 251, 252, 252, 254, 254, 254};
    uint8_t used[256] = {0}; // Which result bytes are already used
    int multiplier;
    int index;
    int c;
    byte_code * code_ptr = codes;
    byte_code * tortoise = codes;

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

bool Codebook::read_from_file(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    file.read((char*)codes, 256);
    if (!file.eof()) {
        return false;
    }
    file.close();
    return this->verify();
}

bool Codebook::verify() {
    uint8_t used[256] = {0};
    byte_code * tortoise = codes;
    while (tortoise < codes + 256) {
        if (used[*tortoise]) {
            return false;
        }
        used[*tortoise] = 1;
        ++tortoise;
    }
    return true;
}

// FullCodebook::FullCodebook(std::string keyword) {
//     using Codebook::Codebook(keyword);
// }

// void FullCodebook::get_full_codes() {
//     byte_code * tortoise = codes;
//     while (tortoise < codes + 256) {
//         full_codes.push_back(tortoise);
//         tortoise = codes + *tortoise;
//     }
// }