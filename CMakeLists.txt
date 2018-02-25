cmake_minimum_required(VERSION 3.10)
project(hangulfontmaker)

# Options
option(BUILD_TESTING "compile with tests" ON)

# Requirements
find_package(Qt5 REQUIRED COMPONENTS Gui Qml Quick Widgets)

message("Qt Version: ${Qt5Core_VERSION} found.")
if (Qt5Core_VERSION VERSION_LESS "5.10.0")
    message(WARNING "Needs Qt >= 5.10.0")
endif()

# Policy
cmake_policy(SET CMP0071 OLD)

# Clang tidy
if(0)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy" "clang-tidy-5.0"
      DOC "Path to clang-tidy executable"
      )
    if(NOT CLANG_TIDY_EXE)
      message(WARNING "clang-tidy not found.")
    else()
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*")
    endif()
endif()

# Source
add_subdirectory(src)

# Tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
