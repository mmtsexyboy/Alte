#!/bin/bash

# 1. Check for root privileges
if [ "$EUID" -ne 0 ]; then
  echo "Please run this script with sudo: sudo ./install.sh"
  exit 1
fi

# 2. Check for required dependencies
echo "Checking for required dependencies..."
REQUIRED_CMDS=("cmake" "g++" "make" "cpack")
REQUIRED_PKGS=("qtbase5-dev" "libqt5svg5-dev") # Assuming Qt5 for now

all_deps_met=true
missing_deps=()

# Check for required commands
for cmd in "${REQUIRED_CMDS[@]}"; do
  if ! command -v "$cmd" &> /dev/null; then
    all_deps_met=false
    missing_deps+=("$cmd (command)")
  fi
done

# Check for required packages
for pkg in "${REQUIRED_PKGS[@]}"; do
  if ! dpkg -s "$pkg" &> /dev/null; then
    all_deps_met=false
    missing_deps+=("$pkg (package)")
  fi
done

if [ "$all_deps_met" = false ]; then
  echo "Error: The following dependencies are required to build and install Alte but were not found:"
  for dep in "${missing_deps[@]}"; do
    echo "  - $dep"
  done
  echo "Please install them and try again. For example, on Debian/Ubuntu:"
  echo "sudo apt-get install cmake g++ make cpack qtbase5-dev libqt5svg5-dev"
  exit 1
else
  echo "All required dependencies are present."
fi

# 3. Build and package the project
echo "Starting build process..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake
echo "Configuring project with CMake..."
if ! cmake ..; then
  echo "Error: CMake configuration failed."
  cd ..
  exit 1
fi
echo "CMake configuration successful."

# Run make
echo "Compiling project with make..."
if ! make; then # Alternatively, use: cmake --build .
  echo "Error: Compilation failed."
  cd ..
  exit 1
fi
echo "Compilation successful."

# Run CPack
echo "Creating .deb package with CPack..."
if ! cpack -G DEB; then
  echo "Error: CPack failed to create .deb package."
  cd ..
  exit 1
fi
echo "CPack finished successfully."

# Dynamically find the .deb package name
DEB_PACKAGE=$(ls alte-*-Linux.deb 2>/dev/null | head -n 1)

if [ -z "$DEB_PACKAGE" ] || [ ! -f "$DEB_PACKAGE" ]; then
  echo "Error: Could not find the .deb package after build."
  echo "Expected a file matching 'alte-*-Linux.deb' in the 'build' directory."
  cd ..
  exit 1
fi
echo "Found package: $DEB_PACKAGE"

# Return to the project root directory (optional, consider if subsequent commands need it)
# For now, installation happens from within 'build' directory.
# cd ..

# 3. Check if the .deb package exists (this check is now after the build process)
# The dynamic DEB_PACKAGE variable handles this.

# 4. Confirm installation
echo "Ready to install $DEB_PACKAGE."
read -p "Proceed with installation? (y/N) " confirm
if [[ "$confirm" != [yY] ]]; then
  echo "Installation cancelled."
  exit 0
fi

# 5. Install the package
echo "Installing $DEB_PACKAGE from $(pwd)..." # Show current directory for clarity
if dpkg -i "$DEB_PACKAGE"; then
  echo "$DEB_PACKAGE installed or updated."
else
  echo "Error during dpkg installation. Attempting to fix dependencies..."
  # Proceed to dependency fixing even if dpkg reports an error, as it's often due to missing dependencies.
fi
# Note: dpkg -i is run from the 'build' directory where the .deb file is located.

# 6. Fix dependencies
echo "Attempting to fix any missing dependencies..."
# Return to project root before running apt-get if necessary,
# or ensure all paths used by apt-get are absolute or correct relative to 'build'.
# For apt-get commands, current directory usually doesn't matter, but good to be mindful.
# cd .. # Example if returning to root was needed here.
echo "Running apt-get update..."
if apt-get update; then
  echo "apt-get update successful."
else
  echo "Warning: apt-get update failed. Proceeding with dependency fix attempt..."
fi

echo "Running apt-get install -f -y..."
if apt-get install -f -y; then
  echo "Installation complete!"
else
  echo "Dependency resolution failed. You may need to resolve dependencies manually."
  exit 1
fi

exit 0
