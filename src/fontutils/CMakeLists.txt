set(UTILS_SOURCE_FILES
    buffer.cpp
    stdstr.cpp
    cffutils.cpp
    csparser.cpp
    glyph.cpp
    otfparser.cpp

    tables/otftable.cpp
    tables/font.cpp
    tables/generictable.cpp
    tables/headtable.cpp
    tables/hmtxtable.cpp
    tables/maxptable.cpp
    tables/hheatable.cpp
    tables/cmaptable.cpp
    tables/cmapformat4.cpp
    tables/cmapformat12.cpp
    tables/cmapformat14.cpp
    tables/cmapsubtable.cpp
    tables/nametable.cpp
    tables/os2table.cpp
    tables/posttable.cpp
    tables/cfftable.cpp
    )
add_library(${PROJECT_NAME}utils STATIC ${UTILS_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}utils PRIVATE)
target_include_directories(${PROJECT_NAME}utils PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    )
set_target_properties(${PROJECT_NAME}utils PROPERTIES
    COMPILE_FLAGS "-Wall -Wextra -pedantic"
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
