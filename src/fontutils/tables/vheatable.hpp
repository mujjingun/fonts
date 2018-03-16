#ifndef TABLES_VHEATABLE_HPP
#define TABLES_VHEATABLE_HPP

#include "otftable.hpp"

namespace geul
{

class VheaTable : public OTFTable
{
public:
    uint16_t minor_version;

    int16_t ascender;
    int16_t descender;
    int16_t line_gap;
    int16_t adv_height_max;
    int16_t min_top_side_bearing;
    int16_t min_bot_side_bearing;
    int16_t y_max_extent;
    int16_t caret_slope_rise;
    int16_t caret_slope_run;
    int16_t caret_offset;
    uint16_t num_long_ver_metrics;

public:
    VheaTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "vhea";
};
}

#endif
