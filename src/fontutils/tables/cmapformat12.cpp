#include "cmapformat12.hpp"

#include <cassert>
#include <typeinfo>
#include <vector>

namespace geul
{

CmapFormat12Subtable::CmapFormat12Subtable(
    uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat12Subtable::parse(InputBuffer& dis)
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

void CmapFormat12Subtable::compile(OutputBuffer& out) const
{
    // Format 12
    out.write<uint16_t>(12);

    // reserved
    out.write<uint16_t>(0);

    struct SequentialMapGroup
    {
        uint32_t start_char_code;
        uint32_t end_char_code;
        uint32_t start_glyph_id;
    };
    std::vector<SequentialMapGroup> group_list;

    uint32_t begin = cmap.begin()->first, last = begin;
    uint32_t begin_gid = cmap.begin()->second, last_gid = begin_gid;
    for (auto it = std::next(cmap.begin()); it != cmap.end(); ++it)
    {
        uint32_t code, gid;

        code = it->first;
        gid = it->second;

        if (last + 1 != code || last_gid + 1 != gid)
        {
            SequentialMapGroup group;
            group.start_char_code = begin;
            group.end_char_code = last;
            group.start_glyph_id = begin_gid;
            group_list.push_back(group);

            begin = code;
            begin_gid = gid;
        }

        last = code;
        last_gid = gid;
    }
    group_list.push_back({ begin, last, begin_gid });

    auto length = 16 + group_list.size() * 12;
    out.write<uint32_t>(length);
    out.write<uint32_t>(language);
    out.write<uint32_t>(group_list.size());

    for (auto const& group : group_list)
    {
        out.write<uint32_t>(group.start_char_code);
        out.write<uint32_t>(group.end_char_code);
        out.write<uint32_t>(group.start_glyph_id);
    }
}

bool CmapFormat12Subtable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<CmapFormat12Subtable const&>(rhs);

    return platform_id == other.platform_id && encoding_id == other.encoding_id
           && cmap == other.cmap;
}
}
