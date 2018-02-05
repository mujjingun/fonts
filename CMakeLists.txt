cmake_minimum_required(VERSION 3.7)
project(hangulfontmaker)

option(BUILD_TESTING "compile with tests" ON)

set (CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(Qt5 REQUIRED COMPONENTS Gui Qml Quick Widgets)

message("Qt Version: ${Qt5Core_VERSION} found.")
if (Qt5Core_VERSION VERSION_LESS "5.10.0")
    message(WARNING "Needs Qt >= 5.10.0")
endif()

find_package(PugiXML REQUIRED)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

# Tests
if(BUILD_TESTING)
    enable_testing()

    set(THREADS_PREFER_PTHREAD_FLAG ON)

    # find gtest dependency
    find_package(Threads REQUIRED)
    find_package(GTest REQUIRED)

    # test source files
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

    set(TEST_SOURCE_FILES
        ${TEST_DIR}/main.cpp
        )

    message("Source files for ${PROJECT_NAME} tests"
        "${TEST_SOURCE_FILES}")

    # add test target
    add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_FILES})

    set(BUILD_TARGETS ${TARGETS}
        "${PROJECT_NAME}_test")

    target_include_directories(${PROJECT_NAME}_test
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PUGIXML_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME}_test
        ${PROJECT_NAME}utils
        Threads::Threads
        ${PUGIXML_LIBRARY}
        ${GTEST_BOTH_LIBRARIES})

    set(TEST_BUILD_FLAGS
        "${CMAKE_CXX_FLAGS} -std=c++14 -g -Wall -Wextra")

    message("Test C++ flags = ${TEST_BUILD_FLAGS}")

    set_target_properties(${PROJECT_NAME}_test
        PROPERTIES COMPILE_FLAGS "${TEST_BUILD_FLAGS}")

    # ctest
    add_test(${PROJECT_NAME}_cmake_test ${PROJECT_NAME}_test)

endif()

add_subdirectory(makeotf_debug)
