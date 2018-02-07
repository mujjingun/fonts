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
    // Pure abstract class, not instantiable

    virtual ~OTFTable() = default;

    /// Compile the table into a Buffer.
    /// DO NOT pad the end of the buffer.
    virtual Buffer compile() const = 0;

    /// Parse the buffer starting from the current position.
    virtual void parse (Buffer &dis) = 0;
};

uint32_t calculate_checksum (Buffer &dis, uint32_t length);

/// Biggest power-of-2 less than or equal to num
int le_pow2(int num);

}

#endif // TABLES_OTFTABLE_HPP
