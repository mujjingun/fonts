#include "cfftable.hpp"

#include <vector>

namespace fontutils
{

CFFTable::CFFTable()
{
    id = "CFF ";
}

class IndexIterator
{
public:
    // constructs the end iterator
    IndexIterator()
    {}

    IndexIterator(size_t count, int off_size, size_t offset_start, Buffer &dis)
        : off_size(off_size), count(count), offset_start(offset_start), dis(&dis)
    {}

    IndexIterator(IndexIterator const& it) = default;

    IndexIterator &operator++()
    {
        // reached the end
        if (++index == count) dis = nullptr;
        return *this;
    }

    bool operator!=(IndexIterator &rhs) const
    {
        return dis || rhs.dis;
    }

    struct OffsetData
    {
        size_t offset, length, index;
    };

    OffsetData operator* () const
    {
        if (!dis)
            throw std::runtime_error("attempt to dereference an end iterator");

        size_t cur = offset_start + off_size * (index - count - 1) + 1;
        auto orig_pos = dis->seek(cur);

        auto offset = dis->read_nbytes(off_size);
        auto length = dis->read_nbytes(off_size) - offset;
        offset += offset_start;

        dis->seek(orig_pos);

        return {offset, length, index};
    }

private:
    size_t index = 0;
    const size_t off_size = 0, count = 0;
    const size_t offset_start = 0;

    Buffer *dis = nullptr;
};

struct IndexView
{
    int count = 0;

    int off_size;
    size_t offset_start;

    Buffer *dis = nullptr;

    IndexIterator begin() const
    {
        if (count) {
            return IndexIterator(count, off_size, offset_start, *dis);
        }
        else return IndexIterator();
    }

    IndexIterator end() const
    { return IndexIterator(); }
};

IndexView parse_index(Buffer &dis)
{
    auto beginning = dis.tell();
    auto count = dis.read<uint16_t>();
    if (count == 0) return IndexView{};

    auto off_size = dis.read<uint8_t>();
    auto offset_start = beginning + 2 + off_size * (count + 1);

    dis.seek(offset_start - off_size + 1);
    auto end = offset_start + dis.read_nbytes(off_size);
    dis.seek(end);

    return IndexView{count, off_size, offset_start, &dis};
}

struct CFFToken
{
    enum class Op
    {
        // one-byte ops
        version = 0x00, notice, fullname, familyname,
        weight, fontbbox, bluevalues, otherblues,
        familyblues, familyotherblues, stdhw, stdvw,
        uniqueid = 0x0d, xuid, charset, encoding,
        charstrings, private_, subrs, defaultwidthx,
        nominalwidthx,

        // two-byte ops
        copyright = 0x0c00, isfixedpitch, italicangle,
        underlineposition, underlinethickness, painttype,
        charstringtype, fontmatrix, strokewidth, bluescale,
        blueshift, bluefuzz, stemsnaph, stemsnapv, forcebold,
        languagegroup = 0x0c11, expansionfactor, initialrandomseed,
        syntheticbase, postscript, basefontname, basefontblend,
        ros = 0x0c1e, cidfontversion, cidfontrevision,
        cidfonttype, cidcount, uidbase, fdarray, fdselect,
        fontname
    };

    enum Type
    {
        op = 1,
        integer = 2,
        floating = 4,
        number = Type::integer | Type::floating
    } type;

    union {
        Op op;
        int integer;
        double floating;
    } value;

    CFFToken(Op op) : type(Type::op)
    { value.op = op; }

    CFFToken(int num) : type(Type::integer)
    { value.integer = num; }

    CFFToken(double flnum) : type(Type::floating)
    { value.floating = flnum; }

    CFFToken(CFFToken const&) = default;
};

CFFToken next_token(Buffer &dis)
{
    auto b0 = dis.read<uint8_t>() & 0xff;
    // two-byte operators
    if (b0 == 12)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        return static_cast<CFFToken::Op>(b0 << 8 | b1);
    }
    // one-byte operators
    if (b0 <= 21)
    {
        return static_cast<CFFToken::Op>(b0);
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
        auto b1 = dis.read<uint8_t>() & 0xff;
        auto b2 = dis.read<uint8_t>() & 0xff;
        return (b1 << 8) | b2;
    }
    // -2^31..+2^31-1
    else if (b0 == 29)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        auto b2 = dis.read<uint8_t>() & 0xff;
        auto b3 = dis.read<uint8_t>() & 0xff;
        auto b4 = dis.read<uint8_t>() & 0xff;
        return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    }
    // floating point
    else if (b0 == 30)
    {
        int byte = 0, t;
        std::string num;
        bool read_next = true;
        do {
            if (read_next) {
                byte = dis.read<uint8_t>() & 0xff;
                t = (byte & 0xf0) >> 4;
            }
            else t = byte & 0x0f;
            read_next = !read_next;
            if (t <= 0x9) num += t + '0';
            if (t == 0xa) num += '.';
            if (t == 0xb) num += 'E';
            if (t == 0xc) num += "E-";
            if (t == 0xe) num += '-';
        } while (t != 0xf);
        return std::stod(num);
    }
    else
    {
        throw std::runtime_error("reserved token");
    }
}

void CFFTable::parse(Buffer &dis)
{
    std::cout << "Parsing 'CFF '..." << std::endl;

    auto beginning = dis.tell();

    auto major = dis.read<uint8_t>();
    if (major != 1)
        throw std::runtime_error("Unrecognized major CFF table version");

    // minor version (ignored)
    dis.read<uint8_t>();

    auto header_size = dis.read<uint8_t>();
    auto offset_size = dis.read<uint8_t>();

    // Seek to name index
    dis.seek(beginning + header_size);

    auto name_index = parse_index(dis);
    fonts.resize(name_index.count);

    for (auto name : name_index)
    {
        auto orig_pos = dis.seek(name.offset);
        fonts[name.index].name = dis.read_string(name.length);
        dis.seek(orig_pos);
    }

    // parse top dict index
    auto dict_index = parse_index(dis);
    for (auto dict : dict_index)
    {
        auto orig_pos = dis.seek(dict.offset);
        std::vector<CFFToken> operands;
        while (dis.tell() < dict.offset + dict.length)
        {
            auto token = next_token(dis);
            if (token.type & CFFToken::number)
                operands.push_back(token);
            else
            {
                // parse operator
            }
        }
        dis.seek(orig_pos);
    }

    // TODO: parse string index
    auto string_index = parse_index(dis);
    for (auto str : string_index)
    {
        auto orig_pos = dis.seek(str.offset);
        auto s = dis.read_string(str.length);
        std::cout << s << std::endl;
        dis.seek(orig_pos);
    }

    // TODO: parse global subr index
}

Buffer CFFTable::compile() const
{
    // TODO
    Buffer buf;
    return buf;
}

}
