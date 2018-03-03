#include "posttable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

PostTable::PostTable()
    : OTFTable(tag)
{}

void PostTable::parse(InputBuffer& dis)
{
    version = dis.read<Fixed>();
    italic_angle = dis.read<Fixed>();
    underline_position = dis.read<int16_t>();
    underline_thickness = dis.read<int16_t>();
    is_fixed_pitch = dis.read<uint32_t>();
    min_mem_type_42 = dis.read<uint32_t>();
    max_mem_type_42 = dis.read<uint32_t>();
    min_mem_type_1 = dis.read<uint32_t>();
    max_mem_type_1 = dis.read<uint32_t>();
}

void PostTable::compile(OutputBuffer& out) const
{
    out.write<Fixed>(version);
    out.write<Fixed>(italic_angle);
    out.write<int16_t>(underline_position);
    out.write<int16_t>(underline_thickness);
    out.write<uint32_t>(is_fixed_pitch);
    out.write<uint32_t>(min_mem_type_42);
    out.write<uint32_t>(max_mem_type_42);
    out.write<uint32_t>(min_mem_type_1);
    out.write<uint32_t>(max_mem_type_1);
}

bool PostTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<PostTable const&>(rhs);
    return version == other.version && italic_angle == other.italic_angle
           && underline_position == other.underline_position
           && underline_thickness == other.underline_thickness
           && is_fixed_pitch == other.is_fixed_pitch
           && min_mem_type_42 == other.min_mem_type_42
           && max_mem_type_42 == other.max_mem_type_42
           && min_mem_type_1 == other.min_mem_type_1
           && max_mem_type_1 == other.max_mem_type_1;
}
}
