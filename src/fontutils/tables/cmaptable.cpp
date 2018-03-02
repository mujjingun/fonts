#include "cmaptable.hpp"

#include "cmapformat12.hpp"
#include "cmapformat14.hpp"
#include "cmapformat4.hpp"

#include <cassert>
#include <sstream>

namespace geul
{

CmapTable::CmapTable()
    : OTFTable(tag)
{}

namespace
{
    /// Factory function to make cmap subtables
    std::unique_ptr<CmapSubtable> make_subtable(
        InputBuffer&   dis,
        std::streampos pos,
        uint16_t       platform_id,
        uint16_t       encoding_id)
    {
        auto orig_pos = dis.seek(pos);
        auto format = dis.peek<uint16_t>();

        std::unique_ptr<CmapSubtable> table;
        if (format == 4)
        {
            table = std::make_unique<CmapFormat4Subtable>(
                platform_id, encoding_id);
        }
        else if (format == 12)
        {
            table = std::make_unique<CmapFormat12Subtable>(
                platform_id, encoding_id);
        }
        else if (format == 14)
        {
            table = std::make_unique<CmapFormat14Subtable>(
                platform_id, encoding_id);
        }
        else
        {
            // return to original position
            dis.seek(orig_pos);

            std::cerr << "Unsupported cmap subtable format " << format << ".";

            return nullptr;
        }

        // parse subtable
        table->parse(dis);

        // return to original position
        dis.seek(orig_pos);

        return table;
    }
}

void CmapTable::parse(InputBuffer& dis)
{
    std::cout << "Parsing 'cmap'... " << std::endl;

    auto beginning = dis.tell();

    auto version = dis.read<uint16_t>();
    if (version != 0)
        throw std::runtime_error("Unrecognized CMap version");

    auto num_tables = dis.read<uint16_t>();

    for (auto i = 0u; i < num_tables; ++i)
    {
        auto           platform_id = dis.read<uint16_t>();
        auto           encoding_id = dis.read<uint16_t>();
        std::streamoff off = dis.read<uint32_t>();

        auto sub
            = make_subtable(dis, beginning + off, platform_id, encoding_id);
        if (sub)
        {
            subtables.push_back(std::move(sub));
        }
    }
}

void CmapTable::compile(OutputBuffer& out) const
{
    auto beginning = out.tell();

    // version
    out.write<uint16_t>(0);

    // numTables
    out.write<uint16_t>(subtables.size());

    // Compile subtables and store the offsets
    std::vector<std::streampos> offsets;

    // encodingRecords[numTables]
    for (auto const& sub : subtables)
    {
        out.write<uint16_t>(sub->platform_id);
        out.write<uint16_t>(sub->encoding_id);

        // placeholder for offsets
        offsets.push_back(out.tell());
        out.write<uint32_t>(0);
    }

    int idx = 0;
    for (auto const& sub : subtables)
    {
        // write offset
        auto offset = out.tell() - beginning;
        auto orig_pos = out.seek(offsets[idx]);
        out.write<uint32_t>(offset);
        out.seek(orig_pos);

        // write table
        sub->compile(out);

        idx++;
    }
}

bool CmapTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<CmapTable const&>(rhs);

    if (subtables.size() != other.subtables.size())
        return false;

    for (auto i = 0u; i < subtables.size(); ++i)
    {
        if (!(*subtables[i] == *other.subtables[i]))
            return false;
    }

    return true;
}
}
