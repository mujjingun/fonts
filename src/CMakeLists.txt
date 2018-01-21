
add_subdirectory(fontutils)

qt5_add_resources(SOURCE_FILES resources.qrc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
    ${SOURCE_FILES}
    main.cpp
    jamoview.cpp
    jamoviewrenderer.cpp
    jamomodel.cpp
    menuhandler.cpp
    )
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}utils
    Qt5::Qml
    Qt5::Gui
    Qt5::Quick
    Qt5::Widgets
    ${PUGIXML_LIBRARY})
