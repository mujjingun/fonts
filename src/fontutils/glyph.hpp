#ifndef FONTUTILS_GLYPH_HPP
#define FONTUTILS_GLYPH_HPP

#include <vector>

namespace fontutils
{

struct Point
{
    int x, y;

    bool operator==(Point rhs) const noexcept;
};

struct Path
{
    Point start;
    struct Segment
    {
        Point ct1, ct2, p;

        bool operator==(Segment const& rhs) const noexcept;
    };
    std::vector<Segment> segments;

    Path(Point start);
    void lineto(Point p);
    void curveto(Point ct1, Point ct2, Point p);

    bool operator==(Path const& rhs) const noexcept;
};

struct Glyph
{
    std::vector<Path> paths;

    bool operator==(Glyph const& rhs) const noexcept;
};
}
#endif
