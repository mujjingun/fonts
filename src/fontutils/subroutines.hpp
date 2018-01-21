#ifndef SUBROUTINES_HPP
#define SUBROUTINES_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace fontutils {

struct Subroutine {
    std::string subr;

    Subroutine() = default;

    explicit Subroutine (std::string charstring);
};

using subroutine_set = std::unordered_map<int, std::vector<Subroutine>>;

}

#endif
