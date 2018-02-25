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

class unsupported_cmapsubtable_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/// Factory function to make cmap subtables
static std::unique_ptr<CmapSubtable> make_subtable(
    Buffer& dis, size_t off, uint16_t platform_id, uint16_t encoding_id)
{
    auto orig_pos = dis.seek(off);
    auto format = dis.read<uint16_t>();

    std::unique_ptr<CmapSubtable> table;
    if (format == 4)
    {
        table = std::make_unique<CmapFormat4Subtable>(platform_id, encoding_id);
    }
    else if (format == 12)
    {
        table
            = std::make_unique<CmapFormat12Subtable>(platform_id, encoding_id);
    }
    else if (format == 14)
    {
        table
            = std::make_unique<CmapFormat14Subtable>(platform_id, encoding_id);
    }
    else
    {
        // return to original position
        dis.seek(orig_pos);

        std::ostringstream oss;
        oss << "Unsupported cmap subtable format " << format << ".";
        throw unsupported_cmapsubtable_error(oss.str());
    }

    // parse subtable
    dis.seek(off);
    table->parse(dis);

    // return to original position
    dis.seek(orig_pos);

    return table;
}

void CmapTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'cmap'... " << std::endl;

    auto beginning = dis.tell();

    auto version = dis.read<uint16_t>();
    if (version != 0)
        throw std::runtime_error("Unrecognized CMap version");

    auto num_tables = dis.read<uint16_t>();

    for (auto i = 0u; i < num_tables; ++i)
    {
        auto platform_id = dis.read<uint16_t>();
        auto encoding_id = dis.read<uint16_t>();
        auto off = dis.read<uint32_t>();

        try
        {
            auto sub
                = make_subtable(dis, beginning + off, platform_id, encoding_id);
            subtables.push_back(std::move(sub));
        }
        catch (unsupported_cmapsubtable_error const& e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << "Skipped subtable." << std::endl;
        }
    }
}

Buffer CmapTable::compile() const
{
    Buffer buf;

    // version
    buf.add<uint16_t>(0);

    // numTables
    buf.add<uint16_t>(subtables.size());

    // Compile subtables and store the offsets
    Buffer              subtables_buf;
    std::vector<size_t> offsets;
    size_t              off = 4 + 8 * subtables.size();
    for (auto const& sub : subtables)
    {
        Buffer sub_buf = sub->compile();
        sub_buf.pad();

        offsets.push_back(off);
        off += sub_buf.size();

        subtables_buf.append(std::move(sub_buf));
    }

    // encodingRecords[numTables]
    for (auto i = 0u; i < subtables.size(); ++i)
    {
        buf.add<uint16_t>(subtables[i]->platform_id);
        buf.add<uint16_t>(subtables[i]->encoding_id);
        buf.add<uint32_t>(offsets[i]);
    }

    // append subtables
    buf.append(std::move(subtables_buf));

    return buf;
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
