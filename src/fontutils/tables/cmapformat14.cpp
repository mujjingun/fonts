#include "cmapformat14.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

CmapFormat14Subtable::CmapFormat14Subtable(
    uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat14Subtable::parse(InputBuffer& dis)
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
        char32_t var_selector = dis.read_nint(3);

        UVS uvs;

        std::streamoff default_uvs_offset = dis.read<uint32_t>();
        if (default_uvs_offset > 0)
        {
            auto lock = dis.seek_lock(beginning + default_uvs_offset);
            auto num_ranges = dis.read<uint32_t>();
            for (auto i = 0u; i < num_ranges; ++i)
            {
                char32_t start_val = dis.read_nint(3);
                int      count = dis.read<uint8_t>() + 1;
                uvs.dflt.push_back({ start_val, count });
            }
        }

        std::streamoff special_uvs_offset = dis.read<uint32_t>();
        if (special_uvs_offset > 0)
        {
            auto lock = dis.seek_lock(beginning + special_uvs_offset);
            auto num_uvs_mappings = dis.read<uint32_t>();
            for (auto i = 0u; i < num_uvs_mappings; ++i)
            {
                char32_t unicode_value = dis.read_nint(3);
                auto     gid = dis.read<uint16_t>();
                uvs.special.push_back({ unicode_value, gid });
            }
        }

        map.emplace(var_selector, uvs);
    }
}

void CmapFormat14Subtable::compile(OutputBuffer& out) const
{
    auto beginning = out.tell();

    // Format 14
    out.write<uint16_t>(14);

    // placeholder for length
    auto length_pos = out.tell();
    out.write<uint32_t>(0);

    // numVarSelectorRecords
    out.write<uint32_t>(map.size());

    std::map<char32_t, std::streampos> offsets;

    for (auto const& selector : map)
    {
        // varSelector
        out.write_nint(3, selector.first);

        offsets[selector.first] = out.tell();

        // placeholder for defaultUVSOffset
        out.write<uint32_t>(0);

        // placeholder for nonDefaultUVSOffset
        out.write<uint32_t>(0);
    }

    for (auto const& selector : map)
    {
        char32_t   ch = selector.first;
        UVS const& uvs = selector.second;
        std::size_t     dflt_size = uvs.dflt.size();
        std::size_t     special_size = uvs.special.size();

        if (dflt_size > 0)
        {
            out.write_at<uint32_t>(offsets[ch], out.tell() - beginning);

            out.write<uint32_t>(dflt_size);
            for (auto const& range : uvs.dflt)
            {
                out.write_nint(3, range.start_val);
                out.write<uint8_t>(range.count - 1);
            }
        }

        if (special_size > 0)
        {
            out.write_at<uint32_t>(
                offsets[ch] + std::streamoff(4), out.tell() - beginning);

            out.write<uint32_t>(special_size);
            for (auto const& mapping : uvs.special)
            {
                out.write_nint(3, mapping.unicode);
                out.write<uint16_t>(mapping.gid);
            }
        }
    }

    out.write_at<uint32_t>(length_pos, out.tell() - beginning);
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
