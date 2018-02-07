#include "headtable.hpp"

#include <chrono>
#include <ctime>

namespace fontutils
{

static time_t beginning_of_time()
{
    std::tm pt;
    pt.tm_year = 1904;
    pt.tm_mon = 0;
    pt.tm_mday = 1;
    pt.tm_hour = 0;
    pt.tm_min = 0;
    pt.tm_sec = 0;
    std::time_t ret = std::mktime(&pt);

    std::tm pgt = *std::gmtime(&ret);
    std::tm plt = *std::localtime(&ret);

    plt.tm_year -= pgt.tm_year - plt.tm_year;
    plt.tm_mon -= pgt.tm_mon - plt.tm_mon;
    plt.tm_mday -= pgt.tm_mday - plt.tm_mday;
    plt.tm_hour -= pgt.tm_hour - plt.tm_hour;
    plt.tm_min -= pgt.tm_min - plt.tm_min;
    plt.tm_sec -= pgt.tm_sec - plt.tm_sec;

    return std::mktime(&plt);
}

HeadTable::HeadTable()
{
    id = "head";

    using namespace std::chrono;

    auto now = system_clock::now();
    auto start = system_clock::from_time_t(beginning_of_time());
    auto time = duration_cast<seconds>(now - start).count();
    created = time;
    modified = time;

    // TODO: set glyph bounding box
}

void HeadTable::parse(Buffer &dis)
{
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

    // Set to zero, directly written to the memory afterwards
    buf.add<uint32_t>(0);

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

    return buf;
}

}
