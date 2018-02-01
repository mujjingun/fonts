#include "parser.hpp"

#include <pugixml.hpp>

#include "glyph.hpp"
#include "subroutines.hpp"

#include <iostream>

namespace fontutils
{

std::unordered_map<std::string, char32_t> parse_cmap(pugi::xml_node cmap_table)
{
    std::unordered_map<std::string, char32_t> cmap;
    for (auto map : cmap_table.child("cmap_format_12")) {
        std::string name = map.attribute("name").as_string();
        char32_t unicode = std::stoi(map.attribute("code").as_string(), 0, 16);
        cmap[name] = unicode;
    }

    cmap[".notdef"] = 0;

    return cmap;
}

Font parse_font(std::string ttx_filename)
{
    // TODO: asynchronously load file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(ttx_filename.c_str());
    if (result.status != pugi::status_ok)
        throw std::runtime_error(std::string("Cannot load file : ") + result.description());

    const auto body = doc.child("ttFont");

    const auto gsub = body.child("GSUB");
    for (auto item : gsub.child("LookupList")) {
        for (auto it = std::next(item.begin(), 2); it != item.end(); ++it) {
            //if (std::string(it->name()) == "ChainContextSubst")
                //std::cout <<  << std::endl;
        }
    }

    const auto gpos = body.child("GPOS");

    const auto cmap = parse_cmap(body.child("cmap"));

    const auto cff = body.child("CFF");

    subroutine_set subroutines;

    for (auto const charstring : cff.child("GlobalSubrs")) {
        subroutines[-1].push_back(Subroutine(charstring.text().get()));
    }

    Font font;
    font.units_per_em = body.child("head").child("unitsPerEm").attribute("value").as_int();
    { // TODO: do for each subfont
        auto cff_font = cff.child("CFFFont");

        for (auto const item : cff_font.child("FDArray")) {
            int idx = item.attribute("index").as_int();
            auto subrs = item.child("Private").child("Subrs");
            if (subrs) {
                for (auto const charstring : subrs) {
                    subroutines[idx].push_back(Subroutine(charstring.text().get()));
                }
            }
        }

        for (auto const charstring : cff_font.child("CharStrings")) {
            std::string chname = charstring.attribute("name").as_string();
            int fd_index = charstring.attribute("fdSelectIndex").as_int();
            auto it = cmap.find(chname);
            if (it != cmap.end()) {
                char32_t unicode = it->second;
                font.glyphs[unicode] = Glyph::from_charstring(chname, charstring.text().get(), subroutines, fd_index);
            }
        }
    }

    return font;
}

} // namespace fontutils
