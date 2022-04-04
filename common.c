/* 2022 Jędrzej Pacanowski */
#include "common.h"

#include <ctype.h>

const char* PROG_NAME = "./parse";

void str_eat_ws(const char **s, size_t *len){
	// remove prefix
	const char *ptr = *s;
	const char* const end = ptr + *len;
	while(ptr != end && isspace(*ptr))
		ptr++;

	// remove suffix
	long long len2 = *len;
	while(len2 > 1 && isspace((*s)[len2-1]))
		len2--;

	// empty result
	if(len2 < 0 || *s + len2 <= ptr)
		len2 = 0;

	*len = len2 - (ptr - *s);
	*s = ptr;
}

char* str_without_ws(const char* s, size_t len){
	const char* s2 = s;
	size_t len2 = len;
	str_eat_ws(&s2, &len2);
	return strdup_substring(s2, len2);
}

bool is_valid_identifier(const char* sbegin, const char* send)
{
	while(sbegin != send && (isalnum(*sbegin) || *sbegin == '-'))
		sbegin++;

	return sbegin == send;
}
