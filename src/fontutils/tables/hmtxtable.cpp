#include "hmtxtable.hpp"

namespace fontutils
{

HmtxTable::HmtxTable(size_t num_glyphs, size_t num_h_metrics)
    : metrics(num_glyphs), lsbs(num_glyphs - num_h_metrics)
{
    id = "hmtx";
}

void HmtxTable::parse(Buffer &dis)
{
    for (auto &metric : metrics)
    {
        metric.advance_width = dis.read<uint16_t>();
        metric.lsb = dis.read<int16_t>();
    }

    for (auto &lsb : lsbs)
    {
        lsb = dis.read<int16_t>();
    }
}

Buffer HmtxTable::compile() const
{
    Buffer buf;
    for (auto const &metric : metrics)
    {
        buf.add<uint16_t>(metric.advance_width);
        buf.add<int16_t>(metric.lsb);
    }

    for (auto const &lsb : lsbs)
    {
        buf.add<int16_t>(lsb);
    }

    buf.pad();

    return buf;
}

}
