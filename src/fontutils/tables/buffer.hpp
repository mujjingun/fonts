#ifndef TABLES_BUFFER_HPP
#define TABLES_BUFFER_HPP

#include <string>
#include <cstring>
#include <stdexcept>

namespace fontutils
{

enum class Fixed : uint32_t
{};

using Tag = std::array<uint8_t, 4>;

template<typename T>
inline T to_machine_endian(char const *buf)
{ static_assert((T{}, 0), "Cannot convert type to machine endian"); }

template<>
inline char to_machine_endian(char const *buf)
{ return *buf; }

template<>
inline uint8_t to_machine_endian(char const *buf)
{ return *buf; }

template<>
inline int8_t to_machine_endian(char const *buf)
{ return *buf; }

template<>
inline uint16_t to_machine_endian(char const *buf)
{ return (uint16_t(buf[1]&0xff)<<0) | (uint16_t(buf[0]&0xff)<<8); }

template<>
inline int16_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint16_t>(buf); }

template<>
inline uint32_t to_machine_endian(char const *buf)
{ return (uint32_t(buf[3]&0xff)<<0) | (uint32_t(buf[2]&0xff)<<8)
            | (uint32_t(buf[1]&0xff)<<16) | (uint32_t(buf[0]&0xff)<<24); }

template<>
inline int32_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint32_t>(buf); }

template<>
inline uint64_t to_machine_endian(char const *buf)
{ return (uint64_t(buf[7]&0xff)<<0) | (uint64_t(buf[6]&0xff)<<8)
            | (uint64_t(buf[5]&0xff)<<16) | (uint64_t(buf[4]&0xff)<<24)
            | (uint64_t(buf[3]&0xff)<<32) | (uint64_t(buf[2]&0xff)<<40)
            | (uint64_t(buf[1]&0xff)<<48) | (uint64_t(buf[0]&0xff)<<56); }

template<>
inline int64_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint64_t>(buf); }

template<>
inline Tag to_machine_endian(char const *buf)
{ return {uint8_t(buf[0]), uint8_t(buf[1]), uint8_t(buf[2]), uint8_t(buf[3])}; }

template<>
inline Fixed to_machine_endian(char const* buf)
{ return Fixed(to_machine_endian<uint32_t>(buf)); }

template<typename T>
void to_big_endian(char *buf, T t)
{ static_assert((T{}, 0), "Cannot convert type to big endian"); }

template<>
inline void to_big_endian(char *buf, char t)
{ *buf = t; }

template<>
inline void to_big_endian(char *buf, int8_t t)
{ *buf = t; }

template<>
inline void to_big_endian(char *buf, uint8_t t)
{ *buf = t; }

template<>
inline void to_big_endian(char *buf, uint16_t t)
{
    buf[0] = (t & 0xff00) >> 8;
    buf[1] = (t & 0x00ff);
}

template<>
inline void to_big_endian(char *buf, int16_t t)
{
    buf[0] = (t & 0xff00) >> 8;
    buf[1] = (t & 0x00ff);
}

template<>
inline void to_big_endian(char *buf, uint32_t t)
{
    buf[0] = (t & 0xff000000) >> 24;
    buf[1] = (t & 0x00ff0000) >> 16;
    buf[2] = (t & 0x0000ff00) >> 8;
    buf[3] = (t & 0x000000ff);
}

template<>
inline void to_big_endian(char *buf, int32_t t)
{
    buf[0] = (t & 0xff000000) >> 24;
    buf[1] = (t & 0x00ff0000) >> 16;
    buf[2] = (t & 0x0000ff00) >> 8;
    buf[3] = (t & 0x000000ff);
}

template<>
inline void to_big_endian(char *buf, uint64_t t)
{
    buf[0] = (t & 0xff00000000000000) >> 56;
    buf[1] = (t & 0x00ff000000000000) >> 48;
    buf[2] = (t & 0x0000ff0000000000) >> 40;
    buf[3] = (t & 0x000000ff00000000) >> 32;
    buf[4] = (t & 0x00000000ff000000) >> 24;
    buf[5] = (t & 0x0000000000ff0000) >> 16;
    buf[6] = (t & 0x000000000000ff00) >> 8;
    buf[7] = (t & 0x00000000000000ff);
}

template<>
inline void to_big_endian(char *buf, int64_t t)
{
    buf[0] = (t & 0xff00000000000000) >> 56;
    buf[1] = (t & 0x00ff000000000000) >> 48;
    buf[2] = (t & 0x0000ff0000000000) >> 40;
    buf[3] = (t & 0x000000ff00000000) >> 32;
    buf[4] = (t & 0x00000000ff000000) >> 24;
    buf[5] = (t & 0x0000000000ff0000) >> 16;
    buf[6] = (t & 0x000000000000ff00) >> 8;
    buf[7] = (t & 0x00000000000000ff);
}

template<>
inline void to_big_endian(char *buf, Fixed t)
{
    to_big_endian<uint32_t>(buf, uint32_t(t));
}

template<>
inline void to_big_endian(char *buf, Tag t)
{
    buf[0] = t[0];
    buf[1] = t[1];
    buf[2] = t[2];
    buf[3] = t[3];
}

class Buffer
{
    std::string arr = "";
    size_t pos = 0;

public:
    Buffer() = default;
    explicit Buffer(std::string const& data)
        : arr(data)
    {}
    explicit Buffer(std::string && data)
        : arr(std::move(data))
    {}
    Buffer(Buffer const& buffer) = delete;
    Buffer(Buffer && buffer) = default;

    /// Write bytes starting from the current postion
    /// of the buffer, UB if it goes over the bounds
    template<typename T>
    void write(T *ptr, size_t count)
    {
        size_t n_bytes = sizeof(T) * count;

        if (pos + n_bytes > size())
            throw std::range_error("Attempt to write beyond buffer size");

        for (auto i = 0u; i < count; ++i)
        {
            to_big_endian<T>(&arr[pos + i * sizeof(T)], ptr[i]);
        }

        pos += n_bytes;
    }

    /// Add bytes at the end of the buffer,
    /// regardless of the current position
    template<typename T>
    void add(T const *ptr, size_t count)
    {
        size_t n_bytes = sizeof(T) * count;
        size_t orig_size = arr.size();
        arr.resize(orig_size + n_bytes);

        for (auto i = 0u; i < count; ++i)
        {
            to_big_endian<T>(&arr[orig_size + i * sizeof(T)], ptr[i]);
        }
    }

    /// Convenience function for adding 1 item
    template<typename T>
    void add(T t)
    {
        add(&t, 1);
    }

    /// Append another buffer to the end of this one
    void append(Buffer const &buf)
    {
        arr.append(buf.arr);
    }

    /// Pad to 4-byte boundary
    void pad()
    {
        size_t n = arr.size() % 4;
        if (n > 0) {
            arr.resize(arr.size() + (4 - n));
        }
    }

    /// Read items from the buffer staring from the
    /// current position
    template<typename T>
    void read(T *dest, size_t count)
    {
        size_t n_bytes = sizeof(T) * count;

        if (pos + n_bytes > size())
            throw std::range_error("Attempt to read beyond buffer size");

        for (auto i = 0u; i < count; ++i)
        {
            dest[i] = to_machine_endian<T>(&arr[pos + i * sizeof(T)]);
        }

        pos += n_bytes;
    }

    /// Convenience function for reading 1 item
    template<typename T>
    T read()
    {
        T t;
        read(&t, 1);
        return t;
    }

    /// Set current offset
    size_t seek(size_t off)
    {
        if (off > size())
            throw std::range_error("Invalid seek position.");
        size_t orig_pos = pos;
        pos = off;
        return orig_pos;
    }

    /// Get current offset
    size_t tell() const
    {
        return pos;
    }

    size_t size() const
    {
        return arr.size();
    }

    char* data()
    {
        return &arr[0];
    }
};

}

#endif // TABLES_BUFFER_HPP
