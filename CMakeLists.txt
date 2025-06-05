cmake_minimum_required(VERSION 3.10)
project(Alte LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 components
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Add include directory
include_directories(include)

# Add source files
file(GLOB SOURCES "src/*.cpp")
# Add headers that require MOC processing explicitly if AUTOMOC has trouble
set(MOC_HEADERS include/AlteSyntaxHighlighter.h)

# Add executable
add_executable(Alte ${SOURCES} ${MOC_HEADERS})

# Link Qt5 libraries
target_link_libraries(Alte PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

# Enable testing
enable_testing()

# --------------------------------------------------------------------------
# CPack configuration for DEB package
# --------------------------------------------------------------------------
install(TARGETS Alte DESTINATION bin)

# Install resource files (syntax definitions and themes)
# This installs the whole resources directory. Adjust if more granular control is needed.
install(DIRECTORY resources/ DESTINATION share/alte/resources)

# Install icon
install(FILES resources/icons/alte_icon.png DESTINATION share/icons/hicolor/128x128/apps RENAME alte_icon.png)
# Add other icon sizes if available e.g. share/icons/hicolor/64x64/apps etc.
# For SVG: install(FILES resources/icons/alte_icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME alte_icon.svg)

# Install .desktop file
install(FILES packaging/linux/alte.desktop DESTINATION share/applications/) # Assuming alte.desktop is in packaging/linux/

# Setup CPack variables
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "alte")
set(CPACK_PACKAGE_VERSION "0.1.0") # Consider making this dynamic from project version
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alte - Lightweight and Fast Text Editor")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight, fast, and user-friendly text editor with syntax highlighting and theming support.")
set(CPACK_PACKAGE_VENDOR "Alte Project") # Or your name
set(CPACK_PACKAGE_MAINTAINER "Alte Developer <dev@example.com>") # Replace with actual maintainer

# Define dependencies - This is crucial!
# Base dependencies for a Qt5 application. May need more depending on specific Qt modules used.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17), libqt5core5t64 (>= 5.15.3), libqt5gui5 (>= 5.15.3), libqt5widgets5 (>= 5.15.3)")

set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_MAINTAINER})
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/your_username/Alte") # Replace with actual URL if any

# Include CPack module to enable packaging
include(CPack)
