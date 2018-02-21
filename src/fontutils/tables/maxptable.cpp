#include "maxptable.hpp"

#include <typeinfo>
#include <cassert>

namespace fontutils
{

MaxpTable::MaxpTable()
    : OTFTable("maxp")
{
}

void MaxpTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'maxp'... " << std::endl;

    version = dis.read<Fixed>();
    if (version == Fixed(0x00005000))
    {
        num_glyphs = dis.read<uint16_t>();
    }
    else
    {
        throw std::runtime_error("CFF fonts must have version 0.5 maxp table");
    }
}

Buffer MaxpTable::compile() const
{
    Buffer buf;

    if (version == Fixed(0x00005000))
    {
        buf.add<Fixed>(version);
        buf.add<uint16_t>(num_glyphs);
    }
    else
    {
        throw std::runtime_error("CFF fonts must have version 0.5 maxp table");
    }

    return buf;
}

bool MaxpTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<MaxpTable const&>(rhs);
    return version == other.version && num_glyphs == other.num_glyphs;
}
}
