cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)
project(Alte LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets)
if(NOT Qt6_FOUND)
  message(STATUS "Qt6 not found, trying Qt5.")
  find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
  set(QT_VERSION 5)
  set(QT_CORE_LIB Qt5::Core)
  set(QT_GUI_LIB Qt5::Gui)
  set(QT_WIDGETS_LIB Qt5::Widgets)
  set(CPACK_DEBIAN_QT_DEPS "libqt5core5t64 (>= 5.15.3), libqt5gui5 (>= 5.15.3), libqt5widgets5 (>= 5.15.3)")
else()
  message(STATUS "Qt6 found.")
  set(QT_VERSION 6)
  set(QT_CORE_LIB Qt6::Core)
  set(QT_GUI_LIB Qt6::Gui)
  set(QT_WIDGETS_LIB Qt6::Widgets)
  set(CPACK_DEBIAN_QT_DEPS "libqt6core6 (>= 6.2.0), libqt6gui6 (>= 6.2.0), libqt6widgets6 (>= 6.2.0)")
endif()

message(STATUS "Using Qt version: ${QT_VERSION}")

include_directories(include)

set(RESOURCE_FILES resources/Alte.qrc)

file(GLOB SOURCES "src/*.cpp")

set(MOC_HEADERS
    include/MainWindow.h
    include/AlteSyntaxHighlighter.h
    include/AlteThemeManager.h
    include/splashscreen.h
)

add_executable(Alte ${SOURCES} ${MOC_HEADERS} ${RESOURCE_FILES})

target_link_libraries(Alte PRIVATE ${QT_WIDGETS_LIB})

enable_testing()

set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources)

add_custom_command(
    TARGET Alte POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR}
    ${DESTINATION_DIR}
    COMMENT "Copying resources to build directory for local execution"
)

install(TARGETS Alte DESTINATION bin)

install(DIRECTORY resources/syntax/ DESTINATION share/alte/resources/syntax)
install(DIRECTORY resources/themes/ DESTINATION share/alte/resources/themes)
install(FILES icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME alte_icon.svg)

install(FILES packaging/linux/alte.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "alte")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alte - Modern Text Editor")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight, fast, and user-friendly text editor with syntax highlighting, theming, and advanced features. Built with C++20 and Qt.")
set(CPACK_PACKAGE_VENDOR "Alte Project")
set(CPACK_PACKAGE_MAINTAINER "Alte Developer <dev@example.com>")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.28), ${CPACK_DEBIAN_QT_DEPS}")

set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_MAINTAINER})
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/your_username/Alte")

include(CPack)
