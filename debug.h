#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

void debug_print_indent(void);

#if defined(DEBUG)
#define P_DEBUG(...) do {                                               \
		debug_print_indent(); \
		printf("  | ");                                         \
		printf(__VA_ARGS__);                                    \
} while (0)
#else
#define P_DEBUG(...)
#endif

#endif
