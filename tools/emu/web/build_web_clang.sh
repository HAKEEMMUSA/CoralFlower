#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Leviathan: the browser emulator's wasm without Emscripten: clang's wasm32-wasi target and the WASI libc (Debian /
# Ubuntu: apt install clang lld wasi-libc libclang-rt-18-dev-wasm32). The module has the same exports and the same
# few WASI imports the worklet stubs; tools/emu/web/test_web_emu.mjs checks it as it checks emcc's (bit-exact with
# the native headless runner). tools/emu/web/wasi/: a pthread.h and a sys/wait.h for the single-threaded build, and a
# getenv that finds nothing (the worklet's WASI stub has no environment).
#   sh tools/emu/web/build_web_clang.sh   -> build/emu-web/{coralflower.wasm, index.html, emu.js, keymap.js, worklet.js}
set -e
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$ROOT"
[ -d build/gen ] || python3 tools/build.py --gen-only
VER=${CR_VERSION:-$(sed -n 's/^#define FELUCCA_VERSION "\(.*\)".*/\1/p' firmware/src/coralflower.c | head -1)}
W=tools/emu/web
OUT=build/emu-web
CLANG=${CLANG:-clang}
RES=$($CLANG -print-resource-dir)
mkdir -p "$OUT"
$CLANG --target=wasm32-wasi -mexec-model=reactor -O2 -w -std=gnu11 -nostdlibinc -isystem "$RES/include" \
    -isystem /usr/include/wasm32-wasi -I$W/wasi -D__EMSCRIPTEN__ -D_WASI_EMULATED_SIGNAL \
    -D_WASI_EMULATED_PROCESS_CLOCKS -D_WASI_EMULATED_GETPID -I$W/compat -Ibuild/gen -Ifirmware/src -DEMU_WEB \
    "-DFELUCCA_VERSION=\"$VER\"" -Wl,--export-dynamic -Wl,-z,stack-size=1048576 -Wl,--initial-memory=33554432 \
    -o "$OUT/coralflower.wasm" $W/wasi/getenv_stub.c $W/emu_web.c \
    -lwasi-emulated-signal -lwasi-emulated-process-clocks -lwasi-emulated-getpid
cp $W/index.html $W/emu.js $W/keymap.js $W/worklet.js "$OUT/"
echo "emu-web (clang): $OUT ($VER), $(wc -c < "$OUT/coralflower.wasm") B"
