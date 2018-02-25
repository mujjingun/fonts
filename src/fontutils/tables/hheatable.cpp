#include "hheatable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

HheaTable::HheaTable()
    : OTFTable(tag)
{}

void HheaTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'hhea'... " << std::endl;

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

Buffer HheaTable::compile() const
{
    Buffer buf;

    // version
    buf.add<uint16_t>(1);
    buf.add<uint16_t>(0);

    buf.add<int16_t>(ascender);
    buf.add<int16_t>(descender);
    buf.add<int16_t>(line_gap);
    buf.add<uint16_t>(advance_width_max);
    buf.add<int16_t>(min_lsb);
    buf.add<int16_t>(max_lsb);
    buf.add<int16_t>(x_max_extent);
    buf.add<int16_t>(caret_slope_rise);
    buf.add<int16_t>(caret_slope_run);
    buf.add<int16_t>(caret_offset);

    // reserved
    buf.add<int16_t>(0);
    buf.add<int16_t>(0);
    buf.add<int16_t>(0);
    buf.add<int16_t>(0);

    buf.add<int16_t>(metric_data_format);
    buf.add<uint16_t>(num_h_metrics);

    return buf;
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
