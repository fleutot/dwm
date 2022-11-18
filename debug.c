#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

#include "debug.h"

static int print_indent_level = 0;

__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	print_indent_level++;

	Dl_info info;
	if (dladdr(this_fn, &info)) {
		debug_print_indent();
		printf("+ [%s] %s\n",
		       info.dli_fname ? info.dli_fname : "?",
		       info.dli_sname ? info.dli_sname : "?");
	}
}

__attribute__((no_instrument_function))
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	print_indent_level--;
}

__attribute__((no_instrument_function))
void debug_print_indent(void)
{
	assert(print_indent_level >= 0);
	for (int i = 0; i < print_indent_level; i++) {
		printf("  ");
	}
}
