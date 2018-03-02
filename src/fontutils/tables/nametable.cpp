#include "nametable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

NameTable::NameTable()
    : OTFTable(tag)
{}

void NameTable::parse(InputBuffer& dis)
{
    std::cout << "Parsing 'name'... " << std::endl;

    size_t beginning = dis.tell();

    format = dis.read<uint16_t>();
    if (format == 0)
    {
        auto   count = dis.read<uint16_t>();
        size_t offset = dis.read<uint16_t>();
        for (auto i = 0u; i < count; ++i)
        {
            NameRecord record;
            record.platform_id = dis.read<uint16_t>();
            record.encoding_id = dis.read<uint16_t>();
            record.language_id = dis.read<uint16_t>();
            record.name_id = dis.read<uint16_t>();

            size_t len = dis.read<uint16_t>();
            size_t stroff = dis.read<uint16_t>();

            record.str.resize(len);

            auto orig_pos = dis.seek(beginning + offset + stroff);
            dis.read<char>(&record.str[0], len);
            dis.seek(orig_pos);

            records.push_back(record);
        }
    }
    else
    {
        throw std::runtime_error("Unrecognized name table format");
    }
}

void NameTable::compile(OutputBuffer& out) const
{
    if (format == 0)
    {
        out.write<uint16_t>(format);
        out.write<uint16_t>(records.size());

        size_t offset = 6 + 12 * records.size();
        out.write<uint16_t>(offset);

        size_t off = 0;
        for (auto const& record : records)
        {
            out.write<uint16_t>(record.platform_id);
            out.write<uint16_t>(record.encoding_id);
            out.write<uint16_t>(record.language_id);
            out.write<uint16_t>(record.name_id);

            out.write<uint16_t>(record.str.size());
            out.write<uint16_t>(off);

            off += record.str.size();
        }

        for (auto const& record : records)
        {
            out.write_string(record.str);
        }
    }
    else
    {
        throw std::runtime_error("Unrecognized name table format");
    }
}

bool NameTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<NameTable const&>(rhs);
    return format == other.format && records == other.records;
}

bool NameTable::NameRecord::operator==(NameRecord const& rhs) const noexcept
{
    return platform_id == rhs.platform_id && encoding_id == rhs.encoding_id
           && language_id == rhs.language_id && name_id == rhs.name_id
           && str == rhs.str;
}
}
