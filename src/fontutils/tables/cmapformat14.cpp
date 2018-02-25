#include "cmapformat14.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

CmapFormat14Subtable::CmapFormat14Subtable(
    uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat14Subtable::parse(Buffer& dis)
{
    auto beginning = dis.tell();

    auto format = dis.read<uint16_t>();
    if (format != 14)
        throw std::runtime_error("Format is not 14");

    // length
    dis.read<uint32_t>();

    auto num_uvs_selectors = dis.read<uint32_t>();
    for (auto i = 0u; i < num_uvs_selectors; ++i)
    {
        char32_t var_selector = dis.read_nbytes(3);

        UVS uvs;

        uint32_t default_uvs_offset = dis.read<uint32_t>();
        if (default_uvs_offset > 0)
        {
            auto orig_pos = dis.seek(beginning + default_uvs_offset);
            auto num_ranges = dis.read<uint32_t>();
            for (auto i = 0u; i < num_ranges; ++i)
            {
                char32_t start_val = dis.read_nbytes(3);
                int      count = dis.read<uint8_t>() + 1;
                uvs.dflt.push_back({ start_val, count });
            }
            dis.seek(orig_pos);
        }

        uint32_t special_uvs_offset = dis.read<uint32_t>();
        if (special_uvs_offset > 0)
        {
            auto orig_pos = dis.seek(beginning + special_uvs_offset);
            auto num_uvs_mappings = dis.read<uint32_t>();
            for (auto i = 0u; i < num_uvs_mappings; ++i)
            {
                char32_t unicode_value = dis.read_nbytes(3);
                auto     gid = dis.read<uint16_t>();
                uvs.special.push_back({ unicode_value, gid });
            }
            dis.seek(orig_pos);
        }

        map.emplace(var_selector, uvs);
    }
}

Buffer CmapFormat14Subtable::compile() const
{
    Buffer buf;

    // Format 14
    buf.add<uint16_t>(14);

    size_t head_length = 10 + 11 * map.size();

    Buffer uvs_buf;
    struct Offsets
    {
        uint32_t dflt = 0, special = 0;
    };
    std::map<char32_t, Offsets> offset_map;
    for (auto const& selector : map)
    {
        UVS const& uvs = selector.second;
        size_t     dflt_size = uvs.dflt.size();
        size_t     special_size = uvs.special.size();
        if (dflt_size > 0)
        {
            offset_map[selector.first].dflt = head_length + uvs_buf.size();
            uvs_buf.add<uint32_t>(dflt_size);
            for (auto const& range : uvs.dflt)
            {
                uvs_buf.add_nbytes(3, range.start_val);
                uvs_buf.add<uint8_t>(range.count - 1);
            }
        }

        if (special_size > 0)
        {
            offset_map[selector.first].special = head_length + uvs_buf.size();
            uvs_buf.add<uint32_t>(special_size);
            for (auto const& mapping : uvs.special)
            {
                uvs_buf.add_nbytes(3, mapping.unicode);
                uvs_buf.add<uint16_t>(mapping.gid);
            }
        }
    }

    // length
    buf.add<uint32_t>(head_length + uvs_buf.size());

    // numVarSelectorRecords
    buf.add<uint32_t>(map.size());
    for (auto const& selector : map)
    {
        // varSelector
        buf.add_nbytes(3, selector.first);

        auto offsets = offset_map[selector.first];

        // defaultUVSOffset
        buf.add<uint32_t>(offsets.dflt);

        // nonDefaultUVSOffset
        buf.add<uint32_t>(offsets.special);
    }

    buf.append(std::move(uvs_buf));

    return buf;
}

bool CmapFormat14Subtable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<CmapFormat14Subtable const&>(rhs);

    return platform_id == other.platform_id && encoding_id == other.encoding_id
           && map == other.map;
}

bool CmapFormat14Subtable::DefaultUVSRange::
     operator==(DefaultUVSRange const& rhs) const noexcept
{
    return start_val == rhs.start_val && count == rhs.count;
}

bool CmapFormat14Subtable::UVSMapping::operator==(UVSMapping const& rhs) const
    noexcept
{
    return unicode == rhs.unicode && gid == rhs.gid;
}

bool CmapFormat14Subtable::UVS::operator==(UVS const& rhs) const noexcept
{
    return dflt == rhs.dflt && special == rhs.special;
}
}
