#ifndef FONTUTILS_ENDIAN_HPP
#define FONTUTILS_ENDIAN_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace geul
{
template <typename T> inline T to_machine_endian(char const* arr)
{
    static_assert(
        std::is_integral<T>::value || std::is_enum<T>::value,
        "Type is not integral nor enum");

    using DataType = typename std::make_unsigned<T>::type;

    DataType ret = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i)
    {
        ret <<= 8;
        ret |= arr[i] & 0xff;
    }

    return static_cast<T>(ret);
}

template <typename T> void to_big_endian(char* arr, T val)
{
    static_assert(
        std::is_integral<T>::value || std::is_enum<T>::value,
        "Type is not integral nor enum");

    using DataType = typename std::make_unsigned<T>::type;

    DataType t = static_cast<DataType>(val);

    int      shift = (sizeof(T) - 1) * 8;
    DataType mask = DataType(0xff) << shift;
    for (std::size_t i = 0; i < sizeof(T); ++i)
    {
        arr[i] = (t & mask) >> shift;
        shift -= 8;
        mask >>= 8;
    }
}

}

#endif
