#ifndef TABLES_OTFTABLE_HPP
#define TABLES_OTFTABLE_HPP

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#include "../buffer.hpp"

namespace fontutils
{

class OTFTable
{
public:
    const std::string id;

public:
    // Pure abstract class, not instantiable

    OTFTable(std::string id);

    virtual ~OTFTable() = default;

    /// Compile the table into a Buffer.
    /// DOES NOT pad the end of the buffer.
    virtual Buffer compile() const = 0;

    /// Parse the buffer starting from the current position.
    virtual void parse(Buffer& dis) = 0;

    /// Compare equality
    virtual bool operator==(OTFTable const& rhs) const noexcept = 0;
};

uint32_t calculate_checksum(Buffer& dis, size_t length);

/// Biggest power-of-2 less than or equal to num
int le_pow2(int num);
}

#endif // TABLES_OTFTABLE_HPP
