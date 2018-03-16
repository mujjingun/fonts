#include "fontutils/otfparser.hpp"

int main(int argc, char **argv) {
    if (argc == 2) {
        geul::Font font;
        try {
            font = geul::parse_otf(argv[1]);
        }
        catch (std::exception const& e)
        {
            return 0;
        }
        geul::write_otf(font, "out.otf");
    }
    return 0;
}
