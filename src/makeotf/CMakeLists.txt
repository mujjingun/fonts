if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MAKEOTFLIB_SOURCE_FILES
    source/cffread/cffread.c
    source/hotconv/BASE.c
    source/hotconv/CFF_.c
    source/hotconv/GDEF.c
    source/hotconv/GPOS.c
    source/pstoken/pstoken.c
    source/typecomp/charset.c
    source/typecomp/cs.c
    source/typecomp/dict.c
    source/typecomp/encoding.c
    source/typecomp/fdselect.c
    source/typecomp/parse.c
    source/typecomp/recode.c
    source/typecomp/sindex.c
    source/typecomp/subr.c
    source/typecomp/tc.c
    )
add_library(${PROJECT_NAME}makeotflib STATIC ${MAKEOTFLIB_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}makeotflib PUBLIC
    api
    resource
    )
