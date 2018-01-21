
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UTILS_SOURCE_FILES
    glyph.cpp
    subroutines.cpp
    parser.cpp
    )
add_library(${PROJECT_NAME}utils STATIC ${UTILS_SOURCE_FILES})
