#include "cfftable.hpp"

#include <vector>
#include <iterator>

#include "stdstr.hpp"

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
    { return dis != rhs.dis; }

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

static IndexView parse_index(Buffer &dis)
{
    auto beginning = dis.tell();
    auto count = dis.read<uint16_t>();
    if (count == 0) return IndexView{};

    auto off_size = dis.read<uint8_t>();
    auto first_offset = dis.read_nbytes(off_size);
    if (first_offset != 1)
        throw std::runtime_error("Invalid INDEX");

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

    Op get_op()
    {
        if (type != op) throw std::runtime_error("type != op");
        return value.op;
    }

    int to_int()
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

    double to_double()
    {
        if (type == integer)
            return value.integer;
        else if (type == floating)
            return value.floating;
        else
            throw std::runtime_error("type is not convertible to double");
    }
};

static CFFToken next_token(Buffer &dis)
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

    auto const beginning = dis.tell();

    auto major = dis.read<uint8_t>();
    if (major != 1)
        throw std::runtime_error("Unrecognized major CFF table version");

    // minor version (ignored)
    dis.read<uint8_t>();

    auto header_size = dis.read<uint8_t>();

    // offset size (FIXME: unused)
    dis.read<uint8_t>();

    // Seek to name index
    dis.seek(beginning + header_size);

    auto name_index = parse_index(dis);
    auto num_fonts = name_index.count;
    fonts.resize(num_fonts);

    for (auto name : name_index)
    {
        auto orig_pos = dis.seek(name.offset);
        fonts[name.index].name = dis.read_string(name.length);
        dis.seek(orig_pos);
    }

    // parse top dict index
    auto dict_index = parse_index(dis);

    // parse sid strings index
    auto string_index = parse_index(dis);
    std::vector<std::string> sid{std::begin(standard_strings), std::end(standard_strings)};
    for (auto str : string_index)
    {
        auto orig_pos = dis.seek(str.offset);
        sid.push_back(dis.read_string(str.length));
        dis.seek(orig_pos);
    }

    // parse top dict
    for (auto dict : dict_index)
    {
        auto orig_pos = dis.seek(dict.offset);
        auto &font = fonts[dict.index];
        auto &fontinfo = font.fontinfo;

        int charset_offset = -1;
        size_t charstrings_offset;
        size_t fdarray_offset;
        size_t fdselect_offset;

        std::vector<CFFToken> operands;
        int is_first_op = true;
        while (dis.tell() < dict.offset + dict.length)
        {
            auto token = next_token(dis);
            if (token.type & CFFToken::number)
            {
                operands.push_back(token);
                continue;
            }

            // parse operator
            CFFToken::Op op = token.get_op();

            // Should start with 'ros' operator for CID,
            //  'syntheticbase' for Synthetic fonts,
            //  and other for type 1.
            if (is_first_op)
            {
                if (op != CFFToken::Op::ros)
                    throw std::runtime_error("Font is not CID-keyed.");
                is_first_op = false;
            }

            if (op == CFFToken::Op::version)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'version' operands != 1");
                fontinfo.version = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::notice)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'notice' operands != 1");
                fontinfo.notice = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::copyright)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'copyright' operands != 1");
                fontinfo.copyright = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::fullname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'fullname' operands != 1");
                fontinfo.fullname = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::familyname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'familyname' operands != 1");
                fontinfo.familyname = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::weight)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'weight' operands != 1");
                fontinfo.weight = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::isfixedpitch)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'isfixedpitch' operands != 1");
                fontinfo.is_fixed_pitch = operands[0].to_int();
            }
            else if (op == CFFToken::Op::italicangle)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'italicangle' operands != 1");
                fontinfo.italic_angle = operands[0].to_int();
            }
            else if (op == CFFToken::Op::underlineposition)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'underlineposition' operands != 1");
                fontinfo.underline_position = operands[0].to_int();
            }
            else if (op == CFFToken::Op::underlinethickness)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'underlinethickness' operands != 1");
                fontinfo.underline_thickness = operands[0].to_int();
            }
            else if (op == CFFToken::Op::painttype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'painttype' operands != 1");
                fontinfo.paint_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::charstringtype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'isfixedpitch' operands != 1");
                fontinfo.charstring_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fontmatrix)
            {
                if (operands.size() != 6)
                    throw std::runtime_error("number of 'fontmatrix' operands != 6");
                for (int i = 0; i < 6; ++i)
                    fontinfo.font_matrix[i] = operands[i].to_double();
            }
            else if (op == CFFToken::Op::uniqueid)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'uniqueid' operands != 1");
                fontinfo.unique_id = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fontbbox)
            {
                if (operands.size() != 4)
                    throw std::runtime_error("number of 'fontbbox' operands != 4");
                for (int i = 0; i < 4; ++i)
                    fontinfo.font_bbox[i] = operands[i].to_int();
            }
            else if (op == CFFToken::Op::strokewidth)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'strokewidth' operands != 1");
                fontinfo.stroke_width = operands[0].to_int();
            }
            else if (op == CFFToken::Op::xuid)
            {
                for (auto &x : operands)
                    fontinfo.xuid.push_back(x.to_int());
            }
            else if (op == CFFToken::Op::charset)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'charset' operands != 1");
                auto charset = operands[0].to_int();
                if (charset > 2)
                    charset_offset = beginning + charset;
                else // TODO: implement standard charset
                    throw std::runtime_error("standard charset unimplemented");
            }
            else if (op == CFFToken::Op::encoding)
            {
                throw std::runtime_error("invalid operand 'encoding'");
            }
            else if (op == CFFToken::Op::charstrings)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'charstrings' operands != 1");
                charstrings_offset = beginning + operands[0].to_int();
            }
            else if (op == CFFToken::Op::syntheticbase)
            {
                throw std::runtime_error("invalid operand 'syntheticbase'");
            }
            else if (op == CFFToken::Op::postscript)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'postscript operands != 1");
                fontinfo.postscript = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::basefontname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'basefontname' operands != 1");
                fontinfo.basefont_name = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::basefontblend)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'basefontblend' operands != 1");
                fontinfo.basefont_blend = operands[0].to_int();
            }
            else if (op == CFFToken::Op::ros)
            {
                if (operands.size() != 3)
                    throw std::runtime_error("number of 'ros' operands != 3");
                fontinfo.registry = sid.at(operands[0].to_int());
                fontinfo.ordering = sid.at(operands[1].to_int());
                fontinfo.supplement = operands[2].to_int();
            }
            else if (op == CFFToken::Op::cidfontversion)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'cidfontversion' operands != 1");
                fontinfo.cid_font_version = operands[0].to_double();
            }
            else if (op == CFFToken::Op::cidfontrevision)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'cidfontrevision' operands != 1");
                fontinfo.cid_font_revision = operands[0].to_double();
            }
            else if (op == CFFToken::Op::cidfonttype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'cidfonttype' operands != 1");
                fontinfo.cid_font_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::cidcount)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'cidcount' operands != 1");
                fontinfo.cid_count = operands[0].to_int();
            }
            else if (op == CFFToken::Op::uidbase)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'uidbase' operands != 1");
                fontinfo.uid_base = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fdarray)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'fdarray' operands != 1");
                fdarray_offset = beginning + operands[0].to_int();
            }
            else if (op == CFFToken::Op::fdselect)
            {
                if (operands.size() != 1)
                    throw std::runtime_error("number of 'fdselect' operands != 1");
                fdselect_offset = beginning + operands[0].to_int();
            }
            else
            {
                throw std::runtime_error("Unknown operand in top dict.");
            }
            operands.clear();
        }

        // parse charstrings index
        dis.seek(charstrings_offset);
        auto cs_index = parse_index(dis);
        auto n_glyphs = cs_index.count;
        font.charstrings.resize(n_glyphs);
        for (auto item : cs_index)
        {
            dis.seek(item.offset);
            font.charstrings[item.index] = dis.read_string(item.length);
        }

        // parse charset
        font.charset.resize(n_glyphs);
        font.charset[0] = 0;

        dis.seek(charset_offset);
        auto charset_format = dis.read<uint8_t>();
        if (charset_format == 0)
        {
            for (int i = 1; i < n_glyphs; ++i)
                font.charset[i] = dis.read<uint16_t>();
        }
        else if (charset_format == 1)
        {
            for (int i = 1; i < n_glyphs;)
            {
                auto s = dis.read<uint16_t>();
                int n_left = dis.read<uint16_t>();
                while (i < n_glyphs && s <= s + n_left)
                    font.charset[i++] = s++;
            }
        }
        else
            throw std::runtime_error("Unrecognized charset format");

        // parse fdselect
        font.fd_select.resize(n_glyphs);

        dis.seek(fdselect_offset);
        auto fdselect_format = dis.read<uint8_t>();
        if (fdselect_format == 0)
        {
            for (int i = 0; i < n_glyphs; ++i)
                font.fd_select[i] = dis.read<uint8_t>();
        }
        else if (fdselect_format == 3)
        {
            int n_ranges = dis.read<uint16_t>();
            for (int i = 0; i < n_ranges; ++i)
            {
                int first = dis.read<uint16_t>();
                auto fd = dis.read<uint8_t>();
                int end = dis.peek<uint16_t>();
                for (int j = first; j < end; ++j)
                    font.fd_select[j] = fd;
            }
        }
        else
            throw std::runtime_error("Unrecognized fdselect format");

        // parse fdarray
        dis.seek(fdarray_offset);
        auto fd_index = parse_index(dis);
        font.fd_array.resize(fd_index.count);
        for (auto item : fd_index)
        {
            auto &font_dict = font.fd_array[item.index];
            size_t priv_size, priv_offset;

            dis.seek(item.offset);
            std::vector<CFFToken> operands;
            while (dis.tell() < item.offset + item.length)
            {
                auto token = next_token(dis);
                if (token.type & CFFToken::number)
                {
                    operands.push_back(token);
                    continue;
                }

                // parse operator
                CFFToken::Op op = token.get_op();
                if (op == CFFToken::Op::fontname)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'fontname' operands != 1");
                    font_dict.name = sid.at(operands[0].to_int());
                }
                else if (op == CFFToken::Op::private_)
                {
                    priv_size = operands[0].to_int();
                    priv_offset = beginning + operands[1].to_int();
                }
                else
                {
                    throw std::runtime_error("Unknown operand in font dict.");
                }
                operands.clear();
            }

            // parse private dict
            auto &priv = font_dict.priv;
            size_t subrs_offset;

            operands.clear();
            dis.seek(priv_offset);
            while (dis.tell() < priv_offset + priv_size)
            {
                auto token = next_token(dis);
                if (token.type & CFFToken::number)
                {
                    operands.push_back(token);
                    continue;
                }

                // parse operator
                CFFToken::Op op = token.get_op();
                if (op == CFFToken::Op::bluevalues)
                {
                    priv.blue_values.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.blue_values.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::otherblues)
                {
                    priv.other_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.other_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::familyblues)
                {
                    priv.family_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.family_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::familyotherblues)
                {
                    priv.family_other_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.family_other_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::bluescale)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'bluescale' operands != 1");
                    priv.blue_scale = operands[0].to_double();
                }
                else if (op == CFFToken::Op::blueshift)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'blueshift' operands != 1");
                    priv.blue_shift = operands[0].to_double();
                }
                else if (op == CFFToken::Op::bluefuzz)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'bluefuzz' operands != 1");
                    priv.blue_fuzz = operands[0].to_double();
                }
                else if (op == CFFToken::Op::stdhw)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'stdhw' operands != 1");
                    priv.std_hw = operands[0].to_int();
                }
                else if (op == CFFToken::Op::stdvw)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'stdvw' operands != 1");
                    priv.std_vw = operands[0].to_int();
                }
                else if (op == CFFToken::Op::stemsnaph)
                {
                    priv.stem_snap_h.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.stem_snap_h.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::stemsnapv)
                {
                    priv.stem_snap_v.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        priv.stem_snap_v.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::forcebold)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'forcebold' operands != 1");
                    priv.force_bold = operands[0].to_int();
                }
                else if (op == CFFToken::Op::languagegroup)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'languagegroup' operands != 1");
                    priv.language_group = operands[0].to_int();
                }
                else if (op == CFFToken::Op::expansionfactor)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'expansionfactor' operands != 1");
                    priv.expansion_factor = operands[0].to_double();
                }
                else if (op == CFFToken::Op::initialrandomseed)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'initialrandomseed' operands != 1");
                    priv.initial_random_seed = operands[0].to_int();
                }
                else if (op == CFFToken::Op::subrs)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'subrs' operands != 1");
                    subrs_offset = beginning + operands[0].to_int();
                }
                else if (op == CFFToken::Op::defaultwidthx)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'defaultwidthx' operands != 1");
                    priv.default_width_x = operands[0].to_int();
                }
                else if (op == CFFToken::Op::nominalwidthx)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error("number of 'nominalwidthx' operands != 1");
                    priv.nominal_width_x = operands[0].to_int();
                }
                else
                {
                    throw std::runtime_error("Unknown operand in private dict.");
                }
                operands.clear();
            } // private dict

            // parse local subroutines
            dis.seek(subrs_offset);
            auto subrs_index = parse_index(dis);
            for (auto item : subrs_index)
            {
                dis.seek(item.offset);
                priv.subrs.push_back(dis.read_string(item.length));
            }
        } // fdarray

        dis.seek(orig_pos);
    }

    // parse global subroutines
    auto gsubr_index = parse_index(dis);
    for (auto item : gsubr_index)
    {
        auto orig_pos = dis.seek(item.offset);
        gsubrs.push_back(dis.read_string(item.length));
        dis.seek(orig_pos);
    }
}

Buffer CFFTable::compile() const
{
    // TODO
    Buffer buf;
    return buf;
}

}
