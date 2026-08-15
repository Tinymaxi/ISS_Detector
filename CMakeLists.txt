cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Pico WH = Pico W (RP2040) with headers pre-soldered, including the debug
# header. The SDK has no separate pico_wh board; electrically it is a pico_w.
set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(iss_box C CXX ASM)

pico_sdk_init()

add_executable(iss_box
    src/main.cpp
    src/tools/i2c_scan.cpp
    src/drivers/soft_i2c.cpp
    src/drivers/i2c_bus.cpp
)

pico_set_program_name(iss_box "iss_box")
pico_set_program_version(iss_box "0.1")

# Stage 0: no SWD probe available (Pico 2 W uses a JST-SH debug connector).
# Console goes over USB CDC instead.
pico_enable_stdio_uart(iss_box 0)
pico_enable_stdio_usb(iss_box 1)

target_link_libraries(iss_box
    pico_stdlib
    pico_cyw43_arch_none      # onboard LED lives on the wireless chip; no networking yet
)

target_include_directories(iss_box PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_add_extra_outputs(iss_box)
