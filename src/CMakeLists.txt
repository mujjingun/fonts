qt5_add_resources(SOURCE_FILES resources.qrc)
add_executable(${PROJECT_NAME} main.cpp ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} Qt5::Qml Qt5::Gui)
