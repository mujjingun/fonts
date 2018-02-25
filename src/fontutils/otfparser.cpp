#include "otfparser.hpp"

#include <fstream>

namespace geul
{

// parse file into Font
Font parse_otf(const std::string& filename)
{
    std::ifstream file(filename);
    if (file.fail())
        throw std::runtime_error("Cannot open file for reading.");

    Font   font;
    Buffer buf(std::string{ std::istreambuf_iterator<char>(file),
                            std::istreambuf_iterator<char>() });
    font.parse(buf);

    return font;
}

// write Font to file
void write_otf(const Font& font, const std::string& filename)
{
    std::ofstream file(filename);
    if (file.fail())
        throw std::runtime_error("Cannot open file for writing.");

    Buffer buf = font.compile();

    file.write(buf.data(), buf.size());
}
}
