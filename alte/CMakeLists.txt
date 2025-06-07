cmake_minimum_required(VERSION 3.16)
project(Alte VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
set(QT_VERSION_MAJOR 6)

if(NOT Qt6_FOUND)
    message(STATUS "Qt6 not found, falling back to Qt5.")
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "Using Qt version: ${QT_VERSION_MAJOR}")

set(ALTE_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ShortcutManager.cpp
    src/ShortcutSettingsDialog.cpp
    src/ScratchpadDialog.cpp
)

set(ALTE_HEADERS
    src/MainWindow.h
    src/ShortcutManager.h
    src/ShortcutSettingsDialog.h
    src/ScratchpadDialog.h
)

add_executable(Alte ${ALTE_SOURCES} ${ALTE_HEADERS})

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(Alte PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
else()
    target_link_libraries(Alte PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "alte")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Alte Project")
set(CPACK_PACKAGE_DESCRIPTION "Alte Text Editor - A lightweight, fast, and modern text editor.")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alte Maintainers <contact@alte.example.org>")
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/${PROJECT_NAME}")

install(TARGETS Alte DESTINATION bin)

set(DESKTOP_FILE_CONTENT "[Desktop Entry]\nVersion=1.0\nName=Alte Text Editor\nComment=Lightweight and Fast Text Editor\nExec=/opt/Alte/bin/Alte\nIcon=text-editor\nTerminal=false\nType=Application\nCategories=Utility;TextEditor;Development;\n")

set(DESKTOP_FILE_NAME "alte.desktop")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}" "${DESKTOP_FILE_CONTENT}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}" DESTINATION share/applications)


include(CPack)
