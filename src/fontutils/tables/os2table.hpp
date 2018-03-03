#ifndef TABLE_OS_2_TABLE_HPP
#define TABLE_OS_2_TABLE_HPP

#include "otftable.hpp"
#include "../types.hpp"

namespace geul
{

class OS2Table : public OTFTable
{
public:
    uint16_t version;
    int16_t  x_avg_char_width;
    uint16_t us_weight_class;
    uint16_t us_width_class;
    uint16_t fs_type;
    int16_t  y_subscript_x_size;
    int16_t  y_subscript_y_size;
    int16_t  y_subscript_x_offset;
    int16_t  y_subscript_y_offset;
    int16_t  y_superscript_x_size;
    int16_t  y_superscript_y_size;
    int16_t  y_superscript_x_offset;
    int16_t  y_superscript_y_offset;
    int16_t  y_strikeout_size;
    int16_t  y_strikeout_position;
    int16_t  s_family_class;
    struct Panose
    {
        uint8_t b_family_type;
        uint8_t b_serif_type;
        uint8_t b_weight;
        uint8_t b_proportion;
        uint8_t b_contrast;
        uint8_t b_stroke_variation;
        uint8_t b_arm_style;
        uint8_t b_letterform;
        uint8_t b_midline;
        uint8_t b_x_height;
    } panose;
    struct UnicodeRange
    {
        uint32_t ul_unicode_range_1;
        uint32_t ul_unicode_range_2;
        uint32_t ul_unicode_range_3;
        uint32_t ul_unicode_range_4;
    } ul_unicode_range;
    Tag      ach_vend_id;
    uint16_t fs_selection;
    uint16_t us_first_char_index;
    uint16_t us_last_char_index;
    int16_t  s_typo_ascender;
    int16_t  s_typo_descender;
    int16_t  s_typo_line_gap;
    uint16_t us_win_ascent;
    uint16_t us_win_descent;
    struct CodepageRange
    {
        uint32_t ul_codepage_range_1;
        uint32_t ul_codepage_range_2;
    } ul_codepage_range;
    int16_t  s_x_height;
    int16_t  s_cap_height;
    uint16_t us_default_char;
    uint16_t us_break_char;
    uint16_t us_max_context;

public:
    OS2Table();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "OS/2";
};
}

#endif
