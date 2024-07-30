#!/bin/sh

# If you want to build with Xcode, you need to uncomment this and set it to your Pico SDK path: 
#PICO_SDK_PATH=

PICO_BOARD=pico_w

mkdir build 2>/dev/null

cd build

if [ ! -f "Makefile" ]; then
	# rebuild the cache
	cmake ..
fi

cmake --build .

#env
