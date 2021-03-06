#ifndef TABLES_HHEA_TABLE_HPP
#define TABLES_HHEA_TABLE_HPP

#include "otftable.hpp"

namespace geul
{

class HheaTable : public OTFTable
{
public:
    int16_t  ascender;
    int16_t  descender;
    int16_t  line_gap;
    uint16_t advance_width_max;
    int16_t  min_lsb;
    int16_t  max_lsb;
    int16_t  x_max_extent;
    int16_t  caret_slope_rise;
    int16_t  caret_slope_run;
    int16_t  caret_offset;
    int16_t  metric_data_format;
    uint16_t num_h_metrics;

public:
    HheaTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "hhea";
};
}

#endif
