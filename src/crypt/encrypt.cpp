#include <istream>
#include <stdexcept>
#include "codebook.h"
#include "encrypt.h"
#include <stdint.h>
#define BLOCKSIZE 256
#define BUFFER_SIZE 32

// Or's in highlighted bit from sequence 'what' to buffer at 'where',
// and returns pointer to right after the ones stop
uint8_t * write_to_buffer(full_code code, uint8_t * where) {
    uint8_t * r;
    uint8_t * what = code.c_str();
    // Write the ones
    while (*what == 1) {
        *where |= *what;
        ++what; ++where;
    }
    // Note where they stop
    r = *where;
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
    return dig_start - ones;
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
        return -1;
    }

    // If there are ones, read them until a new non-one sequence appears
    if (*where & 1) {
        do {
            ++where;
            ++ones;
        } while (!(*(where + 1) & ~(*where)));
    }
    // That new non-one character is the digit
    digit = (*(where + 1) & ~(*where));
    ++where;
    r = where; // Mark where the new non-one sequence starts
    while (*where & digit) ++count;

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
        case 11:
            byte_code = (digit - 1) << 6;
            break;
        case 10:
        case  9:
            byte_code = (ones << 7) | (count << 5) | digit;
            break;
        default:
            byte_code = (ones << 6) | (digit << 3) | count;
        break;
    }

    return r;
}

// Encrypt a stream of characters from input to output stream
int encrypt(std::istream &plaintext, std::ostream &ciphertext, FullCodebook codebook) {
    uint8_t * buffer = (uint8_t *)malloc(BLOCKSIZE);
    uint8_t * c_start = buffer;
    char p = plaintext.get();
    while (plaintext.good()) { // TODO:  Check for end-of-file instead, prepare to use istringstream, ostringstream to work with strings from elsewhere in the program
        while(plaintext.good() && (c_start < buffer + BLOCKSIZE - BUFFER_SIZE)) {
            c_start = where_to_write_in_buffer(codebook*p, c_start);
            c_start = write_to_buffer(codebook+p, c_start);
            p = plaintext.get();
        }
        // And roll back the tape
        ciphertext.write(buffer, BLOCKSIZE - BUFFER_SIZE);
        memcpy(buffer, buffer + BLOCKSIZE - BUFFER_SIZE, BUFFER_SIZE);
        memset(buffer + BUFFER_SIZE, 0, BLOCKSIZE - BUFFER_SIZE);
    }
}

int decrypt(std::istream &ciphertext, std::ostream &plaintext, FullCodebook codebook) {
    uint8_t * buffer = (uint8_t *)malloc(BLOCKSIZE + 1);
    buffer[BLOCKSIZE] = 0xFF;
    uint8_t * c_start = buffer;
    byte_code code;
    ciphertext.read(buffer, BLOCKSIZE);
    do {
        while (c_start < buffer + BLOCKSIZE - BUFFER_SIZE) {
            c_start = read_from_buffer(code, c_start);
            plaintext.put(codebook-code);
        }
        memcpy(buffer, buffer + BLOCKSIZE - BUFFER_SIZE, BUFFER_SIZE);
        memset(buffer + BUFFER_SIZE, 0, BLOCKSIZE - BUFFER_SIZE);
        ciphertext.read(buffer + BUFFER_SIZE, BLOCKSIZE - BUFFER_SIZE);
        c_start -= (BLOCKSIZE - BUFFER_SIZE);
    } while (ciphertext.good());
}