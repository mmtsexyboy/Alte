#!/bin/bash

# 1. Check for root privileges
if [ "$EUID" -ne 0 ]; then
  echo "Please run this script with sudo: sudo ./install.sh"
  exit 1
fi

# 2. Define the .deb package path
DEB_PACKAGE="build_temp/alte-0.1.0-Linux.deb"

# 3. Check if the .deb package exists
if [ ! -f "$DEB_PACKAGE" ]; then
  echo "Error: $DEB_PACKAGE not found."
  echo "Please build the package first. Refer to README.md for build instructions."
  exit 1
fi

# 4. Confirm installation
echo "Found $DEB_PACKAGE."
read -p "Proceed with installation? (y/N) " confirm
if [[ "$confirm" != [yY] ]]; then
  echo "Installation cancelled."
  exit 0
fi

# 5. Install the package
echo "Installing $DEB_PACKAGE..."
if dpkg -i "$DEB_PACKAGE"; then
  echo "$DEB_PACKAGE installed or updated."
else
  echo "Error during dpkg installation. Attempting to fix dependencies..."
  # Proceed to dependency fixing even if dpkg reports an error, as it's often due to missing dependencies.
fi

# 6. Fix dependencies
echo "Attempting to fix any missing dependencies..."
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
