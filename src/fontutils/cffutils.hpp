#ifndef FONTUTILS_CFF_UTILS_HPP
#define FONTUTILS_CFF_UTILS_HPP

#include <cstddef>
#include <vector>
#include <functional>

#include "buffer.hpp"

namespace geul
{

class IndexIterator
{
public:
    /// constructs an end iterator
    IndexIterator() = default;

    IndexIterator(IndexIterator const& it) = default;

    IndexIterator(
        std::size_t count, int off_size, std::size_t offset_start, InputBuffer& dis);

    IndexIterator& operator++();

    bool operator!=(IndexIterator& rhs) const;

    struct OffsetData
    {
        std::streampos pos;
        std::size_t length, index;
    };

    OffsetData operator*() const;

private:
    std::size_t       index = 0;
    const std::size_t off_size = 0, count = 0;
    const std::size_t offset_start = 0;

    InputBuffer* dis = nullptr;
};

struct IndexView
{
    int            count = 0;
    int            off_size;
    std::streampos offset_start;
    InputBuffer*   dis = nullptr;

    IndexIterator begin() const;

    IndexIterator end() const;
};

IndexView parse_index(InputBuffer& dis);

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

CFFToken next_token(InputBuffer& dis);

void write_index(
    OutputBuffer& out, int size, std::function<void(int)> cb);

void write_token(OutputBuffer& out, CFFToken token);

void write_5byte_offset_at(OutputBuffer& out, std::streampos pos, int val);
} // namespace fontutils

#endif
