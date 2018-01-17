#include "parser.hpp"

#include <pugixml.hpp>

#include "subroutines.hpp"

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

    auto body = doc.child("ttFont");

    const auto cmap = parse_cmap(body.child("cmap"));

    auto cff = body.child("CFF");

    std::vector<Subroutine> global_subrs;
    std::unordered_map<int, std::vector<Subroutine>> local_subrs;

    for (auto charstring : cff.child("GlobalSubrs")) {
        global_subrs.push_back(Subroutine(charstring.text().get()));
    }

    Font font;
    font.units_per_em = body.child("head").child("unitsPerEm").attribute("value").as_int();
    { // TODO: do for each subfont
        auto cff_font = cff.child("CFFFont");

        for (auto item : cff_font.child("FDArray")) {
            int idx = item.attribute("index").as_int();
            auto subrs = item.child("Subrs");
            if (subrs) {
                for (auto charstring : subrs) {
                    local_subrs[idx].push_back(Subroutine(charstring.text().get()));
                }
            }
        }

        for (auto charstring : cff_font.child("CharStrings")) {
            std::string chname = charstring.attribute("name").as_string();
            auto it = cmap.find(chname);
            if (it != cmap.end()) {
                char32_t unicode = it->second;
                font.glyphs[unicode] = Glyph::from_charstring(chname, charstring.text().get());
            }
        }
    }

    return font;
}

} // namespace fontutils
