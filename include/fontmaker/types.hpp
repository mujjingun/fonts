
#include <cstdint>
#include <array>

#ifndef FONTMAKER_TYPES_HPP
#define FONTMAKER_TYPES_HPP

namespace fontmaker
{
    using uint8 = std::uint8_t;
    using int8 = std::int8_t;
    using uint16 = std::uint16_t;
    using int16 = std::int16_t;
    enum class uint24 : std::uint32_t
    {};
    using uint32 = std::uint32_t;
    using int32 = std::int32_t;
    enum class fixed : std::uint32_t
    {};
    using fword = int16;
    using ufword = uint16;
    enum class f2dot14_t : std::uint16_t
    {};
    enum class longdatetime_t : std::int64_t;
    using tag_t = std::array<uint8, 4>;
    using offset16_t = uint16_t;
    using offset32_t = uint32_t;
}

#endif
