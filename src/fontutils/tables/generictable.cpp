#include "generictable.hpp"

namespace fontutils
{

GenericTable::GenericTable(std::string tag, size_t length)
{
    id = tag;
    data.resize(length);
}

void GenericTable::parse(Buffer &dis)
{
    dis.read<char>(&data[0], data.size());
}

Buffer GenericTable::compile() const
{
    Buffer buf(data);
    buf.pad();
    return buf;
}

}
