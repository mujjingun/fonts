#include "glyph.hpp"

namespace fontutils
{

Path::Path(Point start)
    : start(start)
    , segments{}
{
}

void Path::lineto(Point p)
{
    segments.push_back({ p, p, p });
}

void Path::curveto(Point ct1, Point ct2, Point p)
{
    segments.push_back({ ct1, ct2, p });
}
}
