#include "cmapformat12.hpp"

namespace fontutils
{

CmapFormat12Subtable::CmapFormat12Subtable(uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat12Subtable::parse(Buffer &dis)
{
    auto format = dis.read<uint16_t>();
    if (format != 12)
        throw std::runtime_error("Format is not 12");
}

Buffer CmapFormat12Subtable::compile() const
{
    // TODO
    Buffer buf;
    return buf;
}

}
