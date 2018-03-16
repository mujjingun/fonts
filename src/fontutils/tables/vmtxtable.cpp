#include "vmtxtable.hpp"

#include <cassert>

namespace geul
{
VmtxTable::VmtxTable(std::size_t num_glyphs, std::size_t num_v_metrics)
    : OTFTable(tag)
    , advance_height(num_glyphs - num_v_metrics)
{}

void VmtxTable::parse(InputBuffer& dis)
{
    advance_height = dis.read<uint16_t>();
    for (auto& tsb : top_side_bearings)
    {
        tsb = dis.read<int16_t>();
    }
}

void VmtxTable::compile(OutputBuffer& out) const
{
    out.write<uint16_t>(advance_height);
    for (auto tsb : top_side_bearings)
    {
        out.write<int16_t>(tsb);
    }
}

bool VmtxTable::operator==(const OTFTable& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<VmtxTable const&>(rhs);

    return advance_height == other.advance_height
           && top_side_bearings == other.top_side_bearings;
}
}
