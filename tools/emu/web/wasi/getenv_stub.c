#include <stddef.h>
/* the browser has no environment: the worklet stubs WASI, so no getenv lookups through environ_get */
char *getenv(const char *name) { (void)name; return NULL; }
