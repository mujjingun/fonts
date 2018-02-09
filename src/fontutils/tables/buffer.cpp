#include "buffer.hpp"

namespace fontutils
{

template<>
char to_machine_endian(char const *buf)
{ return *buf; }

template<>
uint8_t to_machine_endian(char const *buf)
{ return *buf; }

template<>
int8_t to_machine_endian(char const *buf)
{ return *buf; }

template<>
uint16_t to_machine_endian(char const *buf)
{ return (uint16_t(buf[1]&0xff)<<0) | (uint16_t(buf[0]&0xff)<<8); }

template<>
int16_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint16_t>(buf); }

template<>
uint32_t to_machine_endian(char const *buf)
{ return (uint32_t(buf[3]&0xff)<<0) | (uint32_t(buf[2]&0xff)<<8)
            | (uint32_t(buf[1]&0xff)<<16) | (uint32_t(buf[0]&0xff)<<24); }

template<>
int32_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint32_t>(buf); }

template<>
uint64_t to_machine_endian(char const *buf)
{ return (uint64_t(buf[7]&0xff)<<0) | (uint64_t(buf[6]&0xff)<<8)
            | (uint64_t(buf[5]&0xff)<<16) | (uint64_t(buf[4]&0xff)<<24)
            | (uint64_t(buf[3]&0xff)<<32) | (uint64_t(buf[2]&0xff)<<40)
            | (uint64_t(buf[1]&0xff)<<48) | (uint64_t(buf[0]&0xff)<<56); }

template<>
int64_t to_machine_endian(char const *buf)
{ return to_machine_endian<uint64_t>(buf); }

template<>
Tag to_machine_endian(char const *buf)
{ return {uint8_t(buf[0]), uint8_t(buf[1]), uint8_t(buf[2]), uint8_t(buf[3])}; }

template<>
Fixed to_machine_endian(char const* buf)
{ return Fixed(to_machine_endian<uint32_t>(buf)); }

template<>
void to_big_endian(char *buf, char t)
{ *buf = t; }

template<>
void to_big_endian(char *buf, int8_t t)
{ *buf = t; }

template<>
void to_big_endian(char *buf, uint8_t t)
{ *buf = t; }

template<>
void to_big_endian(char *buf, uint16_t t)
{
    buf[0] = (t & 0xff00) >> 8;
    buf[1] = (t & 0x00ff);
}

template<>
void to_big_endian(char *buf, int16_t t)
{
    buf[0] = (t & 0xff00) >> 8;
    buf[1] = (t & 0x00ff);
}

template<>
void to_big_endian(char *buf, uint32_t t)
{
    buf[0] = (t & 0xff000000) >> 24;
    buf[1] = (t & 0x00ff0000) >> 16;
    buf[2] = (t & 0x0000ff00) >> 8;
    buf[3] = (t & 0x000000ff);
}

template<>
void to_big_endian(char *buf, int32_t t)
{
    buf[0] = (t & 0xff000000) >> 24;
    buf[1] = (t & 0x00ff0000) >> 16;
    buf[2] = (t & 0x0000ff00) >> 8;
    buf[3] = (t & 0x000000ff);
}

template<>
void to_big_endian(char *buf, uint64_t t)
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
void to_big_endian(char *buf, int64_t t)
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
void to_big_endian(char *buf, Fixed t)
{
    to_big_endian<uint32_t>(buf, uint32_t(t));
}

template<>
void to_big_endian(char *buf, Tag t)
{
    buf[0] = t[0];
    buf[1] = t[1];
    buf[2] = t[2];
    buf[3] = t[3];
}

Buffer::Buffer(std::string && data)
    : arr(std::move(data))
{}

void Buffer::add_nbytes(int n, uint32_t t)
{
    if (n <= 0 || n > 4)
        throw std::runtime_error("cannot read n-byte integer");

    int shift = (n - 1) * 8;
    uint32_t mask = 0xff << shift;
    for (int i = 0; i < n; ++i)
    {
        add<uint8_t>((t & mask) >> shift);
        shift -= 8;
        mask >>= 8;
    }
}

void Buffer::append(const Buffer &buf)
{
    arr.append(buf.arr);
}

void Buffer::pad()
{
    size_t n = arr.size() % 4;
    if (n > 0) {
        arr.resize(arr.size() + (4 - n));
    }
}

uint32_t Buffer::read_nbytes(int n)
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

std::string Buffer::read_string(size_t length)
{
    std::string str(length, 0);
    read(&str[0], length);
    return str;
}

size_t Buffer::seek(size_t off)
{
    if (off > size())
        throw std::range_error("Invalid seek position.");
    size_t orig_pos = pos;
    pos = off;
    return orig_pos;
}

size_t Buffer::tell() const
{
    return pos;
}

size_t Buffer::size() const
{
    return arr.size();
}

char* Buffer::data()
{
    return &arr[0];
}

}
