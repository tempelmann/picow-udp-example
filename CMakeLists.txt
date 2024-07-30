# Part of https://github.com/tempelmann/picow-udp-example

cmake_minimum_required(VERSION 3.12)

# Name of executable.
set(NAME pico)

# imports
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# C/C++ boilerplate.
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add all source files
add_executable(
	${NAME} main.c
)

# Libraries to link
target_link_libraries(${NAME}
	pico_stdlib
	pico_cyw43_arch_lwip_threadsafe_background
)

# Include dirs
target_include_directories(pico PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)

# Defines for code compilation
target_compile_definitions(${NAME} PRIVATE
	PICO_ENTER_USB_BOOT_ON_EXIT=1 # https://github.com/raspberrypi/picotool/issues/88#issuecomment-2253028113
	#IP_REASSEMBLY=1	# https://forums.raspberrypi.com/viewtopic.php?t=373977#p2240769
)

# enable usb output
pico_enable_stdio_usb(${NAME} 1)

# Create map/bin/hex/uf2 files 
pico_add_extra_outputs(${NAME})
