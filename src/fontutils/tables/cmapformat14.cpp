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

        UnicodeMap<int32_t> map;

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
                auto count = dis.read<uint8_t>() + 1;
                map.set_range(start_val, start_val + count, -1);
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
                map.set(unicode_value, gid);
            }
            dis.seek(orig_pos);
        }

        uvsmap.emplace(var_selector, map);
    }
}

Buffer CmapFormat14Subtable::compile() const
{
    Buffer buf;

    // Format 14
    buf.add<uint16_t>(14);

    size_t length = 10 + uvsmap.size() * 11 + ;

    // length
    buf.add<uint16_t>(length);

    return buf;
}

}
