#!/usr/bin/env bash

# Setup
# Exit the script if any command fails
set -e


# Libwebsockets
sudo apt install libssl-dev
cd /tmp
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
mkdir build && cd build
cmake -DLWS_IPV6=ON ..
make -j$(nproc)
sudo make install


sudo ldconfig
