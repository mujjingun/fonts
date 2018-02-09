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
    std::cout << "Unsupported table '" << id << "'... " << std::endl;

    dis.read<char>(&data[0], data.size());
}

Buffer GenericTable::compile() const
{
    return Buffer(data);
}

}
