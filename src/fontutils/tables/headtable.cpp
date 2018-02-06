#include "headtable.hpp"

#include <chrono>

namespace fontutils
{

HeadTable::HeadTable()
{
    id = "head";

    // FIXME
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    created = time;
    modified = time;

    // TODO: set glyph bounding box
}

void HeadTable::parse(Buffer &dis)
{
    auto major_version = dis.read<uint16_t>();
    if (major_version != 1)
        throw std::runtime_error("Unrecognized head table major version");
    auto minor_version = dis.read<uint16_t>();
    if (minor_version != 0)
        throw std::runtime_error("Unrecognized head table minor version");
    dis.read<Fixed>();
    adjusted_checksum = dis.read<uint32_t>();
    auto magic_number = dis.read<uint32_t>();
    if (magic_number != 0x5F0F3CF5)
        throw std::runtime_error("Invalid magic number");
    flags = dis.read<uint16_t>();
    units_per_em = dis.read<uint16_t>();
    created = dis.read<int64_t>();
    modified = dis.read<int64_t>();
    xmin = dis.read<int16_t>();
    ymin = dis.read<int16_t>();
    xmax = dis.read<int16_t>();
    ymax = dis.read<int16_t>();
    mac_style = dis.read<int16_t>();
    lowest_PPEM = dis.read<int16_t>();
    font_direction_hint = dis.read<uint16_t>();
    auto index_to_loc_format = dis.read<uint16_t>();
    if (index_to_loc_format != 0)
        throw std::runtime_error("Unrecognized index to loc format");
    glyph_data_format = dis.read<uint16_t>();
}

Buffer HeadTable::compile() const
{
    Buffer buf;

    // majorVersion
    buf.add<uint16_t>(1);
    // minorVersion
    buf.add<uint16_t>(0);
    // fontRevision
    buf.add<Fixed>(Fixed(1 << 16));

    // Zero on the first run, set on the second
    buf.add<uint32_t>(adjusted_checksum);

    // magicNumber
    buf.add<uint32_t>(0x5F0F3CF5);

    // flags
    buf.add<uint16_t>(BASELINE_AT_ZERO | LSB_AT_ZERO);

    // unitsPerEm
    buf.add<uint16_t>(units_per_em);

    // created
    buf.add<int64_t>(created);
    // modified
    buf.add<int64_t>(modified);

    // bounding box for all glyphs
    buf.add<int16_t>(xmin);
    buf.add<int16_t>(ymin);
    buf.add<int16_t>(xmax);
    buf.add<int16_t>(ymax);

    // macStyle
    buf.add<int16_t>(0);

    // lowestRecPPEM
    buf.add<int16_t>(3);

    // fontDirectionHint (deprecated, set to 2)
    buf.add<uint16_t>(2);

    // indexToLocFormat (set to short, Offset16)
    buf.add<uint16_t>(0);

    // glyphDataFormat (current)
    buf.add<uint16_t>(0);

    // Pad to 4-byte boundary
    buf.pad();

    return buf;
}

}
