#include <iostream>
#include "../src/crypt/codebook.h"
#include <string>

int main(int argc, char *argv[]) {
    do_something();
    // std::string keyword = "test";
    // std::string filename = "codebook.bin";
    // Codebook codebook(keyword);
    // codebook.write_to_file(filename);
    std::cout << MAGIC_NUMBER << std::endl;
    return 0;
}