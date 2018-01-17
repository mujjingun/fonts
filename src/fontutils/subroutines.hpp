#ifndef SUBROUTINES_HPP
#define SUBROUTINES_HPP

#include <string>

namespace fontutils {

    struct Subroutine {
        std::string subr;

        Subroutine() = default;

        explicit Subroutine (std::string charstring2);
    };

}

#endif
