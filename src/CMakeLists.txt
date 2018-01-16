qt5_add_resources(SOURCE_FILES resources.qrc)
set(SOURCE_FILES
    ${SOURCE_FILES}
    main.cpp
    jamoview.cpp
    jamoviewrenderer.cpp
    jamomodel.cpp
    )
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} Qt5::Qml Qt5::Gui Qt5::Quick)
