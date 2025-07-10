#include <iostream>
#include <../src/crypt/codebook.h>

int main(int argc, char *argv[]) {
    Codebook codebook("test");
    codebook.write_to_file("codebook.bin");
    return 0;
}