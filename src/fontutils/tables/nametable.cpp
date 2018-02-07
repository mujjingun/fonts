#include "nametable.hpp"

namespace fontutils
{

NameTable::NameTable()
{
    id = "name";
}

void NameTable::parse(Buffer &dis)
{
    size_t beginning = dis.tell();

    format = dis.read<uint16_t>();
    if (format == 0)
    {
        auto count = dis.read<uint16_t>();
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
        throw std::runtime_error("Unrecognized name table format");
}

Buffer NameTable::compile() const
{
    Buffer buf;

    if (format == 0)
    {
        buf.add<uint16_t>(format);
        buf.add<uint16_t>(records.size());

        size_t offset = 6 + 12 * records.size();
        buf.add<uint16_t>(offset);

        Buffer strings;
        size_t off = 0;
        for (auto i = 0u; i < records.size(); ++i)
        {
            buf.add<uint16_t>(records[i].platform_id);
            buf.add<uint16_t>(records[i].encoding_id);
            buf.add<uint16_t>(records[i].language_id);
            buf.add<uint16_t>(records[i].name_id);

            buf.add<uint16_t>(records[i].str.size());
            buf.add<uint16_t>(off);

            off += records[i].str.size();
            strings.append(Buffer(records[i].str));
        }

        buf.append(strings);
    }
    else
        throw std::runtime_error("Unrecognized name table format");

    return buf;
}

}