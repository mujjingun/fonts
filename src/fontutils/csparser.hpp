#ifndef FONTUTILS_CS_PARSER_HPP
#define FONTUTILS_CS_PARSER_HPP

#include <string>
#include <vector>

#include "buffer.hpp"
#include "glyph.hpp"

namespace geul
{

Glyph parse_charstring(
    std::string const&              cs,
    std::vector<std::string> const& gsubrs,
    std::vector<std::string> const& lsubrs,
    int                             default_width,
    int                             nominal_width);

Buffer write_charstring(Glyph const& glyph);
}

#endif
