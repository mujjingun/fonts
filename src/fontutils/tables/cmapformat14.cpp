#include "cmapformat14.hpp"

namespace fontutils
{

CmapFormat14Subtable::CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id)
    : CmapSubtable(platform_id, encoding_id)
{}

void CmapFormat14Subtable::parse(Buffer &dis)
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
        auto vs0 = dis.read<uint8_t>() & 0xff;
        auto vs1 = dis.read<uint8_t>() & 0xff;
        auto vs2 = dis.read<uint8_t>() & 0xff;
        char32_t var_selector = vs0 << 16 | vs1 << 8 | vs2;

        UVS uvs;

        uint32_t default_uvs_offset = dis.read<uint32_t>();
        if (default_uvs_offset)
        {
            auto orig_pos = dis.seek(beginning + default_uvs_offset);
            auto num_ranges = dis.read<uint32_t>();
            for (auto i = 0u; i < num_ranges; ++i)
            {
                auto sv0 = dis.read<uint8_t>() & 0xff;
                auto sv1 = dis.read<uint8_t>() & 0xff;
                auto sv2 = dis.read<uint8_t>() & 0xff;
                char32_t start_val = sv0 << 16 | sv1 << 8 | sv2;
                int count = dis.read<uint8_t>() + 1;
                uvs.dflt.push_back({start_val, count});
            }
            dis.seek(orig_pos);
        }

        uint32_t special_uvs_offset = dis.read<uint32_t>();
        if (special_uvs_offset)
        {
            auto orig_pos = dis.seek(beginning + special_uvs_offset);
            auto num_uvs_mappings = dis.read<uint32_t>();
            for (auto i = 0u; i < num_uvs_mappings; ++i)
            {
                auto un0 = dis.read<uint8_t>() & 0xff;
                auto un1 = dis.read<uint8_t>() & 0xff;
                auto un2 = dis.read<uint8_t>() & 0xff;
                char32_t unicode_value = un0 << 16 | un1 << 8 | un2;
                auto gid = dis.read<uint16_t>();
                uvs.special.push_back({unicode_value, gid});
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
        size_t dflt_size = uvs.dflt.size();
        size_t special_size = uvs.special.size();
        if (dflt_size)
        {
            offset_map[selector.first].dflt = head_length + uvs_buf.size();
            uvs_buf.add<uint32_t>(dflt_size);
            for (auto const& range : uvs.dflt)
            {
                uvs_buf.add<uint8_t>((range.start_val & 0xff0000) >> 16);
                uvs_buf.add<uint8_t>((range.start_val & 0x00ff00) >> 8);
                uvs_buf.add<uint8_t>((range.start_val & 0x0000ff));
                uvs_buf.add<uint8_t>(range.count - 1);
            }
        }

        if (special_size)
        {
            offset_map[selector.first].special = head_length + uvs_buf.size();
            uvs_buf.add<uint32_t>(special_size);
            for (auto const& mapping : uvs.special)
            {
                uvs_buf.add<uint8_t>((mapping.unicode & 0xff0000) >> 16);
                uvs_buf.add<uint8_t>((mapping.unicode & 0x00ff00) >> 8);
                uvs_buf.add<uint8_t>((mapping.unicode & 0x0000ff));
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
        buf.add<uint8_t>((selector.first & 0xff0000) >> 16);
        buf.add<uint8_t>((selector.first & 0x00ff00) >> 8);
        buf.add<uint8_t>((selector.first & 0x0000ff));

        auto offsets = offset_map[selector.first];

        // defaultUVSOffset
        buf.add<uint32_t>(offsets.dflt);

        // nonDefaultUVSOffset
        buf.add<uint32_t>(offsets.special);
    }

    buf.append(uvs_buf);

    return buf;
}

}
