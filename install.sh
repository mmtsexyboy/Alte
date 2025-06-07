#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]; then
  echo "Please run this script with sudo: sudo ./install.sh"
  exit 1
fi

DEB_PACKAGE_NAME="alte-0.1.0-Linux.deb"
BUILD_DIR="build_temp"
DEB_PACKAGE_PATH="$BUILD_DIR/$DEB_PACKAGE_NAME"
REBUILD_FLAG=false

for arg in "$@"
do
    if [ "$arg" == "--rebuild" ]
    then
        REBUILD_FLAG=true
        echo "Rebuild flag detected. Forcing a rebuild of the package."
    fi
done

if [ "$REBUILD_FLAG" = true ] || [ ! -f "$DEB_PACKAGE_PATH" ]; then
  if [ "$REBUILD_FLAG" = true ]; then
    echo "Rebuilding package as requested..."
  else
    echo "$DEB_PACKAGE_PATH not found. Attempting to build it..."
  fi

  ORIGINAL_PWD=$(pwd)

  echo "Creating build directory: $BUILD_DIR"
  mkdir -p "$BUILD_DIR"

  echo "Changing directory to $BUILD_DIR"
  cd "$BUILD_DIR"

  echo "Running CMake..."
  if cmake ..; then
    echo "CMake configuration successful."
  else
    echo "CMake configuration failed. Please check the output above."
    cd "$ORIGINAL_PWD"
    exit 1
  fi

  echo "Building project using make..."
  if make -j$(nproc); then
    echo "Project build successful."
  else
    echo "Project build failed. Please check the output above."
    cd "$ORIGINAL_PWD"
    exit 1
  fi

  echo "Generating Debian package using CPack..."
  if cpack -G DEB; then
    echo "Debian package generation successful."
  else
    echo "Debian package generation failed. Please check the output above."
    cd "$ORIGINAL_PWD"
    exit 1
  fi

  echo "Changing directory back to $ORIGINAL_PWD"
  cd "$ORIGINAL_PWD"

  if [ ! -f "$DEB_PACKAGE_PATH" ]; then
    echo "Error: $DEB_PACKAGE_PATH still not found after build attempt."
    echo "Please check the build logs in the $BUILD_DIR directory."
    exit 1
  else
    echo "Successfully built $DEB_PACKAGE_PATH."
  fi
else
  echo "$DEB_PACKAGE_PATH found."
fi

echo "Found $DEB_PACKAGE_PATH."
read -p "Proceed with installation? (y/N) " confirm
if [[ "$confirm" != [yY] ]]; then
  echo "Installation cancelled."
  exit 0
fi

echo "Installing $DEB_PACKAGE_PATH..."
dpkg -i "$DEB_PACKAGE_PATH" || true

echo "Attempting to fix any missing dependencies..."
echo "Running apt-get update..."
if apt-get update; then
  echo "apt-get update successful."
else
  echo "Warning: apt-get update failed. Proceeding with dependency fix attempt..."
fi

echo "Running apt-get install -f -y..."
if apt-get install -f -y; then
  echo "Installation complete! Alte editor should now be installed."
else
  echo "Dependency resolution failed. You may need to resolve dependencies manually."
  exit 1
fi

exit 0
