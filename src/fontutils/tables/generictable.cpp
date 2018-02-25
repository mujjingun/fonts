#include "generictable.hpp"

#include <cassert>
#include <typeinfo>

namespace geul
{

GenericTable::GenericTable(std::string tag, size_t length)
    : OTFTable(std::move(tag))
{
    data.resize(length);
}

void GenericTable::parse(Buffer& dis)
{
    std::cout << "Unsupported table '" << id() << "'... " << std::endl;

    dis.read<char>(&data[0], data.size());
}

Buffer GenericTable::compile() const
{
    return Buffer(data);
}

bool GenericTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(id() == rhs.id() && typeid(*this) == typeid(rhs));
    auto const& other = static_cast<GenericTable const&>(rhs);
    return data == other.data;
}
}
