#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "../crypt/encrypt.h"
#include "../crypt/codebook.h"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d                    Decrypt instead of encrypting\n"
              << "  -f <filename>         File to be encrypted or decrypted\n"
              << "  -o <filename>         Output file (default: append .mlnm or first 3 letters)\n"
              << "  -k <keyword>          Keyword for codebook initialization\n"
              << "  -s <string>           String to be encrypted (instead of file)\n"
              << "  -h                    Show this help message\n"
              << "\n"
              << "Examples:\n"
              << "  " << program_name << " -k secret -f input.txt -o output.mlnm\n"
              << "  " << program_name << " -d -k secret -f output.mlnm -o decrypted.txt\n"
              << "  " << program_name << " -k secret -s \"Hello World\"\n";
}

std::string get_default_output_filename(const std::string& input_filename, bool decrypt_mode) {
    if (decrypt_mode) {
        // For decryption, remove .mlnm extension if present
        if (input_filename.length() > 5 && 
            input_filename.substr(input_filename.length() - 5) == ".mlnm") {
            return input_filename.substr(0, input_filename.length() - 5);
        }
        // If no .mlnm extension, append "_decrypted"
        return input_filename + "_decrypted";
    } else {
        // For encryption, append .mlnm
        return input_filename + ".mlnm";
    }
}

std::string get_default_output_filename_from_string(const std::string& input_string, bool decrypt_mode) {
    if (decrypt_mode) {
        return "decrypted_output";
    } else {
        // Take first 3 letters of input string
        std::string prefix = input_string.substr(0, std::min((long long unsigned int)3, input_string.length()));
        return prefix + ".mlnm";
    }
}

int main(int argc, char **argv) {
    bool decrypt_mode = false;
    std::string input_file;
    std::string output_file;
    std::string keyword;
    std::string input_string;
    bool use_string_input = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            decrypt_mode = true;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i];
            } else {
                std::cerr << "Error: -f requires a filename\n";
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires a filename\n";
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-k") == 0) {
            if (i + 1 < argc) {
                keyword = argv[++i];
            } else {
                std::cerr << "Error: -k requires a keyword\n";
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                input_string = argv[++i];
                use_string_input = true;
            } else {
                std::cerr << "Error: -s requires a string\n";
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Error: Unknown option " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Validate required parameters
    if (keyword.empty()) {
        std::cerr << "Error: Keyword (-k) is required\n";
        print_usage(argv[0]);
        return 1;
    }
    
    if (!use_string_input && input_file.empty()) {
        std::cerr << "Error: Either input file (-f) or input string (-s) must be specified\n";
        print_usage(argv[0]);
        return 1;
    }
    
    if (use_string_input && !input_file.empty()) {
        std::cerr << "Error: Cannot specify both input file (-f) and input string (-s)\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // Set default output filename if not provided
    if (output_file.empty()) {
        if (use_string_input) {
            output_file = get_default_output_filename_from_string(input_string, decrypt_mode);
        } else {
            output_file = get_default_output_filename(input_file, decrypt_mode);
        }
    }
    
    try {
        // Initialize codebook
        FullCodebook codebook(keyword);
        
        if (use_string_input) {
            // Handle string input/output
            std::istringstream input_stream(input_string);
            std::ostringstream output_stream;
            
            if (decrypt_mode) {
                decrypt(input_stream, output_stream, codebook);
            } else {
                encrypt(input_stream, output_stream, codebook);
            }
            
            // Write result to output file
            std::ofstream out_file(output_file, std::ios::binary);
            if (!out_file) {
                std::cerr << "Error: Cannot open output file " << output_file << "\n";
                return 1;
            }
            
            out_file << output_stream.str();
            out_file.close();
            
            std::cout << "Operation completed successfully!\n";
            std::cout << "Input string: " << input_string << "\n";
            std::cout << "Output written to: " << output_file << "\n";
            
        } else {
            // Handle file input/output
            std::ifstream in_file(input_file, std::ios::binary);
            if (!in_file) {
                std::cerr << "Error: Cannot open input file " << input_file << "\n";
                return 1;
            }
            
            std::ofstream out_file(output_file, std::ios::binary);
            if (!out_file) {
                std::cerr << "Error: Cannot open output file " << output_file << "\n";
                return 1;
            }
            
            if (decrypt_mode) {
                decrypt(in_file, out_file, codebook);
            } else {
                encrypt(in_file, out_file, codebook);
            }
            
            in_file.close();
            out_file.close();
            
            std::cout << "Operation completed successfully!\n";
            std::cout << "Input file: " << input_file << "\n";
            std::cout << "Output file: " << output_file << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}