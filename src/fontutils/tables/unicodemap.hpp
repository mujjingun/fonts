#ifndef TABLES_UNICODE_MAP_HPP
#define TABLES_UNICODE_MAP_HPP

#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <limits>

namespace fontutils
{

template<typename T>
class Optional
{
    std::unique_ptr<T> p = nullptr;

public:
    Optional() = default;
    Optional(T const& t)
        : p(std::make_unique<T>(t))
    {}
    Optional(T && t)
        : p(std::make_unique<T>(std::move(t)))
    {}
    Optional(Optional const& other)
    {
        *this = other;
    }
    Optional(Optional && other) = default;
    Optional &operator= (Optional const& other)
    {
        if (other.p)
            p.reset(new T(*other.p));
        else
            p.reset();
        return *this;
    }
    Optional &operator= (Optional && other) = default;
    void clear()
    {
        p.reset();
    }
    bool is_set() const
    {
        return p != nullptr;
    }
    T &get()
    {
        return *p;
    }
    T get() const
    {
        return *p;
    }
};

template<typename V>
class UnicodeMap
{
    // Maps range [value, next_value) -> V
    std::map<char32_t, Optional<V>> map = {{0, Optional<V>{}}};

public:

    class iterator
    {
        using internal_type = typename std::map<char32_t, Optional<V>>::iterator;
        iterator(internal_type val, internal_type end)
            : it(val), end(end)
        {}
    public:
        iterator operator++()
        {
            while (it != end && !it->second.is_set()) ++it;
            return it;
        }

        using pair_type = std::pair<std::pair<char32_t, char32_t>, V>;
        pair_type operator*() const
        { return {it->first, std::next(it)->first, it->second.get()}; }

        bool operator== (iterator const& other) const
        { return it == other.it; }

        bool operator!= (iterator const& other) const
        { return it != other.it; }

    private:
        internal_type it;
        const internal_type end;
    };

    V operator[](char32_t key) const
    {
        return std::prev(map.upper_bound(key))->second.get();
    }

    bool is_set(char32_t key) const
    {
        return std::prev(map.upper_bound(key))->second.is_set();
    }

    void set(char32_t key, V const& v)
    {
        auto pp = map.emplace(key, v);
        auto it = pp.first;

        // map has key with same begin value as `key`
        // -> push it by 1 to the right
        if (!pp.second)
        {
            auto tmp = std::move(it->second);
            it = map.erase(it);
            map.emplace(key + 1, std::move(tmp));
            map.emplace(key, v);
        }
        // duplicate left range value to the
        // right side of the inserted key
        else
        {
            map.emplace(key + 1, std::prev(it)->second);
        }
    }

    void set_range(char32_t begin, char32_t end, V const& v)
    {
        if (begin > end)
            throw std::invalid_argument("set_range: begin > end");

        // our range is inside another range
        if (std::prev(map.upper_bound(begin)) == std::prev(map.lower_bound(end)))
        {
            auto pp = map.emplace(begin, v);
            auto it = pp.first;

            // there is a range with the same begin value as `begin`
            // -> push it to the `end`
            if (!pp.second)
            {
                auto tmp = std::move(it->second);
                it = map.erase(it);
                map.emplace(end, std::move(tmp));
                map.emplace(begin, v);
            }
            // duplicate left range value to the
            // right side of our range
            else
            {
                auto tmp = std::move(it->second);
                map.emplace(end, std::move(tmp));
            }
        }
        else
        {
            // push the end-overlapping range to the right
            auto it = std::prev(map.lower_bound(end));
            auto tmp = std::move(it->second);
            map.emplace(end, std::move(tmp));

            // remove all inside ranges
            // there will always be a range beginning at `end`
            map.erase(map.lower_bound(begin), map.lower_bound(end));

            map.emplace(begin, v);
        }
    }
};

}

#endif
