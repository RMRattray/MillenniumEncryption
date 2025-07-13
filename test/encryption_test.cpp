#include <iostream>
#include <fstream>
#include <string>
#include "../src/crypt/codebook.h"
#include "../src/crypt/encrypt.h"

void write_sample_texts() {
    std::ofstream file("sample_texts.txt");
    file << "Hello, world!" << std::endl;
    file.close();
}

int main(int argc, char *argv[]) {
    write_sample_texts();
    FullCodebook codebook("test");

    std::cout << "Codebook includes:" << std::endl;
    for (char c : "Hello, world!\n") {
        std::cout << c << " -> " << to_readable_code(codebook + c) << std::endl;
    }

    std::ifstream plaintext("sample_texts.txt");
    std::ofstream ciphertext("ciphertext.txt");
    encrypt(plaintext, ciphertext, codebook);
    plaintext.close();
    ciphertext.close();
    std::ifstream ciphertext_new("ciphertext.txt");
    std::ofstream plaintext_new("plaintext.txt");
    decrypt(ciphertext_new, plaintext_new, codebook);
    ciphertext_new.close();
    plaintext_new.close();
    return 0;
}