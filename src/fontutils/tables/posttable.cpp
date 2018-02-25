#include "posttable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

PostTable::PostTable()
    : OTFTable(tag)
{}

void PostTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'post'... " << std::endl;

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

Buffer PostTable::compile() const
{
    Buffer buf;
    buf.add<Fixed>(version);
    buf.add<Fixed>(italic_angle);
    buf.add<int16_t>(underline_position);
    buf.add<int16_t>(underline_thickness);
    buf.add<uint32_t>(is_fixed_pitch);
    buf.add<uint32_t>(min_mem_type_42);
    buf.add<uint32_t>(max_mem_type_42);
    buf.add<uint32_t>(min_mem_type_1);
    buf.add<uint32_t>(max_mem_type_1);
    return buf;
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
