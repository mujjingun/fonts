#include "cmapformat4.hpp"

#include <vector>

namespace fontutils
{

CmapFormat4Subtable::CmapFormat4Subtable(uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{
}

void CmapFormat4Subtable::parse(Buffer &dis)
{
    auto format = dis.read<uint16_t>();
    if (format != 4)
        throw std::runtime_error("Format is not 4");

    length = dis.read<uint16_t>();
    language = dis.read<uint16_t>();

    auto seg_count = dis.read<uint16_t>() / 2;

    // searchRange
    dis.read<uint16_t>();
    // entrySelector
    dis.read<uint16_t>();
    // rangeShift
    dis.read<uint16_t>();

    // Ending character code for each segment, last = 0xFFFF.
    std::vector<size_t> end_code(seg_count);
    for (auto &s : end_code)
    {
        s = dis.read<uint16_t>();
    }

    // reservedPad (Should be zero)
    if (dis.read<uint16_t>() != 0)
        throw std::runtime_error("Reserved pad is not zero");

    // Starting character code for each segment
    std::vector<size_t> start_code(seg_count);
    for (auto &s : start_code)
    {
        s = dis.read<uint16_t>();
    }

    std::vector<size_t> id_delta(seg_count);
    for (auto &s : id_delta)
    {
        s = dis.read<uint16_t>();
    }

    std::vector<size_t> offsets(seg_count);
    for (auto &s : offsets)
    {
        s = dis.read<uint16_t>();
    }


}

Buffer CmapFormat4Subtable::compile() const
{
    // TODO
    Buffer buf;
    return buf;
}

}
