#include <istream>
#include <stdint.h>
#define BLOCKSIZE 128
#define BUFFER_SIZE 16

// Or's in highlighted bit from sequence 'what' to buffer at 'where',
// and returns pointer to where the non-1 bytes start
uint8_t * write(uint8_t * what, uint8_t * where) {
    uint8_t * r;
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