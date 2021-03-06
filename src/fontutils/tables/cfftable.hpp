#ifndef TABLES_CFF_TABLE_HPP
#define TABLES_CFF_TABLE_HPP

#include "../glyph.hpp"
#include "otftable.hpp"

#include <array>
#include <iterator>
#include <vector>

namespace geul
{

class CFFTable : public OTFTable
{
public:
    struct Font
    {
        std::string name;

        // All font info in top dict index
        // except charset, encoding, charstrings, private,
        // fdarray, and fdselect
        struct FontInfo
        {
            // SIDs
            std::string version;
            std::string notice;
            std::string copyright;
            std::string fullname;
            std::string familyname;
            std::string weight;

            bool                  is_fixed_pitch = false;
            int                   italic_angle = 0;
            int                   underline_position = -100;
            int                   underline_thickness = 50;
            int                   paint_type = 0;
            int                   charstring_type = 2;
            std::array<double, 6> font_matrix = { 0.001, 0, 0, 0.001, 0, 0 };
            int                   unique_id = 0;
            std::array<int, 4>    font_bbox = { 0, 0, 0, 0 };
            int                   stroke_width = 0;
            std::vector<int>      xuid;
            std::string           postscript;
            std::string           basefont_name;
            int                   basefont_blend;

            // CID font info
            std::string registry;
            std::string ordering;
            int         supplement;
            double      cid_font_version = 0;
            double      cid_font_revision = 0;
            int         cid_font_type = 0;
            int         cid_count = 8720;
            int         uid_base = 0;

            bool operator==(FontInfo const& rhs) const noexcept;
        } fontinfo;

        // charset (gid -> cid mappings)
        std::vector<uint16_t> charset;

        // charstrings (indexed by gid)
        std::vector<Glyph> glyphs;

        // font dict index
        std::vector<uint8_t> fd_select;

        struct FontDict
        {
            std::string      name;
            std::vector<int> blue_values;
            std::vector<int> other_blues;
            std::vector<int> family_blues;
            std::vector<int> family_other_blues;
            double           blue_scale = 0.039625;
            double           blue_shift = 7;
            double           blue_fuzz = 1;
            int              std_hw;
            int              std_vw;
            std::vector<int> stem_snap_h;
            std::vector<int> stem_snap_v;
            bool             force_bold = false;
            int              language_group = 0;
            double           expansion_factor = 0.06;
            int              initial_random_seed = 0;
            int              default_width_x = 0;
            int              nominal_width_x = 0;

            bool operator==(FontDict const& rhs) const noexcept;
        };
        std::vector<FontDict> fd_array;

        bool operator==(Font const& rhs) const noexcept;
    };
    std::vector<Font> fonts;

public:
    CFFTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "CFF ";
};
}

#endif
