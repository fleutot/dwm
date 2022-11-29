#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

//__attribute__((no_instrument_function))
//static const char *extract_filename(const char *in)
//{
//	char *last_sep = strrchr(in, '/');
//	return last_sep ? last_sep + 1 : in;
//}

static int print_indent_level = 0;

__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	print_indent_level++;

	Dl_info info;
	if (dladdr(this_fn, &info)) {
		debug_print_indent();
		printf("+ %s\n",
		       info.dli_sname ? info.dli_sname : "?");
		//info.dli_fname ? extract_filename(info.dli_fname) : "?");
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
		printf("    ");
	}
}
