#include <iostream>
#include "../src/crypt/codebook.h"
#include <string>
#include <cassert>

int main(int argc, char *argv[]) {

    // Test codebooks
    std::string keyword = "test";
    std::string filename = "codebook.bin";
    Codebook codebook(keyword);
    codebook.write_to_file(filename);
    assert(codebook.verify());
    codebook.read_from_file(filename);
    assert(codebook.verify());

    // Test converters
    byte_code byte_a = 0x92, byte_b = 0x01, byte_c = 0x22, byte_d = 0x58, byte_e = 0x00, byte_f = 0x80, byte_g = 0x86, byte_h = 0xff;
    full_code full_a = "\x1\x1\x4\x4\x4", full_b = "\x2\x2\x2\x2\x2\x2\x2\x2\x2", full_c = "\x10\x10\x10", full_d = "\x1\x8", full_e = "\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2", full_f = "\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8", full_g = "\x1\x40\x40\x40\x40\x40\x40\x40\x40\x40", full_h = "\x1\x1\x1\x80\x80\x80\x80\x80\x80\x80\x80";
    readable_code read_a = "11333", read_b = "222222222", read_c = "555", read_d = "14", read_e = "22222222222", read_f = "44444444444", read_g = "1777777777", read_h = "11188888888";

    assert(to_byte_code(full_a) == byte_a);
    assert(to_byte_code(full_b) == byte_b);
    assert(to_byte_code(full_c) == byte_c);
    assert(to_byte_code(full_d) == byte_d);
    assert(to_byte_code(full_e) == byte_e);
    assert(to_byte_code(full_f) == byte_f);
    assert(to_byte_code(full_g) == byte_g);
    assert(to_byte_code(full_h) == byte_h);

    assert(to_full_code(byte_a) == full_a);
    assert(to_full_code(byte_b) == full_b);
    assert(to_full_code(byte_c) == full_c);
    assert(to_full_code(byte_d) == full_d);
    assert(to_full_code(byte_e) == full_e);
    assert(to_full_code(byte_f) == full_f);
    assert(to_full_code(byte_g) == full_g);
    assert(to_full_code(byte_h) == full_h);

    assert(to_full_code(read_a) == full_a);
    assert(to_full_code(read_b) == full_b);
    assert(to_full_code(read_c) == full_c);
    assert(to_full_code(read_d) == full_d);
    assert(to_full_code(read_e) == full_e);
    assert(to_full_code(read_f) == full_f);
    assert(to_full_code(read_g) == full_g);
    assert(to_full_code(read_h) == full_h);

    assert(to_readable_code(full_a) == read_a);
    assert(to_readable_code(full_b) == read_b);
    assert(to_readable_code(full_c) == read_c);
    assert(to_readable_code(full_d) == read_d);
    assert(to_readable_code(full_e) == read_e);
    assert(to_readable_code(full_f) == read_f);
    assert(to_readable_code(full_g) == read_g);
    assert(to_readable_code(full_h) == read_h);

    std::cout << "All tests passed" << std::endl;
    return 0;
}