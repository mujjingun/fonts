#include "hheatable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

HheaTable::HheaTable()
    : OTFTable(tag)
{}

void HheaTable::parse(InputBuffer& dis)
{
    auto major_version = dis.read<uint16_t>();
    if (major_version != 1)
        throw std::runtime_error("Unrecognized head table major version");
    auto minor_version = dis.read<uint16_t>();
    if (minor_version != 0)
        throw std::runtime_error("Unrecognized head table minor version");
    ascender = dis.read<int16_t>();
    descender = dis.read<int16_t>();
    line_gap = dis.read<int16_t>();
    advance_width_max = dis.read<uint16_t>();
    min_lsb = dis.read<int16_t>();
    max_lsb = dis.read<int16_t>();
    x_max_extent = dis.read<int16_t>();
    caret_slope_rise = dis.read<int16_t>();
    caret_slope_run = dis.read<int16_t>();
    caret_offset = dis.read<int16_t>();

    // reserved
    dis.read<int16_t>();
    dis.read<int16_t>();
    dis.read<int16_t>();
    dis.read<int16_t>();

    metric_data_format = dis.read<int16_t>();
    if (metric_data_format != 0)
        throw std::runtime_error("Unrecognized metric data format");
    num_h_metrics = dis.read<uint16_t>();
}

void HheaTable::compile(OutputBuffer& out) const
{
    // version
    out.write<uint16_t>(1);
    out.write<uint16_t>(0);

    out.write<int16_t>(ascender);
    out.write<int16_t>(descender);
    out.write<int16_t>(line_gap);
    out.write<uint16_t>(advance_width_max);
    out.write<int16_t>(min_lsb);
    out.write<int16_t>(max_lsb);
    out.write<int16_t>(x_max_extent);
    out.write<int16_t>(caret_slope_rise);
    out.write<int16_t>(caret_slope_run);
    out.write<int16_t>(caret_offset);

    // reserved
    out.write<int16_t>(0);
    out.write<int16_t>(0);
    out.write<int16_t>(0);
    out.write<int16_t>(0);

    out.write<int16_t>(metric_data_format);
    out.write<uint16_t>(num_h_metrics);
}

bool HheaTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<HheaTable const&>(rhs);
    return ascender == other.ascender && descender == other.descender
           && line_gap == other.line_gap
           && advance_width_max == other.advance_width_max
           && min_lsb == other.min_lsb && max_lsb == other.max_lsb
           && x_max_extent == other.x_max_extent
           && caret_slope_rise == other.caret_slope_rise
           && caret_slope_run == other.caret_slope_run
           && caret_offset == other.caret_offset
           && metric_data_format == other.metric_data_format
           && num_h_metrics == other.num_h_metrics;
}
}
