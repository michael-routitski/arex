#!/usr/bin/env bash
set -euo pipefail

PURGE=false
RECONFIG=false

# ------------------------------------------------------------------------------
# Flag Parsing (-p for purge, -r for reconfigure)
# ------------------------------------------------------------------------------
while getopts "pr" opt; do
  case ${opt} in
    p)
      PURGE=true
      ;;
    r)
      RECONFIG=true
      ;;
    \?)
      echo "Usage: $0 [-p] [-r]"
      echo "  -p : Purge build directory (rm -rf build/) and reconfigure"
      echo "  -r : Reconfigure CMake preset without purging build directory"
      exit 1
      ;;
  esac
done

# ------------------------------------------------------------------------------
# Purge / Reconfigure Logic
# ------------------------------------------------------------------------------
if [ "${PURGE}" = true ]; then
  echo -e "\n[!] Purging build directory..."
  rm -rf build/
fi

# Auto-reconfigure if requested or if build directory does not exist
if [ "${PURGE}" = true ] || [ "${RECONFIG}" = true ] || [ ! -d "build/dev" ]; then
  echo -e "\nConfiguring CMake preset 'dev'..."
  cmake --preset dev
fi

# ------------------------------------------------------------------------------
# Build & Test Steps
# ------------------------------------------------------------------------------
echo -e "\nRunning build..."
cmake --build --preset dev

echo -e "\nRunning tests..."
ctest --test-dir build/dev --output-on-failure

echo -e "\nBuild and test complete!"
echo "-----------------------"
