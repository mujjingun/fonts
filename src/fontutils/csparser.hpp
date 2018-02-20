#ifndef FONTUTILS_CS_PARSER_HPP
#define FONTUTILS_CS_PARSER_HPP

#include <string>
#include <vector>

#include "buffer.hpp"
#include "glyph.hpp"

namespace fontutils
{

Glyph parse_charstring(
    std::string const& cs,
    std::vector<std::string> const& gsubrs,
    std::vector<std::string> const& lsubrs);

Buffer write_charstring(Glyph const& glyph);
}

#endif
