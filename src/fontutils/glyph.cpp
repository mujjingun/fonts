#include "glyph.hpp"

namespace geul
{

bool Point::operator==(Point rhs) const noexcept
{
    return x == rhs.x && y == rhs.y;
}

Path::Path(Point start)
    : start(start)
    , segments{}
{}

void Path::lineto(Point p)
{
    segments.push_back({ p, p, p });
}

void Path::curveto(Point ct1, Point ct2, Point p)
{
    segments.push_back({ ct1, ct2, p });
}

bool Glyph::operator==(Glyph const& rhs) const noexcept
{
    return paths == rhs.paths;
}

bool Path::operator==(Path const& rhs) const noexcept
{
    return start == rhs.start && segments == rhs.segments;
}

bool Path::Segment::operator==(Segment const& rhs) const noexcept
{
    return ct1 == rhs.ct1 && ct2 == rhs.ct2 && p == rhs.p;
}
}
