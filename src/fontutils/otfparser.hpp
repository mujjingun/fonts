#ifndef FONTUTILS_OTFPARSER_HPP
#define FONTUTILS_OTFPARSER_HPP

#include "tables/font.hpp"

namespace geul
{
Font parse_otf(std::string const& filename);

void write_otf(const Font& font, const std::string& filename);
}

#endif
