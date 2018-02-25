#include "otftable.hpp"

#include <cmath>

namespace geul
{

OTFTable::OTFTable(std::string id)
    : id_(std::move(id))
{}

std::string OTFTable::id() const
{
    return id_;
}

uint32_t calculate_checksum(Buffer& dis, size_t length)
{
    uint32_t checksum = 0;
    uint32_t l;

    l = (length % 4 > 0) ? length / 4 + 1 : length / 4;

    for (uint32_t i = 0; i < l; i++)
    {
        checksum += dis.read<uint32_t>();
    }

    return checksum;
}

int le_pow2(int num)
{
    return 1 << std::ilogb(num);
}
}
