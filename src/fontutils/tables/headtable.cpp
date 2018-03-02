#include "headtable.hpp"

#include <cassert>
#include <chrono>
#include <ctime>
#include <typeinfo>

namespace geul
{

namespace
{
    time_t timestamp()
    {
        std::time_t now = std::time(nullptr);
        return now + 2082844800;
    }
}

HeadTable::HeadTable()
    : OTFTable(tag)
{
    using namespace std::chrono;

    int64_t time = timestamp();
    created = time;
    modified = time;

    // TODO: set glyph bounding box
}

void HeadTable::parse(InputBuffer& dis)
{
    std::cout << "Parsing 'head'... " << std::endl;

    version = dis.read<Fixed>();
    if (version != Fixed(0x00010000))
        throw std::runtime_error("Unrecognized head table version");

    // fontRevision
    font_revision = dis.read<Fixed>();

    // checksumAdjustment
    dis.read<uint32_t>();

    // magicNumber
    if (dis.read<uint32_t>() != 0x5F0F3CF5)
        throw std::runtime_error("Invalid magic number");

    flags = dis.read<uint16_t>();
    units_per_em = dis.read<uint16_t>();
    created = dis.read<int64_t>();
    // modified
    dis.read<int64_t>();
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

void HeadTable::compile(OutputBuffer& out) const
{
    // majorVersion
    out.write<uint16_t>(1);
    // minorVersion
    out.write<uint16_t>(0);
    // fontRevision
    out.write<Fixed>(font_revision);

    // Set to zero, directly written to the memory afterwards
    out.write<uint32_t>(0);

    // magicNumber
    out.write<uint32_t>(0x5F0F3CF5);

    // flags
    out.write<uint16_t>(BASELINE_AT_ZERO | LSB_AT_ZERO);

    // unitsPerEm
    out.write<uint16_t>(units_per_em);

    // created
    out.write<int64_t>(created);
    // modified
    out.write<int64_t>(modified);

    // bounding box for all glyphs
    out.write<int16_t>(xmin);
    out.write<int16_t>(ymin);
    out.write<int16_t>(xmax);
    out.write<int16_t>(ymax);

    // macStyle
    out.write<int16_t>(0);

    // lowestRecPPEM
    out.write<int16_t>(3);

    // fontDirectionHint (deprecated, set to 2)
    out.write<uint16_t>(2);

    // indexToLocFormat (set to short, Offset16)
    out.write<uint16_t>(0);

    // glyphDataFormat (current)
    out.write<uint16_t>(0);
}

bool HeadTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<HeadTable const&>(rhs);
    return xmin == other.xmin && ymin == other.ymin && xmax == other.xmax
           && ymax == other.ymax && units_per_em == other.units_per_em
           && mac_style == other.mac_style && lowest_PPEM == other.lowest_PPEM
           && font_direction_hint == other.font_direction_hint
           && glyph_data_format == other.glyph_data_format
           && version == other.version && font_revision == other.font_revision
           && flags == other.flags && created == other.created;
    /* && modified == other.modified */;
}
}
