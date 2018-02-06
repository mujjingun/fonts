#include "posttable.hpp"

namespace fontutils
{

PostTable::PostTable()
{
    id = "post";
}

void PostTable::parse(Buffer &dis)
{
    version = dis.read<Fixed>();
    italic_angle = dis.read<Fixed>();
    underline_position = dis.read<int16_t>();
    underline_thickness = dis.read<int16_t>();
    is_fixed_pitch = dis.read<uint32_t>();
    min_mem_type_42 = dis.read<uint32_t>();
    max_mem_type_42 = dis.read<uint32_t>();
    min_mem_type_1  = dis.read<uint32_t>();
    max_mem_type_1  = dis.read<uint32_t>();
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
    buf.pad();
    return buf;
}

}
