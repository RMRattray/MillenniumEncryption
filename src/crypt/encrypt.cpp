#include <istream>
#include <stdexcept>
#include "codebook.h"
#include "encrypt.h"
#include <stdint.h>
#define BLOCKSIZE 128
#define BUFFER_SIZE 16

// Or's in highlighted bit from sequence 'what' to buffer at 'where',
// and returns pointer to where the non-1 bytes start
uint8_t * write_to_buffer(full_code code, uint8_t * where) {

    uint8_t * c = code.c_str();
    if (!(*c)) return where + code.size();

    uint8_t * r;
    // Write the ones
    while (*c == 1) {
        *where |= *c;
        ++what; ++where;
    }
    // Note where they stop
    r = where;
    // Write the rest
    while (*c) {
        *where |= *c;
        ++what; ++where;
    }
    return r + 1;
}

uint8_t * read_from_buffer(byte_code &code, uint8_t * where) {
    uint8_t ones = 0, digit = 0, digit_power; count = 0;
    uint8_t * r;
    while (*where == 1) {
        ++ones; ++where;
    }
    digit_power = *where;
    switch (digit_power) {
        case 0: digit = 0; break;
        case 2: digit = 1; break;
        case 4: digit = 2; break;
        case 8: digit = 3; break;
        case 16: digit = 4; break;
        case 32: digit = 5; break;
        case 64: digit = 6; break;
        case 128: digit = 7; break;
        default:
        throw std::runtime_error("Invalid syntax in encrypted buffer");
    }
    ++where;
    r = where;
    if (digit_power) {
        while (*where & digit_power) {
            *where &= (~digit_power)
            ++count;
            ++where;
        }
    }
    else {
        int limit = 8;
        while (limit-- && !(*where)) {
            ++where;
            ++count;
        }
        if (!limit) r = NULL; // Found end of the file
    }

    if (ones > 3 | digit > 7 | count > 7) throw std::runtime_error("Invalid syntax in encrypted buffer");
    *code = (ones << 6) | (digit << 3) | count;
    return r;
}

// Encrypt a stream of characters from input to output stream
// Codebook contains written-out bytes; compact_form is of four-byte form
// where first byte is one-count; second byte is the repeated non-one byte; third byte is the count
int encrypt(std::istream &plaintext, std::ostream &ciphertext, std::unordered_map<char, uint8_t*> codebook, std::unordered_map<char, uint32_t> compact_form) {
    uint8_t * buffer = (uint8_t *)malloc(BLOCKSIZE);
    uint8_t * c_start = buffer;
    char p = plaintext.get();
    uint8_t * c;
    while (p) { // TODO:  Check for end-of-file instead, prepare to use istringstream, ostringstream to work with strings from elsewhere in the program
        while(p && (c_start < buffer + BLOCKSIZE - BUFFER_SIZE)) {
            c = codebook[p];
            // If the non-ones part causes a conflict, keep moving
            while(*(c_start + (compact_form[p] >> 24)) & compact_form[p] >> 16) ++c_start;
            c_start = write(c, c_start + 1);
            p = plaintext.get();
        }
        // And roll back the tape
        ciphertext.write(buffer, BLOCKSIZE - BUFFER_SIZE);
        memcpy(buffer, buffer + BLOCKSIZE - BUFFER_SIZE, BUFFER_SIZE);
        memset(buffer + BUFFER_SIZE, 0, BLOCKSIZE - BUFFER_SIZE);
    }
}