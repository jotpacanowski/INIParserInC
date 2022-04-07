#pragma once
#ifndef HDR_COMMON
#define HDR_COMMON

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern const char* PROG_NAME;
extern void usage(void); // defined in main.c

// Omit whitespace at start and the end of (pointer, size)
void str_eat_ws(const char **s, size_t *len);

char* str_without_ws(const char* s, size_t len);

bool is_valid_identifier(const char* sbegin, const char* send);

static inline char* strdup_substring(const char* s, const size_t len)
{
	char* ret = calloc(1, 1+len);
	ret[0] = '\0';
	strncat(ret, s, len);
	return ret;
}

#endif
