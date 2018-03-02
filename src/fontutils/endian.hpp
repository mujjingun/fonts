#ifndef FONTUTILS_ENDIAN_HPP
#define FONTUTILS_ENDIAN_HPP

#include <cstddef>
#include <cstdint>
#include <array>

namespace geul
{

enum class Fixed : uint32_t
{
};

using Tag = std::array<uint8_t, 4>;

template <typename T> inline T to_machine_endian(char const*)
{
    static_assert((T{}, 0), "Cannot convert type to machine endian");
}

template <> char     to_machine_endian(char const* buf);
template <> uint8_t  to_machine_endian(char const* buf);
template <> int8_t   to_machine_endian(char const* buf);
template <> uint16_t to_machine_endian(char const* buf);
template <> int16_t  to_machine_endian(char const* buf);
template <> uint32_t to_machine_endian(char const* buf);
template <> int32_t  to_machine_endian(char const* buf);
template <> uint64_t to_machine_endian(char const* buf);
template <> int64_t  to_machine_endian(char const* buf);
template <> Tag      to_machine_endian(char const* buf);
template <> Fixed    to_machine_endian(char const* buf);

template <typename T> void to_big_endian(char*, T)
{
    static_assert((T{}, 0), "Cannot convert type to big endian");
}

template <> void to_big_endian(char* buf, char t);
template <> void to_big_endian(char* buf, uint8_t t);
template <> void to_big_endian(char* buf, int8_t t);
template <> void to_big_endian(char* buf, uint16_t t);
template <> void to_big_endian(char* buf, int16_t t);
template <> void to_big_endian(char* buf, uint32_t t);
template <> void to_big_endian(char* buf, int32_t t);
template <> void to_big_endian(char* buf, uint64_t t);
template <> void to_big_endian(char* buf, int64_t t);
template <> void to_big_endian(char* buf, Tag t);
template <> void to_big_endian(char* buf, Fixed t);

}

#endif
