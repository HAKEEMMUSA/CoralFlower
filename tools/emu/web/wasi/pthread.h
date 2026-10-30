#pragma once
/* single-threaded wasm (the browser emulator): the few pthread calls emu_hal_fw.h makes */
#define __NEED_pthread_t
#include <bits/alltypes.h>
static inline pthread_t pthread_self(void) { return (pthread_t)1; }
static inline int pthread_equal(pthread_t a, pthread_t b) { return a == b; }
