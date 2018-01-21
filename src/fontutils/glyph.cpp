#include "glyph.hpp"

#include <pugixml.hpp>
#include <exception>
#include <iostream>
#include <cstdio>
#include <deque>
#include <sstream>
#include <iomanip>
#include <bitset>

#include "subroutines.hpp"

namespace fontutils {

Path::Path(Point start)
    : start_(start), segments_{}
{ }

void Path::add(Line l)
{
    segments_.push_back(l);
}

void Path::add(CubicBezier b)
{
    segments_.push_back(b);
}

std::vector<Segment> Path::segments() const
{
    return segments_;
}

fontutils::Point Path::start() const
{
    return start_;
}

struct parse_state {
    std::deque<int> stack = {};
    Point pos = {0, 0};
    int command_index = 0;
    std::vector<Path> paths = {};
};

static void call_subroutine(
    std::string const& charstring,
    subroutine_set const& subroutines,
    int fd_index,
    parse_state &state)
{
    std::istringstream iss(charstring);
    int width = -1;

    auto &stack = state.stack;
    auto &pos = state.pos;
    auto &command_index = state.command_index;
    auto &paths = state.paths;

    for (;;) {
        std::string sarg;
        if (!(iss >> sarg)) break; // reached end of charstring

        // if sarg is an int
        int iarg;
        if (sscanf(sarg.c_str(), "%d", &iarg) == 1) {
            stack.push_back(iarg);
            continue;
        }

        // sarg is a command

        // starts with a width argument
        if (command_index == 0) {
            if ((stack.size() % 2 == 1 &&
                    (sarg == "hstem" || sarg == "hstemhm" || sarg == "vstem" || sarg == "vstemhm" ||
                     sarg == "cntrmask" || sarg == "hintmask" || sarg == "rmoveto" || sarg == "endchar")) ||
                (stack.size() == 2 && (sarg == "hmoveto" || sarg == "vmoveto"))) {
                width = stack[0];
                stack.pop_front();
            }
        }

        if (sarg == "rmoveto") {

            if (stack.size() != 2) {
                std::ostringstream os;
                os << "incorrect number of arguments for rmoveto: " << stack.size();
                throw std::invalid_argument(os.str());
            }

            pos.x += stack[0];
            pos.y += stack[1];
            paths.push_back(Path(pos));
            stack.clear();
        }
        else if (sarg == "hmoveto") {

            if (stack.size() != 1)
                throw std::invalid_argument("incorrect number of arguments for hmoveto");

            pos.x += stack[0];
            paths.push_back(Path(pos));
            stack.clear();
        }
        else if (sarg == "vmoveto") {

            if (stack.size() != 1)
                throw std::invalid_argument("incorrect number of arguments for vmoveto");

            pos.y += stack[1];
            paths.push_back(Path(pos));
            stack.clear();
        }
        else if (sarg == "rlineto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() || stack.size() % 2)
                throw std::invalid_argument("incorrect number of arguments for rlineto");

            // lines
            for (int i = 0; i < int(stack.size()); i += 2) {
                pos.x += stack[i], pos.y += stack[i + 1];
                paths.back().add(Line{pos});
            }
            stack.clear();
        }
        else if (sarg == "hlineto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty())
                throw std::invalid_argument("incorrect number of arguments for hlineto");

            // alternating horizontal/vertical lines
            for (int i = 0; i < int(stack.size()); i += 2) {
                // horizontal line
                pos.x += stack[i];
                paths.back().add(Line{pos});

                // vertical line
                if (i + 1 < int(stack.size())) {
                    pos.y += stack[i + 1];
                    paths.back().add(Line{pos});
                }
            }
            stack.clear();
        }
        else if (sarg == "vlineto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty())
                throw std::invalid_argument("incorrect number of arguments for vlineto");

            // alternating vertical/horizontal lines
            for (int i = 0; i < int(stack.size()); i += 2) {
                // vertical line
                pos.y += stack[i];
                paths.back().add(Line{pos});

                // horizontal line
                if (i + 1 < int(stack.size())) {
                    pos.x += stack[i + 1];
                    paths.back().add(Line{pos});
                }
            }
            stack.clear();
        }
        else if (sarg == "rrcurveto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() || stack.size() % 6)
                throw std::invalid_argument("incorrect number of arguments for rrcurveto");

            // Bezier curves
            for (int i = 0; i < int(stack.size()); i += 6) {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2], pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4], pos.y += stack[i + 5];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }
            stack.clear();
        }
        else if (sarg == "hhcurveto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() || stack.size() % 4 > 1)
                throw std::invalid_argument("invalid number of arguments for hhcurveto");

            if (stack.size() % 4 == 1) {
                pos.y += stack[0];
                stack.pop_front();
            }

            for (int i = 0; i < int(stack.size()); i += 4) {
                pos.x += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.x += stack[i + 3];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }
            stack.clear();
        }
        else if (sarg == "hvcurveto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() ||
                (stack.size() % 8 != 0 && stack.size() % 8 != 1 &&
                stack.size() % 8 != 4 && stack.size() % 8 != 5))
                throw std::invalid_argument("invalid number of arguments for hvcurveto");

            // alternate start horizontal, end vertical and
            // start vertical, end horizontal
            for (int i = 0; i < int(stack.size()); i += 8) {
                pos.x += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.y += stack[i + 3];
                if (i + 5 == int(stack.size()))
                    pos.x += stack[i + 4];
                paths.back().add(CubicBezier{ct1, ct2, pos});

                if (int(stack.size()) < i + 8) break;

                pos.y += stack[i + 4];
                ct1 = pos;
                pos.x += stack[i + 5], pos.y += stack[i + 6];
                ct2 = pos;
                pos.x += stack[i + 7];
                if (i + 9 == int(stack.size()))
                    pos.y += stack[i + 8];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }

            stack.clear();
        }
        else if (sarg == "rcurveline") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() % 6 != 2)
                throw std::invalid_argument("invalid number of arguments for rcurveline");

            // Bezier curves
            for (int i = 0; i < int(stack.size()) - 2; i += 6) {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2], pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4], pos.y += stack[i + 5];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }

            // followed by a line
            pos.x += stack[stack.size() - 2];
            pos.y += stack[stack.size() - 1];
            paths.back().add(Line{pos});

            stack.clear();
        }
        else if (sarg == "rlinecurve") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() < 8 || stack.size() % 2)
                throw std::invalid_argument("invalid number of arguments for rlinecurve");

            // lines
            for (int i = 0; i < int(stack.size()) - 6; i += 2) {
                pos.x += stack[i], pos.y += stack[i + 1];
                paths.back().add(Line{pos});
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
            paths.back().add(CubicBezier{ct1, ct2, pos});

            stack.clear();
        }
        else if (sarg == "vhcurveto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() ||
                (stack.size() % 8 != 0 && stack.size() % 8 != 1 &&
                stack.size() % 8 != 4 && stack.size() % 8 != 5))
                throw std::invalid_argument("invalid number of arguments for vhcurveto");

            // alternate start vertical, end horizontal and
            // start horizontal, end vertical
            for (int i = 0; i < int(stack.size()); i += 8) {
                pos.y += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.x += stack[i + 3];
                if (i + 5 == int(stack.size()))
                    pos.y += stack[i + 4];
                paths.back().add(CubicBezier{ct1, ct2, pos});

                if (int(stack.size()) < i + 8) break;

                pos.x += stack[i + 4];
                ct1 = pos;
                pos.x += stack[i + 5], pos.y += stack[i + 6];
                ct2 = pos;
                pos.y += stack[i + 7];
                if (i + 9 == int(stack.size()))
                    pos.x += stack[i + 8];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }

            stack.clear();
        }
        else if (sarg == "vvcurveto") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.empty() ||
                (stack.size() % 4 != 0 && stack.size() % 4 != 1))
                throw std::invalid_argument("invalid number of arguments for vvcurveto");

            int i = 0;
            if (stack.size() % 4 == 1) {
                pos.x += stack[0];
                ++i;
            }

            for (; i < int(stack.size()); i += 4) {
                pos.y += stack[i];
                Point ct1 = pos;
                pos.x += stack[i + 1], pos.y += stack[i + 2];
                Point ct2 = pos;
                pos.y += stack[i + 3];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }

            stack.clear();
        }
        else if (sarg == "flex") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() != 13)
                std::invalid_argument("invalid number of arguments for flex");

            for (int i = 0; i < int(stack.size()) - 1; i += 6) {
                pos.x += stack[i], pos.y += stack[i + 1];
                Point ct1 = pos;
                pos.x += stack[i + 2]; pos.y += stack[i + 3];
                Point ct2 = pos;
                pos.x += stack[i + 4]; pos.y += stack[i + 5];
                paths.back().add(CubicBezier{ct1, ct2, pos});
            }

            // TODO: take care of "flex depth"

            stack.clear();
        }
        else if (sarg == "hflex") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() != 7)
                std::invalid_argument("invalid number of arguments for hflex");

            Point orig = pos;

            pos.x += stack[0];
            Point ct1 = pos;
            pos.x += stack[1], pos.y += stack[2];
            Point ct2 = pos;
            pos.x += stack[3];
            paths.back().add(CubicBezier{ct1, ct2, pos});

            pos.x += stack[4];
            ct1 = pos;
            pos.x += stack[5], pos.y = orig.y;
            ct2 = pos;
            pos.x += stack[6];
            paths.back().add(CubicBezier{ct1, ct2, pos});

            // TODO: fd = 50

            stack.clear();
        }
        else if (sarg == "hflex1") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() != 9)
                std::invalid_argument("invalid number of arguments for hflex1");

            Point orig = pos;

            pos.x += stack[0], pos.y += stack[1];
            Point ct1 = pos;
            pos.x += stack[2]; pos.y += stack[3];
            Point ct2 = pos;
            pos.x += stack[4];
            paths.back().add(CubicBezier{ct1, ct2, pos});

            pos.x += stack[5];
            ct1 = pos;
            pos.x += stack[6], pos.y = orig.y;
            ct2 = pos;
            paths.back().add(CubicBezier{ct1, ct2, pos});

            // TODO: fd = 50

            stack.clear();
        }
        else if (sarg == "flex1") {
            if (paths.empty()) paths.push_back(Path(pos));

            if (stack.size() != 11)
                throw std::invalid_argument("invalid number of arguments for flex1");

            Point orig = pos;

            pos.x += stack[0], pos.y += stack[1];
            Point ct1 = pos;
            pos.x += stack[2], pos.y += stack[3];
            Point ct2 = pos;
            pos.x += stack[4], pos.y += stack[5];
            paths.back().add(CubicBezier{ct1, ct2, pos});

            pos.x += stack[6], pos.y += stack[7];
            ct1 = pos;
            pos.x += stack[8], pos.y += stack[9];
            ct2 = pos;

            if (std::abs(pos.x - orig.x) > std::abs(pos.y - orig.y))
                pos.x += stack[10], pos.y = orig.y;
            else
                pos.x = orig.x, pos.y += stack[10];
            paths.back().add(CubicBezier{ct1, ct2, pos});

            // TODO: fd = 50

            stack.clear();
        }
        else if (sarg == "endchar") {
            if (!stack.empty())
                throw std::invalid_argument("stack not empty when finishing path");
            stack.clear();
        }
        else if (sarg == "hstem") {
            // TODO: implement hints
            stack.clear();
        }
        else if (sarg == "vstem") {
            stack.clear();
        }
        else if (sarg == "hstemhm") {
            stack.clear();
        }
        else if (sarg == "vstemhm") {
            stack.clear();
        }
        else if (sarg == "hintmask") {
            std::string mask_str;
            if (!(iss >> mask_str))
                throw std::invalid_argument("no mask after hintmask");
            std::bitset<8> mask(mask_str);
            stack.clear();
        }
        else if (sarg == "cntrmask") {
            std::string mask_str;
            if (!(iss >> mask_str))
                throw std::invalid_argument("no mask after cntrmask");
            std::bitset<8> mask(mask_str);
            stack.clear();
        }
        else if (sarg == "callsubr" || sarg == "callgsubr") {

            if (stack.empty()) {
                std::ostringstream os;
                os << "invalid number of arguments for " << sarg;
                throw std::invalid_argument(os.str());
            }

            auto const& subr = subroutines.at(sarg == "callgsubr"? -1 : fd_index);

            int bias;
            if (subr.size() < 1240) bias = 107;
            else if (subr.size() < 33900) bias = 1131;
            else bias = 32768;

            int subr_idx = stack.back() + bias;
            stack.pop_back();

            call_subroutine(subr.at(subr_idx).subr, subroutines, fd_index, state);
        }
        else if (sarg == "return") {
            break;
        }
        else {
            std::ostringstream os;
            os << "Unimplemented: " << std::quoted(sarg);
            throw std::runtime_error(os.str());
        }
        command_index++;
    }
}

static std::vector<Path> parse_charstring(
    std::string const& charstring,
    subroutine_set const& subroutines,
    int fd_index)
{
    parse_state state;
    call_subroutine(charstring, subroutines, fd_index, state);
    return state.paths;
}

Glyph Glyph::from_charstring(
    std::string const& chname,
    std::string const& charstring,
    subroutine_set const& subroutines,
    int fd_index)
{
    Glyph g;
    g.chname = chname;

    try {
        g.paths = parse_charstring(charstring, subroutines, fd_index);
    }
    catch (std::exception const& e) {
        std::cerr << "Exception occurred when parsing charstring for " << chname << ": " << e.what() << std::endl;
        std::cerr << "Set to an empty glyph." << std::endl;
        g.paths = {};
    }
    return g;
}

}
