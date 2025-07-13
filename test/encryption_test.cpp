#include <iostream>
#include <fstream>
#include <string>
#include "../src/crypt/codebook.h"
#include "../src/crypt/encrypt.h"

void write_sample_texts() {
    std::ofstream file("sample_texts.txt");
    file << "Hello, world!" << std::endl;
    file.close();

    std::ofstream file2("sample_texts2.txt");
    file2 << "T" << (char)130 <<"!" << std::endl;
    file2.close();
}

int main(int argc, char *argv[]) {
    write_sample_texts();
    FullCodebook codebook("test");

    std::cout << "Codebook includes:" << std::endl;
    for (char c : "Hello, world!\n") {
        std::cout << c << " -> " << to_readable_code(codebook + c) << std::endl;
    }

    std::ifstream plaintext("sample_texts.txt");
    std::ofstream ciphertext("ciphertext.mlnm");
    encrypt(plaintext, ciphertext, codebook);
    plaintext.close();
    ciphertext.close();
    std::ifstream ciphertext_new("ciphertext.mlnm");
    std::ofstream plaintext_new("plaintext.txt");
    decrypt(ciphertext_new, plaintext_new, codebook);
    ciphertext_new.close();
    plaintext_new.close();

    std::ifstream plaintext2("sample_texts2.txt");
    std::ofstream ciphertext2("ciphertext2.mlnm");
    encrypt(plaintext2, ciphertext2, codebook);
    plaintext2.close();
    ciphertext2.close();
    std::ifstream ciphertext_new2("ciphertext2.mlnm");
    std::ofstream plaintext_new2("plaintext2.txt");
    decrypt(ciphertext_new2, plaintext_new2, codebook);
    ciphertext_new2.close();
    plaintext_new2.close();

    std::ifstream plaintext3("test/plaintext.txt");
    std::ofstream ciphertext3("ciphertext3.mlnm");
    encrypt(plaintext3, ciphertext3, codebook);
    plaintext3.close();
    ciphertext3.close();
    std::ifstream ciphertext_new3("ciphertext3.mlnm");
    std::ofstream plaintext_new3("plaintext3.txt");
    decrypt(ciphertext_new3, plaintext_new3, codebook);
    ciphertext_new3.close();
    plaintext_new3.close();

    return 0;
}