
add_subdirectory(fontutils)

qt5_add_resources(SOURCE_FILES resources.qrc)

set(SOURCE_FILES
    ${SOURCE_FILES}
    main.cpp
    jamoview.cpp
    jamoviewrenderer.cpp
    jamomodel.cpp
    controller.cpp
    formmodel.cpp
    )
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}utils
    Qt5::Qml
    Qt5::Gui
    Qt5::Quick
    Qt5::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    COMPILE_FLAGS "-Wall -Wextra -pedantic"
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
