if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MAKEOTFLIB_SOURCE_FILES
    source/hotconv/anon.c
    source/hotconv/BASE.c
    source/hotconv/CFF_.c
    source/hotconv/cmap.c
    source/hotconv/fvar.c
    source/hotconv/GDEF.c
    source/hotconv/GPOS.c
    source/hotconv/GSUB.c
    source/hotconv/head.c
    source/hotconv/hhea.c
    source/hotconv/hmtx.c
    source/hotconv/hot.c
    source/hotconv/map.c
    source/hotconv/maxp.c
    source/hotconv/MMFX.c
    source/hotconv/MMSD.c
    source/hotconv/name.c
    source/hotconv/OS_2.c
    source/hotconv/otl.c
    source/hotconv/post.c
    source/hotconv/sfnt.c
    source/hotconv/vhea.c
    source/hotconv/vmtx.c
    source/hotconv/VORG.c
    source/hotconv/featerr.c
    source/hotconv/featgram.c
    source/hotconv/featscan.c

    source/dynarr/dynarr.c

    source/ctutil/ctutil.c

    source/cffread/cffread.c

    source/typecomp/charset.c
    source/typecomp/cs.c
    source/typecomp/dict.c
    source/typecomp/encoding.c
    source/typecomp/fdselect.c
    source/typecomp/parse.c
    source/typecomp/recode.c
    source/typecomp/sindex.c
    source/typecomp/subr.c
    source/typecomp/t13.c
    source/typecomp/tc.c

    source/pstoken/pstoken.c
    )
add_library(${PROJECT_NAME}makeotflib STATIC ${MAKEOTFLIB_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}makeotflib PRIVATE
    api
    resource
    )
target_compile_definitions(${PROJECT_NAME}makeotflib PRIVATE
    HOT_FEAT_SUPPORT=1

    TC_EURO_SUPPORT=1
    TC_SUBR_SUPPORT=1
    TC_HINT_CHECK=1
    TC_T13_SUPPORT=0
    TC_DEBUG=1

    CFF_T13_SUPPORT=0
    CFF_DEBUG=1
    )
target_link_libraries(${PROJECT_NAME}makeotflib PRIVATE m)
