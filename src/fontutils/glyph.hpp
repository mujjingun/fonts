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

class Segment
{
public:
    template<typename T>
    Segment(T x) : self(std::make_shared<model<T>>(std::move(x)))
    { }

    template<typename T>
    T const *get() const {
        model<T> const* p;
        if ((p = dynamic_cast<model<T> const*>(self.get())))
            return &p->data;
        else return nullptr;
    }

private:
    struct concept {
        virtual ~concept() = default;
    };

    template<typename T>
    struct model final : concept {
        model(T x) : data(std::move(x)) {}
        T data;
    };

    std::shared_ptr<const concept> self;
};

class Path
{
public:
    Path() = default;
    Path(Point start);
    void add(Line l);
    void add(CubicBezier b);

    Point start() const;
    std::vector<Segment> segments() const;

private:
    Point start_;
    std::vector<Segment> segments_;
};

struct Glyph
{
    std::string chname;
    std::vector<Path> paths;

    int width;

    static Glyph from_charstring(
        std::string const& chname,
        std::string const& charstring,
        subroutine_set const& subroutines,
        int fd_index);
};

}

#endif
