#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <string>
#include <cstdint>
#include <vector>

#define MAGIC_NUMBER 3

typedef uint8_t byte_code;
typedef std::string full_code;
typedef std::string readable_code;

byte_code to_byte_code(full_code code);
// byte_code to_byte_code(readable_code code);
full_code to_full_code(byte_code code);
full_code to_full_code(readable_code code);
readable_code to_readable_code(byte_code code);
readable_code to_readable_code(full_code code);

class Codebook {
    public:
        Codebook(std::string keyword);
        void write_to_file(std::string filename);
        bool read_from_file(std::string filename);
        bool verify();
    private:
        byte_code codes[256];
};

// class FullCodebook : public Codebook {
//     public:
//         FullCodebook(std::string keyword);
//     private:
//         std::vector<full_code> full_codes;
// };

#endif // CODEBOOK_H