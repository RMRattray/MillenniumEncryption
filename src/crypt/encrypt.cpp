#include <istream>
#include <stdexcept>
#include "codebook.h"
#include "encrypt.h"
#include <stdint.h>
#include <cstring>
#include <cstdlib>

#define BLOCKSIZE 256
#define BUFFER_SIZE 32

// Or's in highlighted bit from sequence 'what' to buffer at 'where',
// and returns pointer to right after the ones stop
uint8_t * write_to_buffer(full_code code, uint8_t * where) {
    uint8_t * r;
    uint8_t * what = (uint8_t*)code.c_str();
    // Write the ones
    while (*what == 1) {
        *where |= *what;
        ++what; ++where;
    }
    // Note where they stop
    r = where;
    // Write the rest
    while (*what) {
        *where |= *what;
        ++what; ++where;
    }
    return r;
}

uint8_t * where_to_write_in_buffer(byte_code code, uint8_t * start) {
    // Break down the code - get the digit and the number of ones
    uint8_t ones = code >> 6;
    uint8_t digit = (code >> 3) & 7;
    uint8_t count = code & 7;

    if (!digit) {
        if (!count) {
            digit = ones;
            ones = 0;
        }
        else {
            digit = count;
            ones = ones >> 1;
        }
    }

    digit = (1 << digit);

    // Find where the digit would start to be written
    // And how far forward we need to move
    uint8_t * dig_start = start + ones;
    while (*dig_start & digit) ++dig_start;
    while (*(dig_start - 1) & digit) ++dig_start;

    // Also possible that, if writing a sequence of no ones,
    // or writing *after* a sequence of no ones, we need to move forward one
    uint8_t * one_start = start;
    if (!ones || (ones && !(*(start - 1) & 1))) ++one_start;

    return std::max(dig_start - ones, one_start);
}

// Returns code - the latest non-space (or non-odd-numbered space read in)
// And space_before - the number of extra spaces before it
uint8_t * read_from_buffer(byte_code &code, uint8_t * where) {
    uint8_t * r; // Indicates where to look for the next sequence (where the 'ones' stop)
    uint8_t ones = 0, digit = 0, count = 0;

    if (*where & 1) goto there;
    
    // If not starting at a sequence of ones, read along until encountering a new sequence
    while (!(*(where + 1) & ~(*where))) {
        ++where;
    }
    ++where; // And there, the new sequence starts

    there:

    if (*where == 0xFF) {
        code = 0;
        return NULL;
    }

    // If there are ones, read them until a new non-one sequence appears
    if (*where & 1) {
        do {
            ++where;
            ++ones;
        } while (!(*(where) & ~(*(where - 1))));
    }
    // That new non-one character is the digit
    digit = (*(where) & ~(*(where - 1)));
    r = where; // Mark where the new non-one sequence starts
    ++where;
    while (*where & digit) {
        ++count;
        ++where;
    }

    // Assemble byte code from ones, digit, and count
    switch (digit) {
        case 2: digit = 1; break;
        case 4: digit = 2; break;
        case 8: digit = 3; break;
        case 16: digit = 4; break;
        case 32: digit = 5; break;
        case 64: digit = 6; break;
        case 128: digit = 7; break;
        default: throw std::runtime_error("Invalid syntax in ciphertext");
    }

    switch (count) {
        case 10:
            code = (digit - 1) << 6;
            break;
        case 9:
        case 8:
            code = (ones << 7) | (count << 6) | digit;
            break;
        default:
            code = (ones << 6) | (digit << 3) | count;
        break;
    }

    return r;
}

// Encrypt a stream of characters from input to output stream
void encrypt(std::istream &plaintext, std::ostream &ciphertext, FullCodebook codebook) {
    char * buffer = (char *)malloc(BLOCKSIZE + 1);
    uint8_t * buffer_start = (uint8_t*)buffer + 1;
    memset(buffer_start, 0, BLOCKSIZE);
    buffer[0] = 0xFF;
    uint8_t * c_start = buffer_start;
    char p;
    while (plaintext.good()) { // TODO:  Check for end-of-file instead, prepare to use istringstream, ostringstream to work with strings from elsewhere in the program
        while(plaintext.good() && (c_start < buffer_start + BLOCKSIZE - BUFFER_SIZE)) {
            p = plaintext.get();
            c_start = where_to_write_in_buffer(codebook*p, c_start);
            c_start = write_to_buffer(codebook+p, c_start);
        }
        // And roll back the tape, unless we're done,
        if (plaintext.good()) {
            ciphertext.write(buffer + 1, BLOCKSIZE - BUFFER_SIZE);
            memcpy(buffer_start, buffer_start + BLOCKSIZE - BUFFER_SIZE, BUFFER_SIZE);
            memset(buffer_start + BUFFER_SIZE, 0, BLOCKSIZE - BUFFER_SIZE);
            c_start -= (BLOCKSIZE - BUFFER_SIZE);
        }
        else { // In which case, simply write out what we've got
            while (*c_start) ++c_start;
            ciphertext.write(buffer + 1, c_start - buffer_start);
        }
    }
    free(buffer);
}

void decrypt(std::istream &ciphertext, std::ostream &plaintext, FullCodebook codebook) {
    char * buffer = (char *)malloc(BLOCKSIZE + 1);
    uint8_t * buffer_start = (uint8_t*)buffer + 1;
    uint8_t * c_start = buffer_start;
    memset(buffer_start, 0, BLOCKSIZE);
    buffer_start[BLOCKSIZE] = 0xFF;
    byte_code code;
    ciphertext.read(buffer + 1, BLOCKSIZE);
    do {
        while (c_start && c_start < buffer_start + BLOCKSIZE - BUFFER_SIZE) {
            c_start = read_from_buffer(code, c_start);
            plaintext.put(codebook-code);
        }
        memcpy(buffer_start, buffer_start + BLOCKSIZE - BUFFER_SIZE, BUFFER_SIZE);
        memset(buffer_start + BUFFER_SIZE, 0, BLOCKSIZE - BUFFER_SIZE);
        ciphertext.read((char*)buffer_start + BUFFER_SIZE, BLOCKSIZE - BUFFER_SIZE);
        c_start -= (BLOCKSIZE - BUFFER_SIZE);
    } while (ciphertext.good());
    free(buffer);
}