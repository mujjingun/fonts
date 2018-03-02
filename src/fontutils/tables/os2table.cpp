#include "os2table.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

OS2Table::OS2Table()
    : OTFTable(tag)
{}

void OS2Table::parse(InputBuffer& dis)
{
    std::cout << "Parsing 'OS/2'... " << std::endl;

    version = dis.read<uint16_t>();
    if (version == 3 || version == 4)
    {
        x_avg_char_width = dis.read<int16_t>();
        us_weight_class = dis.read<uint16_t>();
        us_width_class = dis.read<uint16_t>();
        fs_type = dis.read<uint16_t>();
        y_subscript_x_size = dis.read<int16_t>();
        y_subscript_y_size = dis.read<int16_t>();
        y_subscript_x_offset = dis.read<int16_t>();
        y_subscript_y_offset = dis.read<int16_t>();
        y_superscript_x_size = dis.read<int16_t>();
        y_superscript_y_size = dis.read<int16_t>();
        y_superscript_x_offset = dis.read<int16_t>();
        y_superscript_y_offset = dis.read<int16_t>();
        y_strikeout_size = dis.read<int16_t>();
        y_strikeout_position = dis.read<int16_t>();
        s_family_class = dis.read<int16_t>();

        // panose
        panose.b_family_type = dis.read<uint8_t>();
        panose.b_serif_type = dis.read<uint8_t>();
        panose.b_weight = dis.read<uint8_t>();
        panose.b_proportion = dis.read<uint8_t>();
        panose.b_contrast = dis.read<uint8_t>();
        panose.b_stroke_variation = dis.read<uint8_t>();
        panose.b_arm_style = dis.read<uint8_t>();
        panose.b_letterform = dis.read<uint8_t>();
        panose.b_midline = dis.read<uint8_t>();
        panose.b_x_height = dis.read<uint8_t>();

        // Unicode range
        ul_unicode_range.ul_unicode_range_1 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_2 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_3 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_4 = dis.read<uint32_t>();

        ach_vend_id = dis.read<Tag>();
        fs_selection = dis.read<uint16_t>();
        us_first_char_index = dis.read<uint16_t>();
        us_last_char_index = dis.read<uint16_t>();
        s_typo_ascender = dis.read<int16_t>();
        s_typo_descender = dis.read<int16_t>();
        s_typo_line_gap = dis.read<int16_t>();
        us_win_ascent = dis.read<uint16_t>();
        us_win_descent = dis.read<uint16_t>();

        // Codepage range
        ul_codepage_range.ul_codepage_range_1 = dis.read<uint32_t>();
        ul_codepage_range.ul_codepage_range_2 = dis.read<uint32_t>();

        s_x_height = dis.read<int16_t>();
        s_cap_height = dis.read<int16_t>();
        us_default_char = dis.read<uint16_t>();
        us_break_char = dis.read<uint16_t>();
        us_max_context = dis.read<uint16_t>();
    }
    else
    {
        throw std::runtime_error("Unsupported version of the OS/2 table.");
    }
}

void OS2Table::compile(OutputBuffer& out) const
{
    if (version == 3 || version == 4)
    {
        out.write<uint16_t>(version);
        out.write<int16_t>(x_avg_char_width);
        out.write<uint16_t>(us_weight_class);
        out.write<uint16_t>(us_width_class);
        out.write<uint16_t>(fs_type);
        out.write<int16_t>(y_subscript_x_size);
        out.write<int16_t>(y_subscript_y_size);
        out.write<int16_t>(y_subscript_x_offset);
        out.write<int16_t>(y_subscript_y_offset);
        out.write<int16_t>(y_superscript_x_size);
        out.write<int16_t>(y_superscript_y_size);
        out.write<int16_t>(y_superscript_x_offset);
        out.write<int16_t>(y_superscript_y_offset);
        out.write<int16_t>(y_strikeout_size);
        out.write<int16_t>(y_strikeout_position);
        out.write<int16_t>(s_family_class);

        // panose
        out.write<uint8_t>(panose.b_family_type);
        out.write<uint8_t>(panose.b_serif_type);
        out.write<uint8_t>(panose.b_weight);
        out.write<uint8_t>(panose.b_proportion);
        out.write<uint8_t>(panose.b_contrast);
        out.write<uint8_t>(panose.b_stroke_variation);
        out.write<uint8_t>(panose.b_arm_style);
        out.write<uint8_t>(panose.b_letterform);
        out.write<uint8_t>(panose.b_midline);
        out.write<uint8_t>(panose.b_x_height);

        // unicode range
        out.write<uint32_t>(ul_unicode_range.ul_unicode_range_1);
        out.write<uint32_t>(ul_unicode_range.ul_unicode_range_2);
        out.write<uint32_t>(ul_unicode_range.ul_unicode_range_3);
        out.write<uint32_t>(ul_unicode_range.ul_unicode_range_4);

        out.write<Tag>(ach_vend_id);
        out.write<uint16_t>(fs_selection);
        out.write<uint16_t>(us_first_char_index);
        out.write<uint16_t>(us_last_char_index);
        out.write<int16_t>(s_typo_ascender);
        out.write<int16_t>(s_typo_descender);
        out.write<int16_t>(s_typo_line_gap);
        out.write<uint16_t>(us_win_ascent);
        out.write<uint16_t>(us_win_descent);

        // codepage range
        out.write<uint32_t>(ul_codepage_range.ul_codepage_range_1);
        out.write<uint32_t>(ul_codepage_range.ul_codepage_range_2);

        out.write<int16_t>(s_x_height);
        out.write<int16_t>(s_cap_height);
        out.write<uint16_t>(us_default_char);
        out.write<uint16_t>(us_break_char);
        out.write<uint16_t>(us_max_context);
    }
    else
    {
        throw std::runtime_error("Unsupported version of the OS/2 table.");
    }
}

bool OS2Table::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<OS2Table const&>(rhs);
    return version == other.version
           && x_avg_char_width == other.x_avg_char_width
           && us_weight_class == other.us_weight_class
           && us_width_class == other.us_width_class && fs_type == other.fs_type
           && y_subscript_x_size == other.y_subscript_x_size
           && y_subscript_y_size == other.y_subscript_y_size
           && y_subscript_x_offset == other.y_subscript_x_offset
           && y_subscript_y_offset == other.y_subscript_y_offset
           && y_superscript_x_size == other.y_superscript_x_size
           && y_superscript_y_size == other.y_superscript_y_size
           && y_superscript_x_offset == other.y_superscript_x_offset
           && y_superscript_y_offset == other.y_superscript_y_offset
           && y_strikeout_size == other.y_strikeout_size
           && y_strikeout_position == other.y_strikeout_position
           && s_family_class == other.s_family_class
           && panose.b_family_type == other.panose.b_family_type
           && panose.b_serif_type == other.panose.b_serif_type
           && panose.b_weight == other.panose.b_weight
           && panose.b_proportion == other.panose.b_proportion
           && panose.b_contrast == other.panose.b_contrast
           && panose.b_stroke_variation == other.panose.b_stroke_variation
           && panose.b_arm_style == other.panose.b_arm_style
           && panose.b_letterform == other.panose.b_letterform
           && panose.b_midline == other.panose.b_midline
           && panose.b_x_height == other.panose.b_x_height
           && ul_unicode_range.ul_unicode_range_1
                  == other.ul_unicode_range.ul_unicode_range_1
           && ul_unicode_range.ul_unicode_range_2
                  == other.ul_unicode_range.ul_unicode_range_2
           && ul_unicode_range.ul_unicode_range_3
                  == other.ul_unicode_range.ul_unicode_range_3
           && ul_unicode_range.ul_unicode_range_4
                  == other.ul_unicode_range.ul_unicode_range_4
           && ach_vend_id == other.ach_vend_id
           && fs_selection == other.fs_selection
           && us_first_char_index == other.us_first_char_index
           && us_last_char_index == other.us_last_char_index
           && s_typo_ascender == other.s_typo_ascender
           && s_typo_descender == other.s_typo_descender
           && s_typo_line_gap == other.s_typo_line_gap
           && us_win_ascent == other.us_win_ascent
           && us_win_descent == other.us_win_descent
           && ul_codepage_range.ul_codepage_range_1
                  == other.ul_codepage_range.ul_codepage_range_1
           && ul_codepage_range.ul_codepage_range_2
                  == other.ul_codepage_range.ul_codepage_range_2
           && s_x_height == other.s_x_height
           && s_cap_height == other.s_cap_height
           && us_default_char == other.us_default_char
           && us_break_char == other.us_break_char
           && us_max_context == other.us_max_context;
}
}
