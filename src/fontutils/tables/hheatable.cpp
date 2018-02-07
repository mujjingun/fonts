#include "hheatable.hpp"

namespace fontutils
{

HheaTable::HheaTable()
{
    id = "hhea";
}

void HheaTable::parse(Buffer &dis)
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

}
