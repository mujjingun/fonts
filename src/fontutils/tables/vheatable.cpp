#include "vheatable.hpp"
#include <cassert>

namespace geul
{

VheaTable::VheaTable()
    : OTFTable(tag)
{}

void VheaTable::parse(InputBuffer& dis)
{
    auto major = dis.read<uint16_t>();
    if (major != 1)
        throw std::runtime_error("Unsupported vhea major version");

    minor_version = dis.read<uint16_t>();

    ascender = dis.read<int16_t>(); // ascent in 1.0
    descender = dis.read<int16_t>(); // descent in 1.0
    line_gap = dis.read<int16_t>();
    adv_height_max = dis.read<int16_t>();
    min_top_side_bearing = dis.read<int16_t>();
    min_bot_side_bearing = dis.read<int16_t>();
    y_max_extent = dis.read<int16_t>();
    caret_slope_rise = dis.read<int16_t>();
    caret_slope_run = dis.read<int16_t>();
    caret_offset = dis.read<int16_t>();

    // reserved fields
    dis.read<int16_t>();
    dis.read<int16_t>();
    dis.read<int16_t>();
    dis.read<int16_t>();
    dis.read<int16_t>();

    num_long_ver_metrics = dis.read<uint16_t>();
}

void VheaTable::compile(OutputBuffer& out) const
{
    out.write<uint16_t>(1);
    out.write<uint16_t>(minor_version);

    out.write<int16_t>(ascender);
    out.write<int16_t>(descender);
    out.write<int16_t>(line_gap);
    out.write<int16_t>(adv_height_max);
    out.write<int16_t>(min_top_side_bearing);
    out.write<int16_t>(min_bot_side_bearing);
    out.write<int16_t>(y_max_extent);
    out.write<int16_t>(caret_slope_rise);
    out.write<int16_t>(caret_slope_run);
    out.write<int16_t>(caret_offset);

    out.write<int16_t>(0);
    out.write<int16_t>(0);
    out.write<int16_t>(0);
    out.write<int16_t>(0);
    out.write<int16_t>(0);

    out.write<uint16_t>(num_long_ver_metrics);
}

bool VheaTable::operator==(const OTFTable& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<VheaTable const&>(rhs);

    return minor_version == other.minor_version && ascender == other.ascender
           && descender == other.descender && line_gap == other.line_gap
           && adv_height_max == other.adv_height_max
           && min_top_side_bearing == other.min_top_side_bearing
           && min_bot_side_bearing == other.min_bot_side_bearing
           && y_max_extent == other.y_max_extent
           && caret_slope_rise == other.caret_slope_rise
           && caret_slope_run == other.caret_slope_run
           && caret_offset == other.caret_offset
           && num_long_ver_metrics == other.num_long_ver_metrics;
}
}
