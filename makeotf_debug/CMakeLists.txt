set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g -O0")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MAKEOTFDEBUG_SOURCE_FILES
    fcdb.c
    file.c
    main.c
    cb.c
    cbpriv.c
    sep.c
    )
add_executable(${PROJECT_NAME}makeotfdebug ${MAKEOTFDEBUG_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}makeotfdebug PRIVATE
    ${PROJECT_NAME}makeotflib)
target_include_directories(${PROJECT_NAME}makeotfdebug PRIVATE
    ${PROJECT_SOURCE_DIR}/src/makeotf/api
    ${PROJECT_SOURCE_DIR}/src/makeotf/resource
    )
