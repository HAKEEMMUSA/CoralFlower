#pragma once
/* single-threaded wasm: hostsim.c's render helpers fork; never called in the browser */
#include <sys/types.h>
static inline pid_t fork(void) { return -1; }
static inline pid_t waitpid(pid_t p, int *st, int o) { (void)p; (void)st; (void)o; return -1; }
#define WIFEXITED(s) 1
#define WEXITSTATUS(s) 0
