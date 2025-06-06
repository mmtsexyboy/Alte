cmake_minimum_required(VERSION 3.16)
project(AlteEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try to find Qt6 first
find_package(Qt6 COMPONENTS Widgets)

# If Qt6 is not found, try Qt5
if(NOT Qt6_FOUND)
  message(STATUS "Qt6 not found, trying Qt5.")
  find_package(Qt5 COMPONENTS Widgets REQUIRED) # Qt5 is required if Qt6 is not found
  set(QT_VERSION 5)
  # For CPack, ensure Qt5 version variables are available
  set(QT_WIDGETS_VERSION ${Qt5Widgets_VERSION_STRING})
  set(QT_GUI_VERSION ${Qt5Gui_VERSION_STRING})
  set(QT_CORE_VERSION ${Qt5Core_VERSION_STRING})
else()
  message(STATUS "Qt6 found.")
  set(QT_VERSION 6)
  # For CPack, ensure Qt6 version variables are available
  set(QT_WIDGETS_VERSION ${Qt6Widgets_VERSION_STRING})
  set(QT_GUI_VERSION ${Qt6Gui_VERSION_STRING})
  set(QT_CORE_VERSION ${Qt6Core_VERSION_STRING})
endif()

message(STATUS "Using Qt version: ${QT_VERSION}")

add_executable(AlteEditor
    src/main.cpp
    src/syntaxhighlighter.cpp
    src/splashscreen.cpp  # Added splashscreen source
    src/AlteRope.cpp      # Added AlteRope source
    src/AlteThemeManager.cpp # Added ThemeManager source
    ../include/splashscreen.h # Ensure MOC processes this header
    resources.qrc # Added QRC file
)

# Add include directory for project headers (e.g., splashscreen.h)
# splashscreen.h is in 'PROJECT_ROOT/include'
# This CMakeLists.txt is in 'PROJECT_ROOT/AlteEditor/'
# So, the path from here to the include dir is '../include'.
# CMAKE_SOURCE_DIR when running cmake from AlteEditor/build (cmake ..) is /app/AlteEditor.
target_include_directories(AlteEditor PUBLIC ${CMAKE_SOURCE_DIR}/../include)

if(QT_VERSION EQUAL 6)
  qt_standard_project_setup()
  target_link_libraries(AlteEditor PRIVATE Qt6::Widgets)
else()
  target_link_libraries(AlteEditor PRIVATE Qt5::Widgets)
endif()

# Installation for creating a .deb package later
install(TARGETS AlteEditor
    RUNTIME DESTINATION bin)

# --- CPack configuration ---
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alte Text Editor - Lightweight and Featureful")
set(CPACK_PACKAGE_VENDOR "Alte Project")
set(CPACK_PACKAGE_CONTACT "user@example.com") # Placeholder
set(CPACK_PACKAGE_HOMEPAGE_URL "https://example.com/alte") # Placeholder

set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# CPACK_DEBIAN_PACKAGE_ARCHITECTURE will be auto-detected or can be set explicitly

# Determine Qt dependencies based on QT_VERSION
# Note: Qt5Core version might be Qt5Core_VERSION_STRING or specific to libqt5core5a package versioning.
# For debian dependencies, it's often better to use generic package names if specific versions aren't strictly needed
# or use the exact package names from the distribution.
if(QT_VERSION EQUAL 6)
    set(QT_RUNTIME_DEPS "libqt6widgets6 (>= ${QT_WIDGETS_VERSION}), libqt6gui6 (>= ${QT_GUI_VERSION}), libqt6core6 (>= ${QT_CORE_VERSION})")
else()
    # For Qt5, libqt5core5a is common on Debian/Ubuntu. Versioning needs to be handled carefully.
    # Using a slightly more generic dependency if exact version string isn't robust.
    set(QT_RUNTIME_DEPS "libqt5widgets5, libqt5gui5, libqt5core5a")
endif()
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${QT_RUNTIME_DEPS}, libc6 (>= 2.17)") # Basic C library dependency

# Set the generator to DEB
set(CPACK_GENERATOR "DEB")

# Include standard CPack module
include(CPack)
