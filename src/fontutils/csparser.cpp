#include "csparser.hpp"
#include "buffer.hpp"

#include <algorithm>
#include <deque>
#include <sstream>
#include <vector>

namespace fontutils
{

enum class Op
{
    // clang-format off
    // one-byte ops
    hstem = 0x01, vstem = 0x03, vmoveto, rlineto, hlineto,
    vlineto, rrcurveto, callsubr = 0x0a, return_,
    endchar = 0x0e, hstemhm = 0x12, hintmask, cntrmask,
    rmoveto, hmoveto, vstemhm, rcurveline, rlinecurve,
    vvcurveto, hhcurveto, callgsubr = 0x1d, vhcurveto,
    hvcurveto,

    // two-byte ops
    and_ = 0x0c03, or_, not_, abs = 0x0c09, add, sub,
    div, neg = 0x0c0e, eq, drop = 0x0c12, put = 0x0c14,
    get, ifelse, random, mul, sqrt = 0x0c1a, dup, exch,
    index, roll, hflex = 0x0c22, flex, hflex1, flex1
    // clang-format on
};

static bool is_number(Buffer const& dis)
{
    auto byte = dis.peek<uint8_t>() & 0xff;
    return byte == 28 || 32 <= byte;
}

static int parse_number(Buffer& dis)
{
    auto b0 = dis.read<uint8_t>() & 0xff;
    // -107..+107
    if (32 <= b0 && b0 <= 246)
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
    else if (b0 == 255)
    {
        return dis.read<int32_t>();
    }
    else
        throw std::runtime_error("The next token is not a valid number.");
}

static Op parse_op(Buffer& dis)
{
    auto b0 = dis.read<uint8_t>() & 0xff;
    // two-byte operators
    if (b0 == 12)
    {
        auto b1 = dis.read<uint8_t>() & 0xff;
        return Op(b0 << 8 | b1);
    }
    else if (b0 <= 11 || (13 <= b0 && b0 <= 27) || (29 <= b0 && b0 <= 31))
        return Op(b0);
    else
        throw std::runtime_error("The next token is not a valid op.");
}

static int get_subr_bias(int subr_count)
{
    int bias = 32768;
    if (subr_count < 1240)
        bias = 107;
    else if (subr_count < 33900)
        bias = 1131;
    return bias;
}

struct ParseState
{
    std::deque<int> stack = {};
    Point pos = { 0, 0 };
    int op_index = 0;
    Glyph glyph = {};
    int width = -1; // FIXME
    int n_hints = 0;
    bool finished = false;
};

static void call_subroutine(
    std::string cs,
    std::vector<std::string> const& gsubrs,
    std::vector<std::string> const& lsubrs,
    ParseState& state)
{
    Buffer buf(std::move(cs));

    auto& stack = state.stack;
    auto& pos = state.pos;
    auto& glyph = state.glyph;

    while (!state.finished)
    {
        if (is_number(buf))
        {
            stack.push_back(parse_number(buf));
            continue;
        }

        Op op = parse_op(buf);

        if (state.op_index == 0)
        {
            bool is_even_op = op == Op::hstem || op == Op::hstemhm
                              || op == Op::vstem || op == Op::vstemhm
                              || op == Op::cntrmask || op == Op::hintmask
                              || op == Op::rmoveto || op == Op::endchar;

            bool one_arg_op = op == Op::hmoveto || op == Op::vmoveto;

            bool is_subr_op = op == Op::endchar || op == Op::callsubr
                              || op == Op::callgsubr || op == Op::return_;

            if (!is_even_op && !one_arg_op && !is_subr_op)
                throw std::runtime_error("Invalid first operator.");

            if ((stack.size() % 2 == 1 && is_even_op)
                || (stack.size() == 2 && one_arg_op))
            {
                state.width = stack[0];
                stack.pop_front();
            }
        }

        if (op == Op::rmoveto)
        {
            if (stack.size() != 2)
            {
                std::ostringstream os;
                os << "incorrect number of arguments for rmoveto: "
                   << stack.size();
                throw std::invalid_argument(os.str());
            }

            pos.x += stack[0];
            pos.y += stack[1];
            glyph.paths.emplace_back(pos);
            stack.clear();
        }
        else if (op == Op::hmoveto)
        {
            if (stack.size() != 1)
                throw std::invalid_argument(
                    "incorrect number of arguments for hmoveto");

            pos.x += stack[0];
            glyph.paths.emplace_back(pos);
            stack.clear();
        }
        else if (op == Op::vmoveto)
        {
            if (stack.size() != 1)
                throw std::invalid_argument(
                    "incorrect number of arguments for vmoveto");

            pos.y += stack[0];
            glyph.paths.emplace_back(pos);
            stack.clear();
        }
        else if (op == Op::rlineto)
        {
            if (glyph.paths.empty())
                glyph.paths.emplace_back(pos);

            if (stack.empty() || stack.size() % 2)
                throw std::invalid_argument(
                    "incorrect number of arguments for rlineto");

            // lines
            for (int i = 0; i < int(stack.size()); i += 2)
            {
                pos.x += stack[i], pos.y += stack[i + 1];
                glyph.paths.back().lineto(pos);
            }
            stack.clear();
        }
        else if (op == Op::hlineto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty())
                throw std::invalid_argument(
                    "incorrect number of arguments for hlineto");

            // alternating horizontal/vertical lines
            for (int i = 0; i < int(stack.size()); i += 2)
            {
                // horizontal line
                pos.x += stack[i];
                glyph.paths.back().lineto(pos);

                // vertical line
                if (i + 1 < int(stack.size()))
                {
                    pos.y += stack[i + 1];
                    glyph.paths.back().lineto(pos);
                }
            }
            stack.clear();
        }
        else if (op == Op::vlineto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty())
                throw std::invalid_argument(
                    "incorrect number of arguments for vlineto");

            // alternating vertical/horizontal lines
            for (int i = 0; i < int(stack.size()); i += 2)
            {
                // vertical line
                pos.y += stack[i];
                glyph.paths.back().lineto(pos);

                // horizontal line
                if (i + 1 < int(stack.size()))
                {
                    pos.x += stack[i + 1];
                    glyph.paths.back().lineto(pos);
                }
            }
            stack.clear();
        }
        else if (op == Op::rrcurveto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty() || stack.size() % 6)
                throw std::invalid_argument(
                    "incorrect number of arguments for rrcurveto");

            // Bezier curves
            for (int i = 0; i < int(stack.size()); i += 6)
            {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2], pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4], pos.y += stack[i + 5];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }
            stack.clear();
        }
        else if (op == Op::hhcurveto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty() || stack.size() % 4 > 1)
                throw std::invalid_argument(
                    "invalid number of arguments for hhcurveto");

            if (stack.size() % 4 == 1)
            {
                pos.y += stack[0];
                stack.pop_front();
            }

            for (int i = 0; i < int(stack.size()); i += 4)
            {
                pos.x += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.x += stack[i + 3];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }
            stack.clear();
        }
        else if (op == Op::hvcurveto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty() || (stack.size() % 8 != 0 && stack.size() % 8 != 1
                                  && stack.size() % 8 != 4
                                  && stack.size() % 8 != 5))
                throw std::invalid_argument(
                    "invalid number of arguments for hvcurveto");

            // alternate start horizontal, end vertical and
            // start vertical, end horizontal
            for (int i = 0; i < int(stack.size()); i += 8)
            {
                pos.x += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.y += stack[i + 3];
                if (i + 5 == int(stack.size()))
                    pos.x += stack[i + 4];
                glyph.paths.back().curveto(ct1, ct2, pos);

                if (int(stack.size()) < i + 8)
                    break;

                pos.y += stack[i + 4];
                ct1 = pos;
                pos.x += stack[i + 5], pos.y += stack[i + 6];
                ct2 = pos;
                pos.x += stack[i + 7];
                if (i + 9 == int(stack.size()))
                    pos.y += stack[i + 8];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }

            stack.clear();
        }
        else if (op == Op::rcurveline)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() % 6 != 2)
                throw std::invalid_argument(
                    "invalid number of arguments for rcurveline");

            // Bezier curves
            for (int i = 0; i < int(stack.size()) - 2; i += 6)
            {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2], pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4], pos.y += stack[i + 5];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }

            // followed by a line
            pos.x += stack[stack.size() - 2];
            pos.y += stack[stack.size() - 1];
            glyph.paths.back().lineto(pos);

            stack.clear();
        }
        else if (op == Op::rlinecurve)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() < 8 || stack.size() % 2)
                throw std::invalid_argument(
                    "invalid number of arguments for rlinecurve");

            // lines
            for (int i = 0; i < int(stack.size()) - 6; i += 2)
            {
                pos.x += stack[i], pos.y += stack[i + 1];
                glyph.paths.back().lineto(pos);
            }

            // followed by a curve
            pos.x += stack[stack.size() - 6];
            pos.y += stack[stack.size() - 5];
            Point ct1 = pos;
            pos.x += stack[stack.size() - 4];
            pos.y += stack[stack.size() - 3];
            Point ct2 = pos;
            pos.x += stack[stack.size() - 2];
            pos.y += stack[stack.size() - 1];
            glyph.paths.back().curveto(ct1, ct2, pos);

            stack.clear();
        }
        else if (op == Op::vhcurveto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty() || (stack.size() % 8 != 0 && stack.size() % 8 != 1
                                  && stack.size() % 8 != 4
                                  && stack.size() % 8 != 5))
                throw std::invalid_argument(
                    "invalid number of arguments for vhcurveto");

            // alternate start vertical, end horizontal and
            // start horizontal, end vertical
            for (int i = 0; i < int(stack.size()); i += 8)
            {
                pos.y += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.x += stack[i + 3];
                if (i + 5 == int(stack.size()))
                    pos.y += stack[i + 4];
                glyph.paths.back().curveto(ct1, ct2, pos);

                if (int(stack.size()) < i + 8)
                    break;

                pos.x += stack[i + 4];
                ct1 = pos;
                pos.x += stack[i + 5], pos.y += stack[i + 6];
                ct2 = pos;
                pos.y += stack[i + 7];
                if (i + 9 == int(stack.size()))
                    pos.x += stack[i + 8];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }

            stack.clear();
        }
        else if (op == Op::vvcurveto)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.empty()
                || (stack.size() % 4 != 0 && stack.size() % 4 != 1))
                throw std::invalid_argument(
                    "invalid number of arguments for vvcurveto");

            int i = 0;
            if (stack.size() % 4 == 1)
            {
                pos.x += stack[0];
                ++i;
            }

            for (; i < int(stack.size()); i += 4)
            {
                pos.y += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.y += stack[i + 3];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }

            stack.clear();
        }
        else if (op == Op::flex)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() != 13)
                std::invalid_argument("invalid number of arguments for flex");

            for (int i = 0; i < int(stack.size()) - 1; i += 6)
            {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2];
                pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4];
                pos.y += stack[i + 5];
                glyph.paths.back().curveto(ct1, ct2, pos);
            }

            // TODO: take care of "flex depth"

            stack.clear();
        }
        else if (op == Op::hflex)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() != 7)
                std::invalid_argument("invalid number of arguments for hflex");

            Point orig = pos;

            pos.x += stack[0];
            Point ct1 = pos;
            pos.x += stack[1], pos.y += stack[2];
            Point ct2 = pos;
            pos.x += stack[3];
            glyph.paths.back().curveto(ct1, ct2, pos);

            pos.x += stack[4];
            ct1 = pos;
            pos.x += stack[5], pos.y = orig.y;
            ct2 = pos;
            pos.x += stack[6];
            glyph.paths.back().curveto(ct1, ct2, pos);

            // TODO: fd = 50

            stack.clear();
        }
        else if (op == Op::hflex1)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() != 9)
                std::invalid_argument("invalid number of arguments for hflex1");

            Point orig = pos;

            pos.x += stack[0], pos.y += stack[1];
            Point ct1 = pos;
            pos.x += stack[2];
            pos.y += stack[3];
            Point ct2 = pos;
            pos.x += stack[4];
            glyph.paths.back().curveto(ct1, ct2, pos);

            pos.x += stack[5];
            ct1 = pos;
            pos.x += stack[6], pos.y = orig.y;
            ct2 = pos;
            glyph.paths.back().curveto(ct1, ct2, pos);

            // TODO: fd = 50

            stack.clear();
        }
        else if (op == Op::flex1)
        {
            if (glyph.paths.empty())
                glyph.paths.push_back(Path(pos));

            if (stack.size() != 11)
                throw std::invalid_argument(
                    "invalid number of arguments for flex1");

            Point orig = pos;

            pos.x += stack[0], pos.y += stack[1];
            Point ct1 = pos;
            pos.x += stack[2], pos.y += stack[3];
            Point ct2 = pos;
            pos.x += stack[4], pos.y += stack[5];
            glyph.paths.back().curveto(ct1, ct2, pos);

            pos.x += stack[6], pos.y += stack[7];
            ct1 = pos;
            pos.x += stack[8], pos.y += stack[9];
            ct2 = pos;

            if (std::abs(pos.x - orig.x) > std::abs(pos.y - orig.y))
                pos.x += stack[10], pos.y = orig.y;
            else
                pos.x = orig.x, pos.y += stack[10];
            glyph.paths.back().curveto(ct1, ct2, pos);

            // TODO: fd = 50

            stack.clear();
        }
        else if (op == Op::endchar)
        {
            if (!stack.empty())
                throw std::invalid_argument(
                    "stack not empty when finishing glyph");
            state.finished = true;
            break;
        }
        else if (op == Op::hstem)
        {
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for hstem");
            state.n_hints += stack.size() / 2;
            stack.clear();
        }
        else if (op == Op::vstem)
        {
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for vstem");
            state.n_hints += stack.size() / 2;
            stack.clear();
        }
        else if (op == Op::hstemhm)
        {
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for hstemhm");
            state.n_hints += stack.size() / 2;
            stack.clear();
        }
        else if (op == Op::vstemhm)
        {
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for vstemhm");
            state.n_hints += stack.size() / 2;
            stack.clear();
        }
        else if (op == Op::hintmask)
        {
            // arguments for omitted vstem op
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for hintmask");
            state.n_hints += stack.size() / 2;

            // read mask
            int n_bytes = (state.n_hints + 7) / 8;
            for (int i = 0; i < n_bytes; ++i)
                buf.read<uint8_t>();
            stack.clear();
        }
        else if (op == Op::cntrmask)
        {
            // arguments for omitted vstem op
            if (stack.size() % 2 == 1)
                throw std::invalid_argument(
                    "invalid number of arguments for cntrmask");
            state.n_hints += stack.size() / 2;

            // read mask
            int n_bytes = (state.n_hints + 7) / 8;
            for (int i = 0; i < n_bytes; ++i)
                buf.read<uint8_t>();
            stack.clear();
        }
        else if (op == Op::callgsubr)
        {
            size_t idx = stack.back() + get_subr_bias(gsubrs.size());
            stack.pop_back();
            if (idx >= gsubrs.size())
                throw std::invalid_argument("gsubr index out of bounds");
            call_subroutine(gsubrs[idx], gsubrs, lsubrs, state);
            continue;
        }
        else if (op == Op::callsubr)
        {
            size_t idx = stack.back() + get_subr_bias(lsubrs.size());
            stack.pop_back();
            if (idx >= lsubrs.size())
                throw std::invalid_argument("lsubr index out of bounds");
            call_subroutine(lsubrs[idx], gsubrs, lsubrs, state);
            continue;
        }
        else if (op == Op::return_)
        {
            break;
        }
        else
        {
            throw std::runtime_error("Unimplemented operator");
        }
        state.op_index++;
    }
    if (buf.tell() != buf.size())
        throw std::runtime_error("Stray data after call to return or endchar");
}

Glyph parse_charstring(
    std::string const& cs,
    std::vector<std::string> const& gsubrs,
    std::vector<std::string> const& lsubrs)
{
    ParseState state;
    call_subroutine(cs, gsubrs, lsubrs, state);
    if (!state.finished)
        throw std::runtime_error("premature end of charstring parsing");
    return state.glyph;
}

static void write_number(Buffer &buf, int val)
{
    if (-107 <= val && val <= 107)
    {
        buf.add<uint8_t>(val + 139);
    }
    else if (108 <= val && val <= 1131)
    {
        buf.add<uint8_t>(((val - 108) >> 8) + 247);
        buf.add<uint8_t>((val - 108) & 0xff);
    }
    else if (-1131 <= val && val <= -108)
    {
        buf.add<uint8_t>(((-val - 108) >> 8) + 251);
        buf.add<uint8_t>((-val - 108) & 0xff);
    }
    else if (-32768 <= val && val <= 32767)
    {
        buf.add<uint8_t>(28);
        buf.add<int16_t>(val);
    }
    else
    {
        buf.add<uint8_t>(255);
        buf.add<int32_t>(val);
    }
}

static void write_op(Buffer &buf, Op op)
{
    int int_op = int(op);
    if (int_op & 0xff00)
        buf.add<uint16_t>(int_op);
    else
        buf.add<uint8_t>(int_op);
}

Buffer write_charstring(Glyph const& glyph)
{
    Buffer buf;
    Point pos = {0, 0};
    for (auto const& path : glyph.paths)
    {
        write_number(buf, path.start.x - pos.x);
        write_number(buf, path.start.y - pos.y);
        write_op(buf, Op::rmoveto);
        pos = path.start;
    }

    write_op(buf, Op::endchar);
    return buf;
}
}
