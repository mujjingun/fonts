#include "font.hpp"

#include "cfftable.hpp"
#include "cmaptable.hpp"
#include "generictable.hpp"
#include "headtable.hpp"
#include "hheatable.hpp"
#include "hmtxtable.hpp"
#include "maxptable.hpp"
#include "nametable.hpp"
#include "os2table.hpp"
#include "posttable.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <sstream>
#include <typeinfo>

namespace geul
{

Font::Font()
    : OTFTable("sfnt")
{}

// Factory method for making tables
static std::unique_ptr<OTFTable>
make_table(std::string name, Buffer& dis, size_t offset, size_t length)
{
    std::unique_ptr<OTFTable> table;

    if (name == "cmap")
        table = std::make_unique<CmapTable>();
    else if (name == "head")
        table = std::make_unique<HeadTable>();
    else if (name == "name")
        table = std::make_unique<NameTable>();
    else if (name == "OS/2")
        table = std::make_unique<OS2Table>();
    else if (name == "post")
        table = std::make_unique<PostTable>();
    else if (name == "CFF ")
        table = std::make_unique<CFFTable>();
    else if (name == "maxp")
        table = std::make_unique<MaxpTable>();
    else if (name == "hhea")
        table = std::make_unique<HheaTable>();
    else
        table = std::make_unique<GenericTable>(name, length);

    auto orig_pos = dis.seek(offset);
    table->parse(dis);
    dis.seek(orig_pos);

    return table;
}

void Font::parse(Buffer& dis)
{
    auto beginning = dis.tell();

    auto sfnt_version = dis.read<uint32_t>();
    if (sfnt_version != 0x4F54544F)
        throw std::runtime_error("Not a CFF font");
    auto num_tables = dis.read<uint16_t>();

    // searchRange
    dis.read<uint16_t>();
    // entrySelector
    dis.read<uint16_t>();
    // rangeShift
    dis.read<uint16_t>();

    struct TableInfo
    {
        size_t offset;
        size_t length;
    };
    std::map<std::string, TableInfo> tables_pos;
    uint32_t                         entire_checksum = 0;
    uint32_t                         checksum_adjustment = 0;
    for (auto i = 0u; i < num_tables; ++i)
    {
        Tag         tag = dis.read<Tag>();
        std::string table_name = std::string(tag.begin(), tag.end());

        uint32_t checksum = dis.read<uint32_t>();

        size_t offset = dis.read<uint32_t>();
        size_t length = dis.read<uint32_t>();
        tables_pos[table_name] = { offset, length };

        auto orig_pos = dis.tell();
        dis.seek(offset);

        uint32_t calc_checksum = calculate_checksum(dis, length);

        // Exclude checkSumAdjustment value for the 'head' table
        if (table_name == "head")
        {
            dis.seek(offset + 8);
            checksum_adjustment = dis.read<uint32_t>();
            calc_checksum -= checksum_adjustment;
        }

        entire_checksum += calc_checksum;

        // Verify checksum
        if (checksum != calc_checksum)
        {
            std::ostringstream oss;
            oss << "Invalid checksum " << std::hex << checksum;
            oss << "for table " << table_name << ".\n";
            oss << "Calculated : " << calc_checksum;
            throw std::runtime_error(oss.str());
        }
        dis.seek(orig_pos);
    }

    const char* required_tables[] = {
        "cmap", "head", "hhea", "hmtx",
        "maxp", "name", "OS/2", "post", // required for sfnt
        "CFF ", "VORG", // required for CFF outlines
    };

    for (auto r : required_tables)
    {
        if (tables_pos.count(r) == 0)
        {
            std::ostringstream oss;
            oss << "Font does not have the required table '" << r << "'.";
            throw std::runtime_error(oss.str());
        }
    }

    // Validate checksumAdjustment
    auto length = dis.seek(beginning) - beginning;
    entire_checksum += calculate_checksum(dis, length);
    if (entire_checksum + checksum_adjustment != 0xB1B0AFBA)
    {
        throw std::runtime_error("Invalid font checksum.");
    }

    // Parse the tables
    for (auto const& table : tables_pos)
    {
        std::string const& tag = table.first;
        TableInfo const&   info = table.second;
        tables[tag] = make_table(tag, dis, info.offset, info.length);
    }

    // Parse remaining tables
    dis.seek(tables_pos["hmtx"].offset);
    tables_pos.erase(tables_pos.find("hmtx"));

    auto hmtx = std::make_unique<HmtxTable>(
        dynamic_cast<MaxpTable&>(*tables["maxp"]).num_glyphs,
        dynamic_cast<HheaTable&>(*tables["hhea"]).num_h_metrics);
    hmtx->parse(dis);
    tables["hmtx"] = std::move(hmtx);
}

Buffer Font::compile() const
{
    Buffer buf;

    // snft version
    buf.add<uint32_t>(0x4F54544F);

    // numTables
    buf.add<uint16_t>(tables.size());

    // searchRange
    auto search_range = 16 * le_pow2(tables.size());
    buf.add<uint16_t>(search_range);
    // entrySelector
    buf.add<uint16_t>(std::ilogb(tables.size()));
    // rangeShift
    buf.add<uint16_t>(tables.size() * 16 - search_range);

    Buffer   tables_data;
    size_t   offset_table_size = 12 + 16 * tables.size();
    uint32_t entire_checksum = 0;
    int      checksum_adj_pos = -1;
    // Table Records
    for (auto const& pp : tables)
    {
        std::string table_name = pp.first;
        auto const& table = pp.second;
        // table name
        buf.add<char>(table_name.data(), 4);

        Buffer table_buf = table->compile();
        auto   length = table_buf.size();
        table_buf.pad();

        // checksum
        uint32_t checksum = calculate_checksum(table_buf, length);
        buf.add<uint32_t>(checksum);

        entire_checksum += checksum;

        // offset
        auto off = offset_table_size + tables_data.size();
        buf.add<uint32_t>(off);

        // Store position of checksumAdjustment
        if (table_name == "head")
            checksum_adj_pos = off + 8;

        // length
        buf.add<uint32_t>(length);

        tables_data.append(std::move(table_buf));
    }

    if (checksum_adj_pos == -1)
        throw std::runtime_error("'head' table not present");

    buf.append(std::move(tables_data));
    buf.pad();

    // set checksumAdjustment value
    buf.seek(0);
    entire_checksum += calculate_checksum(buf, offset_table_size);
    buf.seek(checksum_adj_pos);
    uint32_t checksum_adj = uint32_t(0xB1B0AFBA) - entire_checksum;
    buf.write<uint32_t>(&checksum_adj, 1);

    return buf;
}

bool Font::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<Font const&>(rhs);

    if (tables.size() != other.tables.size())
        return false;

    for (auto const& table : tables)
    {
        // no table with matching tag
        if (other.tables.count(table.first) == 0)
            return false;

        auto const& t0 = *table.second;
        auto const& t1 = *other.tables.at(table.first);
        if (!(t0 == t1))
            return false;
    }
    return true;
}

Glyph &Font::glyph(char32_t ch)
{
    auto &cmap = dynamic_cast<CmapTable&>(*tables["cmap"]);
    auto &cff = dynamic_cast<CFFTable&>(*tables["CFF "]);
    // TODO
    return cff.fonts[0].glyphs[0];
}
}