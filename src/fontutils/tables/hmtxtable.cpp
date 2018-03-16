#include "hmtxtable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

HmtxTable::HmtxTable(std::size_t num_glyphs, std::size_t num_h_metrics)
    : OTFTable(tag)
    , metrics(num_h_metrics)
    , lsbs(num_glyphs - num_h_metrics)
{}

void HmtxTable::parse(InputBuffer& dis)
{
    for (auto& metric : metrics)
    {
        metric.advance_width = dis.read<uint16_t>();
        metric.lsb = dis.read<int16_t>();
    }

    dis.read<int16_t>(lsbs.data(), lsbs.size());
}

void HmtxTable::compile(OutputBuffer& out) const
{
    for (auto const& metric : metrics)
    {
        out.write<uint16_t>(metric.advance_width);
        out.write<int16_t>(metric.lsb);
    }

    out.write(lsbs.data(), lsbs.size());
}

bool HmtxTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<HmtxTable const&>(rhs);
    return metrics == other.metrics && lsbs == other.lsbs;
}

bool HmtxTable::HMetric::operator==(HMetric const& rhs) const noexcept
{
    return advance_width == rhs.advance_width && lsb == rhs.lsb;
}
}
