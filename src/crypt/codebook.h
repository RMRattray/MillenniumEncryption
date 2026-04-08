#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <string>
#include <cstdint>
#include <vector>
#include <map>

typedef uint8_t byte_code;
typedef std::string full_code;
typedef std::string readable_code;

// A "readable code" is a code like 777 or 13,
// to indicate a seventh bit being set thrice, or a first bit set followed by a third, etc

// A "full code" is the string of bytes with bits set in that manner,
// such as "@@@" for 777, or something generally unreadable

// A "byte code" is a means of representing that in a single byte.
// Recall that each string contains a number of ones, some other digit 2 to 8, and that
// digit some number of times (referred to approximately in code as "ones", "digit", and "count")
// This restricts us to 256 possible strings.

// Note that "ones" can be 0 or more; digit, 2 to 8; count, at least one.
// In many bytes, the first two bits indicate the count of ones (up to three);
// the next three bits indicate the digit (minus one, e.g., 001 for 2, 111 for 8),
// and the last three bits indicate the count minus one (e.g., 000 means once, 111 means eight times)
//       OODDDCCC
// This leaves bytes of the form:
//       XX000XXX
// unclaimed.  Those are used to represent strings with count greater than 8, in the form:
//       OC000DDD
// wherein the number of ones is at most one, the count is represented minus 9 (a bit indicating it to be 9 or 10),
// and the digit is the same as before, leaving the same unclaimed string as before when DDD=000:
//       XX000000
// which is taken to refer to those with a count of 11, and no ones, measured as
//       DD000000
// and thus referring only to the first four digits

// This forms restrictions on which strings are valid:
// * for a count of up to eight, for any digit, there may be up to three ones
// * for a count of nine or ten, for any digit, there may or may not be a one
// * for a count of eleven, digit must be 2-5, and there may not be a one

byte_code to_byte_code(full_code code);
full_code to_full_code(byte_code code);
full_code to_full_code(readable_code code);
readable_code to_readable_code(full_code code);
bool valid_readable_code(readable_code code);

class Codebook {
    public:
        Codebook(std::string keyword);
        void write_to_file(std::string filename);
        bool read_from_file(std::string filename);
        bool read_from_strings(std::map<uint8_t, readable_code> &strings);
        virtual bool verify();
    protected:
        byte_code codes[256];
};

class FullCodebook : public Codebook {
    public:
        FullCodebook(std::string keyword);
        full_code operator+(const unsigned char c) const;
        byte_code operator*(const unsigned char c) const;
        char operator-(const byte_code code) const;
        bool verify() override;
    private:
        void get_full_codes();
        std::vector<full_code> full_codes;
        char uncodes[256];
};

#endif // CODEBOOK_H