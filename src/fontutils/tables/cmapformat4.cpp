#include "cmapformat4.hpp"

#include <vector>
#include <unordered_map>
#include <set>
#include <cmath>

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

    auto length = dis.read<uint16_t>();
    language = dis.read<uint16_t>();

    int seg_count = dis.read<uint16_t>() / 2;

    // searchRange
    dis.read<uint16_t>();
    // entrySelector
    dis.read<uint16_t>();
    // rangeShift
    dis.read<uint16_t>();

    // Ending character code for each segment, last = 0xFFFF.
    std::vector<uint16_t> end_code(seg_count);
    dis.read<uint16_t>(end_code.data(), seg_count);
    if (end_code[seg_count - 1] != 0xFFFF)
        throw std::runtime_error("Last end code is not 0xFFFF");

    // reservedPad (Should be zero)
    if (dis.read<uint16_t>() != 0)
        throw std::runtime_error("Reserved pad is not zero");

    // Starting character code for each segment
    std::vector<uint16_t> start_code(seg_count);
    dis.read<uint16_t>(start_code.data(), seg_count);
    if (start_code[seg_count - 1] != 0xFFFF)
        throw std::runtime_error("Last start code is not 0xFFFF");

    // Used when idRangeOffset == 0
    std::vector<uint16_t> id_delta(seg_count);
    dis.read<uint16_t>(id_delta.data(), seg_count);

    // idRangeOffset
    std::vector<uint16_t> id_range_offset(seg_count);
    std::vector<size_t> range_offset_start(seg_count);
    for (int i = 0; i < seg_count; ++i)
    {
        range_offset_start[i] = dis.tell();
        id_range_offset[i] = dis.read<uint16_t>();
    }

    // glyphIdArray
    size_t gid_len = (length - 16 - 8 * seg_count) / 2;
    std::vector<uint16_t> gid_array(gid_len);
    dis.read<uint16_t>(gid_array.data(), gid_len);

    // for each segment
    for (int i = 0; i < seg_count - 1; ++i)
    {
        // for each code point in segment
        uint16_t j = 0, c;
        do
        {
            c = start_code[i] + j;
            uint16_t gid;
            if (id_range_offset[i] == 0)
            {
                // modulo 65536
                gid = start_code[i] + id_delta[i] + j;
            }
            else
            {
                size_t id = (i - seg_count) + id_range_offset[i] / 2 + j;
                if (id >= gid_len)
                    throw std::runtime_error("id out of glyph id array bounds");

                if (id == 0)
                {
                    // missing glyph
                    gid = 0;
                }
                else
                {
                    // modulo 65536
                    gid = gid_array[id] + id_delta[i];
                }
            }
            if (gid != 0) cmap[c] = gid;
            ++j;
        } while (c != end_code[i]);
    }
}

Buffer CmapFormat4Subtable::compile() const
{
    Buffer buf;

    // Format 4
    buf.add<uint16_t>(4);

    struct Segment
    {
        uint16_t start_code;
        uint16_t end_code;
        uint16_t id_delta;
        uint16_t id_range_offset;
    };
    std::vector<Segment> seg_list;

    std::vector<uint16_t> gid_array;
    gid_array.push_back(0);

    int begin = cmap.begin()->first, last = begin;
    int offset = uint16_t(cmap.begin()->second - begin);
    for (auto it = std::next(cmap.begin());; ++it)
    {
        int code, gid;

        if (it != cmap.end())
        {
            code = it->first;
            gid = it->second;
        }

        if (it == cmap.end() || last + 1 != code)
        {
            Segment seg;
            seg.start_code = begin;
            seg.end_code = last;

            // All glyphs in the range has
            // the same offset
            if (offset >= 0)
            {
                seg.id_delta = offset;
                seg.id_range_offset = 0;
            }
            else
            {
                seg.id_delta = 0;

                int id = gid_array.size();
                int i = seg_list.size();

                // add seg_count*2 later
                seg.id_range_offset = (id - i) * 2;

                for (int j = begin; j <= last; ++j)
                    gid_array.push_back(cmap.at(j));
            }

            seg_list.push_back(seg);

            if (it == cmap.end()) break;

            begin = code;
            offset = uint16_t(gid - code);
        }

        last = code;
        if (offset != uint16_t(gid - code))
            offset = -1;
    }

    seg_list.push_back({0xFFFF, 0xFFFF, 0, 0});
    for (auto &seg : seg_list)
    {
        if (seg.id_delta == 0)
            seg.id_range_offset += seg_list.size() * 2;
    }

    size_t length = 16 + 8 * seg_list.size() + 2 * gid_array.size();

    if (length >= 65535)
        throw std::runtime_error("subtable too long");

    buf.add<uint16_t>(length);
    buf.add<uint16_t>(language);
    buf.add<uint16_t>(seg_list.size() * 2);

    // searchRange
    auto search_range = 2 * le_pow2(seg_list.size());
    buf.add<uint16_t>(search_range);
    // entrySelector
    buf.add<uint16_t>(std::ilogb(seg_list.size()));
    // rangeShift
    buf.add<uint16_t>(2 * seg_list.size() - search_range);

    for (auto const &seg : seg_list)
        buf.add<uint16_t>(seg.end_code);

    // reservedPad
    buf.add<uint16_t>(0);

    for (auto const &seg : seg_list)
        buf.add<uint16_t>(seg.start_code);

    for (auto const &seg : seg_list)
        buf.add<uint16_t>(seg.id_delta);

    for (auto const &seg : seg_list)
        buf.add<uint16_t>(seg.id_range_offset);

    buf.add<uint16_t>(gid_array.data(), gid_array.size());

    return buf;
}

}
