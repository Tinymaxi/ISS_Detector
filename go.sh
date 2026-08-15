#!/usr/bin/env bash
# Build / flash / monitor helper for the ISS-Box.
#
#   ./go.sh              build only
#   ./go.sh flash        build, then load onto the Pico 2 W
#   ./go.sh monitor      open the USB serial console
#   ./go.sh flash monitor
#
# First flash needs BOOTSEL held while plugging in USB. After that the firmware
# exposes a reset interface, so picotool can reboot the board itself.

set -euo pipefail
cd "$(dirname "$0")"

# These must stay in step with CMakeLists.txt and .vscode/settings.json, so that
# building from the shell and building from the VS Code button use one toolchain.
PICO_SDK_TOOLS="$HOME/.pico-sdk"
NINJA_VERSION="v1.13.2"
TOOLCHAIN_VERSION="15_2_Rel1"
PICOTOOL_VERSION="2.3.0"
CMAKE_VERSION="v4.3.4"

export PATH="$PICO_SDK_TOOLS/ninja/$NINJA_VERSION:$PICO_SDK_TOOLS/toolchain/$TOOLCHAIN_VERSION/bin:$PICO_SDK_TOOLS/cmake/$CMAKE_VERSION/bin:$PATH"
PICOTOOL="$PICO_SDK_TOOLS/picotool/$PICOTOOL_VERSION/picotool/picotool"

do_flash=false
do_monitor=false
for arg in "$@"; do
    case "$arg" in
        flash)   do_flash=true ;;
        monitor) do_monitor=true ;;
        *) echo "unknown argument: $arg" >&2; exit 1 ;;
    esac
done

# --- build -----------------------------------------------------------------
cmake -S . -B build -G Ninja > /dev/null
cmake --build build

echo
arm-none-eabi-size build/iss_box.elf

# --- flash -----------------------------------------------------------------
if $do_flash; then
    echo
    echo "Loading onto the board..."
    # -f asks a running board to reboot into BOOTSEL; -x runs it afterwards.
    "$PICOTOOL" load -x -f build/iss_box.uf2
fi

# --- monitor ---------------------------------------------------------------
if $do_monitor; then
    # The board re-enumerates after a reset, so wait for the port to reappear.
    port=""
    for _ in $(seq 1 40); do
        port=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1 || true)
        [ -n "$port" ] && break
        sleep 0.25
    done

    if [ -z "$port" ]; then
        echo "No /dev/cu.usbmodem* appeared. Is the board plugged in?" >&2
        exit 1
    fi

    echo
    echo "Console on $port  —  quit with Ctrl-A then K"
    exec screen "$port" 115200
fi
