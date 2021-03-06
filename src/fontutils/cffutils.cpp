#include "cffutils.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace geul
{

IndexView parse_index(InputBuffer& dis)
{
    std::streampos beginning = dis.tell();

    int count = dis.read<uint16_t>();
    if (count == 0)
        return IndexView{};

    int  off_size = dis.read<uint8_t>();
    auto first_offset = dis.read_nint(off_size);
    if (first_offset != 1)
        throw std::runtime_error("Invalid INDEX");

    auto offset_start = beginning + std::streamoff(2 + off_size * (count + 1));

    dis.seek(offset_start - std::streamoff(off_size - 1));
    auto end = offset_start + std::streamoff(dis.read_nint(off_size));
    dis.seek(end);

    return IndexView{ count, off_size, offset_start, &dis };
}

IndexIterator::IndexIterator(
    std::size_t count, int off_size, std::size_t offset_start, InputBuffer& dis)
    : off_size(off_size)
    , count(count)
    , offset_start(offset_start)
    , dis(&dis)
{}

IndexIterator& IndexIterator::operator++()
{
    // reached the end
    if (++index == count)
        dis = nullptr;
    return *this;
}

bool IndexIterator::operator!=(IndexIterator& rhs) const
{
    return dis != rhs.dis;
}

IndexIterator::OffsetData IndexIterator::operator*() const
{
    if (!dis)
        throw std::runtime_error("attempt to dereference an end iterator");

    std::size_t cur = offset_start + off_size * (index - count - 1) + 1;
    auto   lock = dis->seek_lock(cur);

    auto offset = dis->read_nint(off_size);
    auto length = dis->read_nint(off_size) - offset;
    offset += offset_start;

    return { offset, length, index };
}

IndexIterator IndexView::begin() const
{
    if (count)
    {
        return IndexIterator(count, off_size, offset_start, *dis);
    }
    else
        return IndexIterator();
}

IndexIterator IndexView::end() const
{
    return IndexIterator();
}

CFFToken::CFFToken(Op op)
    : type(Type::op)
{
    value.op = op;
}

CFFToken::CFFToken(int num)
    : type(Type::integer)
{
    value.integer = num;
}

CFFToken::CFFToken(double flnum)
    : type(Type::floating)
{
    value.floating = flnum;
}

CFFToken::Type CFFToken::get_type()
{
    return type;
}

CFFToken::Op CFFToken::get_op()
{
    if (type != op)
        throw std::runtime_error("type != op");
    return value.op;
}

int CFFToken::to_int()
{
    if (type == integer)
        return value.integer;
    else if (type == floating)
    {
        std::cerr << "truncating float operand to int..." << std::endl;
        return value.floating;
    }
    else
        throw std::runtime_error("type is not convertible to int");
}

double CFFToken::to_double()
{
    if (type == integer)
        return value.integer;
    else if (type == floating)
        return value.floating;
    else
        throw std::runtime_error("type is not convertible to double");
}

CFFToken next_token(InputBuffer& dis)
{
    auto b0 = dis.read<uint8_t>() & 0xff;
    // two-byte operators
    if (b0 == 12)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        return CFFToken::Op(b0 << 8 | b1);
    }
    // one-byte operators
    if (b0 <= 21)
    {
        return CFFToken::Op(b0);
    }
    // -107..+107
    else if (32 <= b0 && b0 <= 246)
    {
        return b0 - 139;
    }
    // +108..+1131
    else if (247 <= b0 && b0 <= 250)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        return (b0 - 247) * 256 + b1 + 108;
    }
    // -1131..-108
    else if (251 <= b0 && b0 <= 254)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        return -(b0 - 251) * 256 - b1 - 108;
    }
    // -32768..+32767
    else if (b0 == 28)
    {
        return dis.read<int16_t>();
    }
    // -2^31..+2^31-1
    else if (b0 == 29)
    {
        return dis.read<int32_t>();
    }
    // floating point
    else if (b0 == 30)
    {
        int         byte = 0, t;
        std::string num;
        bool        read_next = true;
        do
        {
            if (read_next)
            {
                byte = dis.read<uint8_t>() & 0xff;
                t = (byte & 0xf0) >> 4;
            }
            else
                t = byte & 0x0f;
            read_next = !read_next;
            if (t <= 0x9)
                num += t + '0';
            if (t == 0xa)
                num += '.';
            if (t == 0xb)
                num += 'E';
            if (t == 0xc)
                num += "E-";
            if (t == 0xe)
                num += '-';
        } while (t != 0xf);
        return std::stod(num);
    }
    else
    {
        throw std::runtime_error("reserved token");
    }
}

void write_index(OutputBuffer& out, int size, std::function<void(int)> cb)
{
    auto beginning = out.tell();

    out.write<uint16_t>(size);

    if (size == 0)
        return;

    out.write<uint8_t>(4);

    auto offset_start = beginning + std::streamoff(2 + 4 * (size + 1));
    std::streamoff offset = 1;
    out.write<uint32_t>(offset);
    for (int i = 0; i < size; ++i)
    {
        {
            auto begin = offset_start + offset;
            auto lock = out.seek_lock(begin);
            cb(i);
            auto len = out.tell() - begin;
            offset += len;
        }

        out.write<uint32_t>(offset);
    }

    out.seek(offset_start + offset);
}

void write_token(OutputBuffer& out, CFFToken token)
{
    // op
    if (token.get_type() == CFFToken::Type::op)
    {
        int int_op = int(token.get_op());
        if (int_op & 0xff00)
            out.write<uint16_t>(int_op);
        else
            out.write<uint8_t>(int_op);
    }
    // integer
    else if (token.get_type() == CFFToken::Type::integer)
    {
        int val = token.to_int();

        if (-107 <= val && val <= 107)
        {
            out.write<uint8_t>(val + 139);
        }
        else if (108 <= val && val <= 1131)
        {
            out.write<uint8_t>(((val - 108) >> 8) + 247);
            out.write<uint8_t>((val - 108) & 0xff);
        }
        else if (-1131 <= val && val <= -108)
        {
            out.write<uint8_t>(((-val - 108) >> 8) + 251);
            out.write<uint8_t>((-val - 108) & 0xff);
        }
        else if (-32768 <= val && val <= 32767)
        {
            out.write<uint8_t>(28);
            out.write<int16_t>(val);
        }
        else
        {
            out.write<uint8_t>(29);
            out.write<int32_t>(val);
        }
    }
    // floating point
    else
    {
        out.write<uint8_t>(30);

        std::ostringstream oss;
        oss << std::scientific << token.to_double();
        std::string num = oss.str();

        int  byte = 0;
        bool new_byte = true;
        for (auto it = num.begin(); it != num.end(); ++it)
        {
            char c = *it;
            if ('0' <= c && c <= '9')
                byte |= c - '0';
            else if (c == '.')
                byte |= 0xa;
            else if (c == 'e')
            {
                if (*++it == '+')
                    byte |= 0xb;
                else
                    byte |= 0xc;
            }
            else if (c == '-')
            {
                byte |= 0xe;
            }

            if (!new_byte)
            {
                out.write<uint8_t>(byte);
                byte = 0;
            }
            else
                byte <<= 4;
            new_byte = !new_byte;
        }
        if (new_byte)
            byte = 0xff;
        else
            byte |= 0xf;
        out.write<uint8_t>(byte);
    }
}

void write_5byte_offset_at(OutputBuffer& out, std::streampos pos, int val)
{
    auto orig = out.seek(pos);
    out.write<uint8_t>(29);
    out.write<int32_t>(val);
    out.seek(orig);
}
}
