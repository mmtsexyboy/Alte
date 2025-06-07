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
  # Create desktop shortcut for the original user
  if [ -n "\\$SUDO_USER" ] && [ "\\$SUDO_USER" != "root" ]; then
      USER_HOME=\$(getent passwd "\\$SUDO_USER" | cut -d: -f6)
      if [ -n "\\$USER_HOME" ] && [ -d "\\$USER_HOME" ]; then
          # Try to get Desktop path using xdg-user-dir as the user
          DESKTOP_DIR=\$(sudo -u "\\$SUDO_USER" xdg-user-dir DESKTOP 2>/dev/null || true)

          # Fallback if xdg-user-dir failed, returned empty, or non-directory
          if [ -z "\\$DESKTOP_DIR" ] || ! sudo -u "\\$SUDO_USER" test -d "\\$DESKTOP_DIR"; then
              DESKTOP_DIR="\\$USER_HOME/Desktop"
          fi

          # Check if Desktop directory exists (as user)
          if sudo -u "\\$SUDO_USER" test -d "\\$DESKTOP_DIR"; then
              DESKTOP_FILE_PATH="/usr/share/applications/alte.desktop"
              SHORTCUT_NAME="Alte Text Editor.desktop"

              if [ -f "\\$DESKTOP_FILE_PATH" ]; then
                  echo "Attempting to create desktop shortcut in '\\$DESKTOP_DIR' for user '\\$SUDO_USER'..."
                  if sudo -u "\\$SUDO_USER" ln -sf "\\$DESKTOP_FILE_PATH" "\\$DESKTOP_DIR/\\$SHORTCUT_NAME"; then
                      echo "Desktop shortcut created successfully: '\\$DESKTOP_DIR/\\$SHORTCUT_NAME'"
                      # Attempt to make the shortcut executable (often needed for .desktop files to be trusted)
                      sudo -u "\\$SUDO_USER" chmod +x "\\$DESKTOP_DIR/\\$SHORTCUT_NAME" || true
                  else
                      echo "Warning: Failed to create desktop shortcut for user '\\$SUDO_USER'."
                  fi
              else
                  echo "Warning: Could not create desktop shortcut. Main .desktop file ('\\$DESKTOP_FILE_PATH') not found."
              fi
          else
              echo "Warning: User's Desktop directory ('\\$DESKTOP_DIR') not found or not accessible. No shortcut created."
          fi
      else
          echo "Warning: Could not determine home directory for user '\\$SUDO_USER' or it does not exist. No desktop shortcut created."
      fi
  else
      echo "Info: Running as root or SUDO_USER not set/is root. Skipping user-specific desktop shortcut."
  fi
else
  echo "Dependency resolution failed. You may need to resolve dependencies manually."
  exit 1
fi

exit 0
