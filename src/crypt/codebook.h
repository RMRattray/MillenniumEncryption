#include <string>
#include <vector>

class Codebook {
    public:
        Codebook(std::string keyword);
        void write_to_file(std::string filename);
    private:
        byte codes[256];
};

class FullCodebook : public Codebook {
    public:
        FullCodebook(std::string keyword);
    private:
        std::vector<std::string> full_codes;
};