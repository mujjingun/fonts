#ifndef FONTUTILS_GLYPH_HPP
#define FONTUTILS_GLYPH_HPP

#include <vector>

namespace fontutils
{

struct Point
{
    int x, y;
};

struct Path
{
    Point start;
    struct Segment
    {
        Point ct1, ct2, p;
    };
    std::vector<Segment> segments;

    Path(Point start);
    void lineto(Point p);
    void curveto(Point ct1, Point ct2, Point p);
};

struct Glyph
{
    std::vector<Path> paths;
};
}
#endif
