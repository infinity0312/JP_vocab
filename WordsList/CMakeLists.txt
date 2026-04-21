cmake_minimum_required(VERSION 3.16)

project(WordsList VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
find_package(Qt6 REQUIRED COMPONENTS Quick QuickDialogs2)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appWordsList
    main.cpp
)

qt_add_qml_module(appWordsList
    URI WordsList
    QML_FILES
        Main.qml
        Mode1Window.qml
        Mode2Window.qml
        EditorWindow.qml
    SOURCES words.h words.cpp
    SOURCES wordsmodel.h wordsmodel.cpp
)

set_target_properties(appWordsList PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appWordsList
    PRIVATE Qt6::Quick
            Qt6::QuickControls2
            Qt6::QuickDialogs2
)

include(GNUInstallDirs)
install(TARGETS appWordsList
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
