#ifndef FONTUTILS_BUFFER_HPP
#define FONTUTILS_BUFFER_HPP

#include <string>
#include <cstring>
#include <stdexcept>
#include <array>
#include <unordered_map>

namespace fontutils
{

enum class Fixed : uint32_t
{};

using Tag = std::array<uint8_t, 4>;

template<typename T>
inline T to_machine_endian(char const *)
{ static_assert((T{}, 0), "Cannot convert type to machine endian"); }

template<> char     to_machine_endian(char const *buf);
template<> uint8_t  to_machine_endian(char const *buf);
template<> int8_t   to_machine_endian(char const *buf);
template<> uint16_t to_machine_endian(char const *buf);
template<> int16_t  to_machine_endian(char const *buf);
template<> uint32_t to_machine_endian(char const *buf);
template<> int32_t  to_machine_endian(char const *buf);
template<> uint64_t to_machine_endian(char const *buf);
template<> int64_t  to_machine_endian(char const *buf);
template<> Tag      to_machine_endian(char const *buf);
template<> Fixed    to_machine_endian(char const* buf);

template<typename T>
void to_big_endian(char *, T)
{ static_assert((T{}, 0), "Cannot convert type to big endian"); }

template<> void to_big_endian(char* buf, char     t);
template<> void to_big_endian(char* buf, uint8_t  t);
template<> void to_big_endian(char* buf, int8_t   t);
template<> void to_big_endian(char* buf, uint16_t t);
template<> void to_big_endian(char* buf, int16_t  t);
template<> void to_big_endian(char* buf, uint32_t t);
template<> void to_big_endian(char* buf, int32_t  t);
template<> void to_big_endian(char* buf, uint64_t t);
template<> void to_big_endian(char* buf, int64_t  t);
template<> void to_big_endian(char* buf, Tag      t);
template<> void to_big_endian(char* buf, Fixed    t);

class Buffer
{
    std::string arr = "";
    size_t pos = 0;

    std::unordered_map<std::string, size_t> markers = {};

public:
    Buffer() = default;
    explicit Buffer(std::string const& data);
    explicit Buffer(std::string && data);
    Buffer(Buffer const&) = delete;
    Buffer(Buffer &&) = default;
    Buffer &operator=(Buffer const&) = delete;
    Buffer &operator=(Buffer &&) = default;

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

    /// Convenience function for writing 1 item
    template<typename T>
    void write(T t)
    {
        write(&t, 1);
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

    /// Add n-byte integer (n = 1..4)
    void add_nbytes(int n, uint32_t t);

    /// Append another buffer to the end of this one
    void append(Buffer const &buf);

    /// Pad to 4-byte boundary
    void pad();

    /// Peek items from the buffer staring from the
    /// current position
    template<typename T>
    void peek(T *dest, size_t count) const
    {
        size_t n_bytes = sizeof(T) * count;

        if (pos + n_bytes > size())
            throw std::range_error("Attempt to read beyond buffer size");

        for (auto i = 0u; i < count; ++i)
        {
            dest[i] = to_machine_endian<T>(&arr[pos + i * sizeof(T)]);
        }
    }

    /// Convenience function for peeking 1 item
    template<typename T>
    T peek() const
    {
        T t;
        peek(&t, 1);
        return t;
    }

    /// Read items from the buffer staring from the
    /// current position, and increment position
    template<typename T>
    void read(T *dest, size_t count)
    {
        peek(dest, count);
        pos += sizeof(T) * count;
    }

    /// Convenience function for reading 1 item
    template<typename T>
    T read()
    {
        T t;
        read(&t, 1);
        return t;
    }

    std::string read_string(size_t length);

    /// Read n-byte integer (n = 1..4)
    uint32_t read_nbytes(int n);

    /// Set current offset
    size_t seek(size_t off);

    /// Get current offset
    size_t tell() const;

    size_t size() const;

    size_t marker(std::string const& name) const;

    void add_marker(size_t pos, std::string const& name);

    void clear_markers();

    char* data();
};

}

#endif // TABLES_BUFFER_HPP