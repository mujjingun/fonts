#include "os2table.hpp"

namespace fontutils
{

OS2Table::OS2Table()
{
    id = "OS/2";
}

void OS2Table::parse(Buffer &dis)
{
    version = dis.read<uint16_t>();
    if (version == 3 || version == 4)
    {
        x_avg_char_width = dis.read<int16_t>();
        us_weight_class = dis.read<uint16_t>();
        us_width_class = dis.read<uint16_t>();
        fs_type = dis.read<uint16_t>();
        y_subscript_x_size     = dis.read<int16_t>();
        y_subscript_y_size     = dis.read<int16_t>();
        y_subscript_x_offset   = dis.read<int16_t>();
        y_subscript_y_offset   = dis.read<int16_t>();
        y_superscript_x_size   = dis.read<int16_t>();
        y_superscript_y_size   = dis.read<int16_t>();
        y_superscript_x_offset = dis.read<int16_t>();
        y_superscript_y_offset = dis.read<int16_t>();
        y_strikeout_size       = dis.read<int16_t>();
        y_strikeout_position   = dis.read<int16_t>();
        s_family_class         = dis.read<int16_t>();

        // panose
        panose.b_family_type      = dis.read<uint8_t>();
        panose.b_serif_type       = dis.read<uint8_t>();
        panose.b_weight           = dis.read<uint8_t>();
        panose.b_proportion       = dis.read<uint8_t>();
        panose.b_contrast         = dis.read<uint8_t>();
        panose.b_stroke_variation = dis.read<uint8_t>();
        panose.b_arm_style        = dis.read<uint8_t>();
        panose.b_letterform       = dis.read<uint8_t>();
        panose.b_midline          = dis.read<uint8_t>();
        panose.b_x_height         = dis.read<uint8_t>();

        // Unicode range
        ul_unicode_range.ul_unicode_range_1 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_2 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_3 = dis.read<uint32_t>();
        ul_unicode_range.ul_unicode_range_4 = dis.read<uint32_t>();

        ach_vend_id = dis.read<Tag>();
        fs_selection = dis.read<uint16_t>();
        us_first_char_index = dis.read<uint16_t>();
        us_last_char_index = dis.read<uint16_t>();
        s_typo_ascender  = dis.read<int16_t>();
        s_typo_descender = dis.read<int16_t>();
        s_typo_line_gap  = dis.read<int16_t>();
        us_win_ascent = dis.read<uint16_t>();
        us_win_descent = dis.read<uint16_t>();

        // Codepage range
        ul_codepage_range.ul_codepage_range_1 = dis.read<uint32_t>();
        ul_codepage_range.ul_codepage_range_2 = dis.read<uint32_t>();

        s_x_height = dis.read<int16_t>();
        s_cap_height = dis.read<int16_t>();
        us_default_char = dis.read<uint16_t>();
        us_break_char   = dis.read<uint16_t>();
        us_max_context  = dis.read<uint16_t>();
    }
    else
        throw std::runtime_error("Unsupported version of the OS/2 table.");
}

Buffer OS2Table::compile() const
{
    Buffer buf;

    if (version == 3 || version == 4)
    {
        buf.add<uint16_t>(version);
        buf.add<int16_t>(x_avg_char_width);
        buf.add<uint16_t>(us_weight_class);
        buf.add<uint16_t>(us_width_class);
        buf.add<uint16_t>(fs_type);
        buf.add<int16_t>(y_subscript_x_size);
        buf.add<int16_t>(y_subscript_y_size);
        buf.add<int16_t>(y_subscript_x_offset);
        buf.add<int16_t>(y_subscript_y_offset);
        buf.add<int16_t>(y_superscript_x_size);
        buf.add<int16_t>(y_superscript_y_size);
        buf.add<int16_t>(y_superscript_x_offset);
        buf.add<int16_t>(y_superscript_y_offset);
        buf.add<int16_t>(y_strikeout_size);
        buf.add<int16_t>(y_strikeout_position);
        buf.add<int16_t>(s_family_class);

        // panose
        buf.add<uint8_t>(panose.b_family_type);
        buf.add<uint8_t>(panose.b_serif_type);
        buf.add<uint8_t>(panose.b_weight);
        buf.add<uint8_t>(panose.b_proportion);
        buf.add<uint8_t>(panose.b_contrast);
        buf.add<uint8_t>(panose.b_stroke_variation);
        buf.add<uint8_t>(panose.b_arm_style);
        buf.add<uint8_t>(panose.b_letterform);
        buf.add<uint8_t>(panose.b_midline);
        buf.add<uint8_t>(panose.b_x_height);

        // unicode range
        buf.add<uint32_t>(ul_unicode_range.ul_unicode_range_1);
        buf.add<uint32_t>(ul_unicode_range.ul_unicode_range_2);
        buf.add<uint32_t>(ul_unicode_range.ul_unicode_range_3);
        buf.add<uint32_t>(ul_unicode_range.ul_unicode_range_4);

        buf.add<Tag>(ach_vend_id);
        buf.add<uint16_t>(fs_selection);
        buf.add<uint16_t>(us_first_char_index);
        buf.add<uint16_t>(us_last_char_index);
        buf.add<int16_t>(s_typo_ascender);
        buf.add<int16_t>(s_typo_descender);
        buf.add<int16_t>(s_typo_line_gap);
        buf.add<uint16_t>(us_win_ascent);
        buf.add<uint16_t>(us_win_descent);

        // codepage range
        buf.add<uint32_t>(ul_codepage_range.ul_codepage_range_1);
        buf.add<uint32_t>(ul_codepage_range.ul_codepage_range_2);

        buf.add<int16_t>(s_x_height);
        buf.add<int16_t>(s_cap_height);
        buf.add<uint16_t>(us_default_char);
        buf.add<uint16_t>(us_break_char);
        buf.add<uint16_t>(us_max_context);
    }
    else
        throw std::runtime_error("Unsupported version of the OS/2 table.");

    return buf;
}

}
