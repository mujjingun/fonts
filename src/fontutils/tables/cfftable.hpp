#ifndef TABLES_CFF_TABLE_HPP
#define TABLES_CFF_TABLE_HPP

#include "otftable.hpp"

#include <vector>
#include <array>

namespace fontutils
{

class CFFTable : public OTFTable
{
    struct Font
    {
        std::string name;
        struct FontInfo
        {
            std::string version = "";
            std::string notice = "";
            std::string copyright = "";
            std::string fullname = "";
            std::string familyname = "";
            std::string weight = "";
            bool is_fixed_pitch = false;
            int italic_angle = 0;
            int underline_position = -100;
            int underline_thickness = 50;
            int paint_type = 0;
            int charstring_type = 2;
            std::array<double, 6> font_matrix = {0.001, 0, 0, 0.001, 0, 0};
            int unique_id = 0;
            std::array<int, 4> font_bbox = {0, 0, 0, 0};
            int stroke_width = 0;
            // gid -> cid mapping
            std::vector<std::string> charset;
        } fontinfo;
    };
    std::vector<Font> fonts;

public:
    CFFTable();
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
