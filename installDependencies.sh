#!/usr/bin/env bash

# Setup
# Exit the script if any command fails
set -e


# Libwebsockets
sudo apt install libssl-dev
cd /tmp
git clone --depth 1 -b v4.0.15 https://github.com/warmcat/libwebsockets.git
cd libwebsockets
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install


sudo ldconfig
