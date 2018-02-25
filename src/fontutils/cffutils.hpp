#ifndef FONTUTILS_CFF_UTILS_HPP
#define FONTUTILS_CFF_UTILS_HPP

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "buffer.hpp"

namespace geul
{

class IndexIterator
{
public:
    /// constructs an end iterator
    IndexIterator() = default;

    IndexIterator(IndexIterator const& it) = default;

    IndexIterator(size_t count, int off_size, size_t offset_start, Buffer& dis);

    IndexIterator& operator++();

    bool operator!=(IndexIterator& rhs) const;

    struct OffsetData
    {
        size_t offset, length, index;
    };

    OffsetData operator*() const;

private:
    size_t       index = 0;
    const size_t off_size = 0, count = 0;
    const size_t offset_start = 0;

    Buffer* dis = nullptr;
};

struct IndexView
{
    int     count = 0;
    int     off_size;
    size_t  offset_start;
    Buffer* dis = nullptr;

    IndexIterator begin() const;

    IndexIterator end() const;
};

IndexView parse_index(Buffer& dis);

class CFFToken
{
public:
    enum class Op
    {
        // clang-format off
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
        // clang-format on
    };

    enum Type
    {
        op = 1,
        integer = 2,
        floating = 4,
        number = Type::integer | Type::floating
    };

    CFFToken(Op op);

    CFFToken(int num);

    CFFToken(double flnum);

    CFFToken(CFFToken const&) = default;

    Op get_op();

    int to_int();

    double to_double();

    Type get_type();

private:
    Type type;

    union
    {
        Op     op;
        int    integer;
        double floating;
    } value;
};

CFFToken next_token(Buffer& dis);

Buffer write_index(std::vector<Buffer>&& data);

void write_token(Buffer& buf, CFFToken token);
} // namespace fontutils

#endif
