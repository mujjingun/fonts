#ifndef TABLES_OTFTABLE_HPP
#define TABLES_OTFTABLE_HPP

#include <cstdint>
#include <string>
#include <cmath>
#include <iostream>
#include <array>

#include "buffer.hpp"

namespace fontutils
{

class OTFTable
{
public:
    std::string id = "NO_ID";

public:
    virtual ~OTFTable() = default;

    virtual Buffer compile() const = 0;
    virtual void parse (Buffer &dis) = 0;
};

uint32_t calculate_checksum (Buffer &dis, uint32_t length);
int le_pow2(int num);

}

#endif // TABLES_OTFTABLE_HPP
