#include "offsettable.hpp"

#include "cmaptable.hpp"
#include "headtable.hpp"
#include "hheatable.hpp"
#include "hmtxtable.hpp"
#include "maxptable.hpp"
#include "nametable.hpp"
#include "os2table.hpp"
#include "posttable.hpp"
#include "generictable.hpp"

#include <map>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace fontutils
{

OffsetTable::OffsetTable()
{}

// Factory method for making tables
static std::unique_ptr<OTFTable> make_table(
        std::string name,
        Buffer &dis,
        size_t offset,
        size_t length)
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
    else
        table = std::make_unique<GenericTable>(name, length);

    auto orig_pos = dis.tell();
    dis.seek(offset);
    table->parse(dis);
    dis.seek(orig_pos);

    return table;
}

void OffsetTable::parse(Buffer &dis)
{
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

    struct TableInfo{
        size_t offset;
        size_t length;
    };
    std::map<std::string, TableInfo> tables_pos;
    for (auto i = 0u; i < num_tables; ++i)
    {
        Tag tag = dis.read<Tag>();
        std::string table_name = std::string(tag.begin(), tag.end());

        uint32_t checksum = dis.read<uint32_t>();

        size_t offset = dis.read<uint32_t>();
        size_t length = dis.read<uint32_t>();
        tables_pos[table_name] = {offset, length};

        auto orig_pos = dis.tell();
        dis.seek(offset);

        uint32_t calc_checksum = calculate_checksum(dis, length);

        // Exclude checkSumAdjustment value for the 'head' table
        if (table_name == "head")
        {
            dis.seek(offset + 8);
            auto checksum_adjustment = dis.read<uint32_t>();
            calc_checksum -= checksum_adjustment;
        }

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

    const char *required_tables[] = {
        "cmap", "head", "hhea", "hmtx", "maxp", "name", "OS/2", "post"
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

    auto maxp = new MaxpTable();
    dis.seek(tables_pos["maxp"].offset);
    tables_pos.erase(tables_pos.find("maxp"));
    maxp->parse(dis);
    tables["maxp"] = std::unique_ptr<OTFTable>(maxp);

    auto hhea = new HheaTable();
    dis.seek(tables_pos["hhea"].offset);
    tables_pos.erase(tables_pos.find("hhea"));
    hhea->parse(dis);
    tables["hhea"] = std::unique_ptr<OTFTable>(hhea);

    auto hmtx = new HmtxTable(maxp->num_glyphs, hhea->num_h_metrics);
    dis.seek(tables_pos["hmtx"].offset);
    tables_pos.erase(tables_pos.find("hmtx"));
    hmtx->parse(dis);
    tables["hmtx"] = std::unique_ptr<OTFTable>(hmtx);

    // Parse the remaining tables
    for (auto const &table : tables_pos)
    {
        std::string const& tag = table.first;
        TableInfo const& info = table.second;
        tables[tag] = make_table(tag, dis, info.offset, info.length);
    }
}

Buffer OffsetTable::compile() const
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

    Buffer tables_data;
    size_t offset_table_size = 12 + 16 * tables.size();
    size_t checksum_adj_pos;
    // Table Records
    for (auto const& pp : tables)
    {
        std::string table_name = pp.first;
        auto const& table = pp.second;
        // table name
        buf.add<char>(table_name.data(), 4);

        Buffer table_buf = table->compile();

        // checksum
        buf.add<uint32_t>(calculate_checksum(table_buf, table_buf.size()));

        // offset
        auto off = offset_table_size + tables_data.size();
        if (table_name == "head")
        {
            checksum_adj_pos = off + 8;
        }
        buf.add<uint32_t>(off);
        tables_data.append(table_buf);

        // length
        buf.add<uint32_t>(table_buf.size());
    }

    buf.append(tables_data);
    buf.pad();

    // set checksumAdjustment value
    buf.seek(0);
    auto entire_checksum = calculate_checksum(buf, buf.size());
    buf.seek(checksum_adj_pos);
    auto checksum_adj = uint32_t(0xB1B0AFBA) - entire_checksum;
    buf.write<uint32_t>(&checksum_adj, 1);

    return buf;
}

}
