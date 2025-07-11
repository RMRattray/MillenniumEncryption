#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <string>
#include <cstdint>
#include <vector>

#define MAGIC_NUMBER 3

void do_something();

class Codebook {
    public:
        Codebook(std::string keyword);
        void write_to_file(std::string filename);
    private:
        uint8_t codes[256];
};

// class FullCodebook : public Codebook {
//     public:
//         FullCodebook(std::string keyword);
//     private:
//         std::vector<std::string> full_codes;
// };

#endif // CODEBOOK_H