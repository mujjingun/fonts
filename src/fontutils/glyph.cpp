#include "glyph.hpp"

#include <QDebug>

#include <pugixml.hpp>
#include <exception>
#include <iostream>
#include <cstdio>
#include <stack>
#include <sstream>

#include "subroutines.hpp"

namespace fontutils {

Path::Path(Point start)
    : start_(start), segments_{}
{ }

void Path::add(Line l)
{
    segments_.push_back(l);
}

std::vector<Segment> Path::segments()
{
    return segments_;
}

fontutils::Point Path::start()
{
    return start_;
}


Glyph Glyph::from_charstring(std::string chname, std::string charstring)
{
    Glyph g{chname, {}};
    std::deque<int> stack;
    std::istringstream iss(charstring);
    Point pos{0, 0};
    do {
        std::string sarg;
        if (iss >> sarg) {
            int iarg;
            // if sarg is an int
            if (sscanf(sarg.c_str(), "%d", &iarg) == 1) {
                stack.push_back(iarg);
            }
            else { // sarg is a command
                if (sarg == "rmoveto") {
                    int dx1 = stack[0], dy1 = stack[1];
                    stack.clear();
                    pos.x += dx1;
                    pos.y += dy1;
                    g.paths.push_back(Path(pos));
                }
                else if (sarg == "hmoveto") {
                    int dx1 = stack[0];
                    stack.clear();
                    pos.x += dx1;
                    g.paths.push_back(Path(pos));
                }
                else if (sarg == "vmoveto") {
                    int dy1 = stack[1];
                    stack.clear();
                    pos.y += dy1;
                    g.paths.push_back(Path(pos));
                }
                else if (sarg == "rlineto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    for (auto i = 0u; i < stack.size(); i += 2) {
                        Point p{pos.x + stack[i], pos.y + stack[i + 1]};
                        g.paths.back().add(Line{p});
                    }
                    stack.clear();
                }
                else if (sarg == "hlineto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));

                    // alternating horizontal/vertical lines
                    for (auto i = 0u; i < stack.size(); i += 2) {
                        // horizontal line
                        Point ph{pos.x + stack[i], pos.y};
                        g.paths.back().add(Line{ph});

                        // vertical line
                        if (i + 1 < stack.size()) {
                            Point pv{pos.x, pos.y + stack[i + 1]};
                            g.paths.back().add(Line{pv});
                        }
                    }
                    stack.clear();
                }
                else if (sarg == "vlineto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));

                    // alternating vertical/horizontal lines
                    for (auto i = 0u; i < stack.size(); i += 2) {
                        // vertical line
                        Point pv{pos.x, pos.y + stack[i]};
                        g.paths.back().add(Line{pv});

                        // horizontal line
                        if (i + 1 < stack.size()) {
                            Point ph{pos.x  + stack[i + 1], pos.y};
                            g.paths.back().add(Line{ph});
                        }
                    }
                    stack.clear();
                }
                else if (sarg == "rrcurveto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "hhcurveto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "hvcurveto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "rcurveline") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "rlinecurve") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "vhcurveto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "vvcurveto") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "flex") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "hflex") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "hflex1") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "flex1") {
                    if (g.paths.empty()) g.paths.push_back(Path(pos));
                    stack.clear();
                }
                else if (sarg == "endchar") {
                    stack.clear();
                }
                if (sarg == "hstem") {
                    stack.clear();
                }
                else if (sarg == "vstem") {
                    stack.clear();
                }
                else {
                    //std::cout << "Unimplemented: " << sarg << std::endl;
                }
            }
        }
    } while (!iss.eof());
    return g;
}

}
