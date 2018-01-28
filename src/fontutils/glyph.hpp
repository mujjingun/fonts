#ifndef GLYPH_HPP
#define GLYPH_HPP

#include <vector>
#include <memory>

#include "subroutines.hpp"

namespace fontutils {

struct Point
{
    int x, y;
};

struct Line
{
    Point p;
};

struct CubicBezier
{
    Point ct1, ct2, p;
};

class Path
{
public:
    Path() = default;
    Path(Point start);
    void add(Line l);
    void add(CubicBezier b);

    Point start() const;
    std::vector<CubicBezier> segments() const;

private:
    Point start_;
    std::vector<CubicBezier> segments_;
};

struct Glyph
{
    std::string chname;
    std::vector<Path> paths;
    int width;

    Glyph();

    static Glyph from_charstring(
        std::string const& chname,
        std::string const& charstring,
        subroutine_set const& subroutines,
        int fd_index);
};

}

#endif
