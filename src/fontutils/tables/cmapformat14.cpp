#include "cmapformat14.hpp"

namespace fontutils
{

CmapFormat14Subtable::CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat14Subtable::parse(Buffer &dis)
{
    auto format = dis.read<uint16_t>();
    if (format != 14)
        throw std::runtime_error("Format is not 14");

    auto length = dis.read<uint32_t>();
    auto num_uvs_selectors = dis.read<uint32_t>();

    for (auto i = 0u; i < num_uvs_selectors; ++i)
    {
        // TODO
    }
}

Buffer CmapFormat14Subtable::compile() const
{
    // TODO
    Buffer buf;
    return buf;
}

}
