#include "otfparser.hpp"

#include <fstream>

namespace geul
{

// parse file into Font
Font parse_otf(const std::string& filename)
{
    Font font;
    auto input_buf = InputBuffer::open(filename);
    font.parse(input_buf);

    return font;
}

// write Font to file
void write_otf(const Font& font, const std::string& filename)
{
    auto buf = OutputBuffer::open(filename);
    font.compile(buf);
}
}
