#ifndef PARSER_HPP
#define PARSER_HPP

#include <unordered_map>

#include "glyph.hpp"

namespace fontutils
{

struct Font
{
    int units_per_em;
    std::unordered_map<char32_t, Glyph> glyphs;
};

Font parse_font(std::string ttx_filename);
Font parse_otf(std::string otf_filename);

}

#endif
