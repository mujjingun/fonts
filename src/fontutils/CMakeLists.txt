if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UTILS_SOURCE_FILES
    glyph.cpp
    subroutines.cpp
    parser.cpp
    otfwriter.cpp
    )
add_library(${PROJECT_NAME}utils STATIC ${UTILS_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}utils PRIVATE
    ${PROJECT_NAME}makeotflib
    )
target_include_directories(${PROJECT_NAME}utils PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    )
