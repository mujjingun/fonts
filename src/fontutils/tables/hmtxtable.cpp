#include "hmtxtable.hpp"

#include <typeinfo>
#include <cassert>

namespace fontutils
{

HmtxTable::HmtxTable(size_t num_glyphs, size_t num_h_metrics)
    : OTFTable("hmtx")
    , metrics(num_h_metrics)
    , lsbs(num_glyphs - num_h_metrics)
{
}

void HmtxTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'hmtx'... " << std::endl;

    for (auto& metric : metrics)
    {
        metric.advance_width = dis.read<uint16_t>();
        metric.lsb = dis.read<int16_t>();
    }

    dis.read<int16_t>(lsbs.data(), lsbs.size());
}

Buffer HmtxTable::compile() const
{
    Buffer buf;
    for (auto const& metric : metrics)
    {
        buf.add<uint16_t>(metric.advance_width);
        buf.add<int16_t>(metric.lsb);
    }

    buf.add(lsbs.data(), lsbs.size());

    return buf;
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
