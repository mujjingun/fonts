enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find gtest dependency
find_package(GTest REQUIRED)

# test source files
set(TEST_SOURCE_FILES
    main.cpp)

message("Source files for ${PROJECT_NAME} tests"
    "${TEST_SOURCE_FILES}")

# add test target
add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}_test
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}_test
    ${PROJECT_NAME}utils
    Threads::Threads
    ${GTEST_BOTH_LIBRARIES})

set_target_properties(${PROJECT_NAME}_test PROPERTIES
    COMPILE_FLAGS "-Wall -Wextra -pedantic -fsanitize=address"
    LINK_FLAGS "-static-libasan -static-libstdc++ -fsanitize=address"
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# ctest
add_test(${PROJECT_NAME}_cmake_test ${PROJECT_NAME}_test)

################## Fuzzer ####################
set(ENV{AFL_USE_ASAN} 1)
add_executable(${PROJECT_NAME}_fuzzer fuzzer.cpp)
target_link_libraries(${PROJECT_NAME}_fuzzer
    ${PROJECT_NAME}utils)

target_include_directories(${PROJECT_NAME}_fuzzer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src)

set_target_properties(${PROJECT_NAME}_fuzzer PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
