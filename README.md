# Alte Text Editor

Alte is a lightweight, fast, and modern text editor designed for performance and a clean user experience. It aims to provide essential features for developers and writers, with a focus on speed, customizability, and beautiful rendering of text.

(Alte : یک اپلیکیشن تکس ادیتور به شدت سبک و به شدت ساده و به شدت کاربردی)

For the detailed vision, philosophy, and future goals of the Alte project, please see [VISION.md](VISION.md).

## Features (Current & Planned)

*   Cross-platform (initially Linux)
*   High performance, capable of handling large files (using a Rope data structure)
*   Syntax highlighting for various programming languages
*   Theming support
*   Plugin architecture (planned)
*   UTF-8 and RTL language support (e.g., Persian, Arabic)
*   C++20 core with Qt6 (fallback to Qt5) for the UI

## Building from Source

### Prerequisites

*   A C++20 compliant compiler (e.g., GCC, Clang)
*   CMake (version 3.16 or higher)
*   Qt (version 6 or 5). Qt6 is preferred.
    *   Required Qt modules: Core, Gui, Widgets

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <repository_url>
    cd alte
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure with CMake:**
    ```bash
    cmake ..
    ```
    (CMake will try to find Qt6 first. If not found, it will look for Qt5.)

4.  **Compile:**
    ```bash
    make
    ```
    (Or your chosen build system's command, e.g., `ninja`)

The executable `Alte` will be created in the `build` directory.

## Installation (Linux)

A DEB package can be created for easier installation on Debian-based Linux distributions:

1.  Follow the build steps above.
2.  From the `build` directory, run:
    ```bash
    cpack -G DEB
    ```
This will generate a `.deb` file (e.g., `alte-0.1.0-Linux.deb`) which can be installed using your system's package manager (e.g., `sudo dpkg -i alte-0.1.0-Linux.deb` followed by `sudo apt-get install -f` if there are dependency issues).

The editor will typically be installed to `/usr/local/bin` or `/opt/` and a `.desktop` file will be added for application menus.

## Contributing

Contributions are welcome! Please refer to the [VISION.md](VISION.md) for the overall direction. (Further contributing guidelines can be added here later).
