#ifndef FONTUTILS_BUFFER_HPP
#define FONTUTILS_BUFFER_HPP

#include <ios>
#include <memory>
#include <streambuf>
#include <string>

#include "endian.hpp"

namespace geul
{
class OutputBuffer;

class InputBuffer
{
protected:
    std::ios::openmode              mode;
    std::unique_ptr<std::streambuf> buf;

    InputBuffer() = default;

    template <typename T> void read_impl(T* dest, size_t count) const
    {
        char bytes[sizeof(T)];
        for (auto i = 0u; i < count; ++i)
        {
            int n = buf->sgetn(bytes, sizeof(T));
            if (n < int(sizeof(T)))
                throw std::runtime_error("Attempt to read beyond buffer.");
            dest[i] = to_machine_endian<T>(bytes);
        }
    }

    friend OutputBuffer;

public:
    static InputBuffer open(std::string filename);
    explicit InputBuffer(std::string&& data);

    /// Peek items from the buffer staring from the
    /// current position
    template <typename T> void peek(T* dest, size_t count) const
    {
        auto orig_pos = buf->pubseekoff(0, std::ios::cur, mode);
        read_impl(dest, count);
        buf->pubseekpos(orig_pos, mode);
    }

    /// Convenience function for peeking 1 item
    template <typename T> T peek() const
    {
        T t;
        peek(&t, 1);
        return t;
    }

    /// Read items from the buffer staring from the
    /// current position, and increment position
    template <typename T> void read(T* dest, size_t count)
    {
        read_impl(dest, count);
    }

    /// Convenience function for reading 1 item
    template <typename T> T read()
    {
        T t;
        read(&t, 1);
        return t;
    }

    /// Read std::string of length `length`
    std::string read_string(std::streamsize length);

    /// Read n-byte integer (n = 1..4)
    uint32_t read_nint(int n);

    /// Set current offset from the beginning
    /// returns original position
    std::streampos seek(std::streampos pos);

    /// Seek to the beginning
    std::streampos seek_begin();

    /// Seek to the end
    std::streampos seek_end();

    /// Current position on the stream
    std::streampos tell() const;

    /// Size of the buffer for only string-backed buffers
    size_t size() const;
};

class OutputBuffer : public InputBuffer
{
    OutputBuffer() = default;

public:
    /// Make a file buffer
    static OutputBuffer open(std::string filename);

    /// Make a string buffer
    explicit OutputBuffer(std::string&& data);

    /// Write bytes starting from the current postion
    /// of the buffer
    template <typename T> void write(T const* ptr, size_t count)
    {
        char bytes[sizeof(T)];
        for (auto i = 0u; i < count; ++i)
        {
            to_big_endian<T>(bytes, ptr[i]);
            int n = buf->sputn(bytes, sizeof(T));
            if (n < int(sizeof(T)))
                throw std::runtime_error("Cannot write to buffer.");
        }
    }

    /// Convenience function for writing 1 item
    template <typename T> void write(T t)
    {
        write(&t, 1);
    }

    /// Convenience function for writing 1 item at certain position
    template <typename T> void write_at(std::streampos pos, T t)
    {
        auto orig_pos = seek(pos);
        write(t);
        seek(orig_pos);
    }

    /// Write string
    void write_string(std::string const& str);

    /// Write n-byte integer (n = 1..4)
    void write_nint(int n, uint32_t t);

    /// Write another buffer to the current pos
    void write_buf(InputBuffer&& other);

    /// Pad to 4-byte boundary
    void pad();
};
}

#endif // TABLES_BUFFER_HPP
