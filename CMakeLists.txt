cmake_minimum_required(VERSION 3.16) # Increased for Qt6 compatibility if needed
project(Alte LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) # Changed to C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add AddressSanitizer flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (Prefer Qt6, fallback to Qt5)
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
  # Update CPack dependencies for Qt6. Adjust package names as per distribution.
  # These are examples for Debian/Ubuntu.
  set(CPACK_DEBIAN_QT_DEPS "libqt6core6 (>= 6.2.0), libqt6gui6 (>= 6.2.0), libqt6widgets6 (>= 6.2.0)")
endif()

message(STATUS "Using Qt version: ${QT_VERSION}")

# Add include directory
include_directories(include)

# Add QRC file for resources (like icons)
# This assumes Alte.qrc will be in the resources/ directory
set(RESOURCE_FILES resources/Alte.qrc)

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Add headers that require MOC processing
# AlteRope.h might not need MOC if it's pure C++ logic.
# If splashscreen is a QWidget, it needs MOC.
# AlteThemeManager might need MOC if it's a QObject.
set(MOC_HEADERS
    include/AlteSyntaxHighlighter.h
    include/AlteThemeManager.h
    # include/AlteRope.h # Assuming it's not a QObject
    include/splashscreen.h # Assuming it's a QObject or QWidget
)

# Add executable
add_executable(Alte ${SOURCES} ${MOC_HEADERS} ${RESOURCE_FILES})

# Link Qt libraries
target_link_libraries(Alte PRIVATE ${QT_WIDGETS_LIB}) # Links Core, Gui, and Widgets

# Enable testing
enable_testing()

# --------------------------------------------------------------------------
# CPack configuration for DEB package
# --------------------------------------------------------------------------
install(TARGETS Alte DESTINATION bin)

# Install resource files (syntax definitions and themes)
install(DIRECTORY resources/syntax/ DESTINATION share/alte/resources/syntax)
install(DIRECTORY resources/themes/ DESTINATION share/alte/resources/themes)
# Note: Icon is now part of Alte.qrc, but we can still install it for system integration
install(FILES resources/icons/alte_icon.png DESTINATION share/icons/hicolor/128x128/apps RENAME alte_icon.png)

# Install .desktop file
install(FILES packaging/linux/alte.desktop DESTINATION share/applications/)

# Setup CPack variables
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "alte")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alte - Modern Text Editor")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight, fast, and user-friendly text editor with syntax highlighting, theming, and advanced features. Built with C++20 and Qt.")
set(CPACK_PACKAGE_VENDOR "Alte Project")
set(CPACK_PACKAGE_MAINTAINER "Alte Developer <dev@example.com>") # Replace

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.28), ${CPACK_DEBIAN_QT_DEPS}") # Adjusted libc6 and added Qt deps variable

set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_MAINTAINER})
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/your_username/Alte") # Replace

# Include CPack module to enable packaging
include(CPack)
