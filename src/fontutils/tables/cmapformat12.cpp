#include "cmapformat12.hpp"

#include <vector>

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

    // reserved
    if (dis.read<uint16_t>() != 0)
        throw std::runtime_error("Reserved field not 0");

    // length
    dis.read<uint32_t>();
    // language
    language = dis.read<uint32_t>();

    auto num_groups = dis.read<uint32_t>();
    for (auto i = 0u; i < num_groups; ++i)
    {
        uint32_t start_char_code = dis.read<uint32_t>();
        uint32_t end_char_code = dis.read<uint32_t>();
        uint32_t start_glyph_id = dis.read<uint32_t>();

        for (auto c = start_char_code; c <= end_char_code; ++c)
        {
            cmap[c] = start_glyph_id++;
        }
    }
}

Buffer CmapFormat12Subtable::compile() const
{
    Buffer buf;

    // Format 12
    buf.add<uint16_t>(12);

    // reserved
    buf.add<uint16_t>(0);

    struct SequentialMapGroup
    {
        uint32_t start_char_code;
        uint32_t end_char_code;
        uint32_t start_glyph_id;
    };
    std::vector<SequentialMapGroup> group_list;

    uint32_t begin = cmap.begin()->first, last = begin;
    uint32_t begin_gid = cmap.begin()->second, last_gid = begin_gid;
    for (auto it = std::next(cmap.begin());; ++it)
    {
        uint32_t code, gid;

        if (it != cmap.end())
        {
            code = it->first;
            gid = it->second;
        }

        if (it == cmap.end() || (last + 1 != code || last_gid + 1 != gid))
        {
            SequentialMapGroup group;
            group.start_char_code = begin;
            group.end_char_code = last;
            group.start_glyph_id = begin_gid;
            group_list.push_back(group);

            if (it == cmap.end()) break;

            begin = code;
            begin_gid = gid;
        }

        last = code;
        last_gid = gid;
    }

    auto length = 16 + group_list.size() * 12;
    buf.add<uint32_t>(length);
    buf.add<uint32_t>(language);
    buf.add<uint32_t>(group_list.size());

    for (auto const& group : group_list)
    {
        buf.add<uint32_t>(group.start_char_code);
        buf.add<uint32_t>(group.end_char_code);
        buf.add<uint32_t>(group.start_glyph_id);
    }

    return buf;
}

}
