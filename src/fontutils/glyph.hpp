#ifndef GLYPH_HPP
#define GLYPH_HPP

#include <vector>
#include <memory>

namespace fontutils {

struct Point
{
    int x, y;
};

struct Line
{
    Point p;
};

class Segment
{
public:
    template<typename T>
    Segment(T x) : self(std::make_shared<model<T>>(std::move(x)))
    { }

    template<typename T>
    T get() const {
        return static_cast<model<T> const*>(self.get())->data;
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

    static Glyph from_charstring(std::string chname, std::string charstring);
};

}

#endif
