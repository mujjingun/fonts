#include "buffer.hpp"

#include <fstream>
#include <sstream>

namespace geul
{

// InputBuffer

InputBuffer InputBuffer::open(std::string filename)
{
    InputBuffer input_buf;
    input_buf.mode = std::ios::in;

    auto buf = std::make_unique<std::filebuf>();
    if (!buf->open(filename, input_buf.mode | std::ios::binary))
        throw std::runtime_error("Cannot open file");
    input_buf.buf = std::move(buf);

    return input_buf;
}

InputBuffer::InputBuffer(std::string&& data)
    : mode(std::ios::in)
    , buf(std::make_unique<std::stringbuf>(data, mode | std::ios::binary))
{
    data = std::string();
}

std::string InputBuffer::read_string(std::streamsize length)
{
    std::string str(length, 0);
    auto        n = buf->sgetn(&str[0], length);
    if (n < length)
        throw std::runtime_error("cannot read string");
    return str;
}

uint32_t InputBuffer::read_nint(int n)
{
    if (n <= 0 || n > 4)
        throw std::runtime_error("cannot read n-byte integer");

    uint32_t ret = 0;
    for (int i = 0; i < n; ++i)
    {
        ret <<= 8;
        ret |= read<uint8_t>() & 0xff;
    }
    return ret;
}

std::streampos InputBuffer::seek(std::streampos pos)
{
    auto orig_pos = tell();
    if (buf->pubseekpos(pos, mode) == std::streamoff(-1))
        throw std::runtime_error("Cannot seek to pos");
    return orig_pos;
}

std::streampos InputBuffer::seek_begin()
{
    auto orig_pos = tell();
    if (buf->pubseekoff(0, std::ios::beg, mode) == std::streamoff(-1))
        throw std::runtime_error("Cannot seek to the beginning of buffer.");
    return orig_pos;
}

std::streampos InputBuffer::seek_end()
{
    auto orig_pos = tell();
    if (buf->pubseekoff(0, std::ios::end, mode) == std::streamoff(-1))
        throw std::runtime_error("Cannot seek to the end of buffer.");
    return orig_pos;
}

std::streampos InputBuffer::tell() const
{
    auto pos = buf->pubseekoff(0, std::ios::cur, mode);
    if (pos == std::streamoff(-1))
        throw std::runtime_error("Cannot tell current position.");
    return pos;
}

size_t InputBuffer::size() const
{
    auto orig_pos = tell();
    auto begin = buf->pubseekoff(0, std::ios::beg, mode);
    auto end = buf->pubseekoff(0, std::ios::end, mode);
    buf->pubseekpos(orig_pos, mode);
    return end - begin;
}

// OutputBuffer

OutputBuffer OutputBuffer::open(std::string filename)
{
    OutputBuffer output_buf;
    output_buf.mode = std::ios::in | std::ios::out;

    auto buf = std::make_unique<std::filebuf>();
    if (!buf->open(filename, output_buf.mode | std::ios::binary | std::ios::trunc))
        throw std::runtime_error("Cannot open file");
    output_buf.buf = std::move(buf);

    return output_buf;
}

OutputBuffer::OutputBuffer(std::string&& data)
{
    mode = std::ios::in | std::ios::out;
    buf = std::make_unique<std::stringbuf>(data, mode | std::ios::binary);
}

void OutputBuffer::write_string(const std::string &str)
{
    write<char>(str.data(), str.size());
}

void OutputBuffer::write_nint(int n, uint32_t t)
{
    if (n <= 0 || n > 4)
        throw std::runtime_error("cannot read n-byte integer");

    int      shift = (n - 1) * 8;
    uint32_t mask = 0xff << shift;
    for (int i = 0; i < n; ++i)
    {
        write<uint8_t>((t & mask) >> shift);
        shift -= 8;
        mask >>= 8;
    }
}

void OutputBuffer::write_buf(InputBuffer&& other)
{
    constexpr auto  SIZE = 4096;
    char            arr[SIZE];
    std::streamsize n;
    do
    {
        n = other.buf->sgetn(arr, SIZE);
        auto written = buf->sputn(arr, n);
        if (written < n)
            throw std::runtime_error("Cannot write to buffer");
    } while (n == SIZE);
}

void OutputBuffer::pad()
{
    auto orig_pos = seek_begin();
    auto begin = seek_end();
    auto end = tell();
    auto remainder = (end - begin) % 4;
    if (remainder > 0)
        write_nint(4 - remainder, 0);
    seek(orig_pos);
}

}
