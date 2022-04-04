#pragma once
#ifndef HDR_PARSER
#define HDR_PARSER

#include <stdlib.h>

enum INI_LINE {
	INI_IGNORE,
	INI_SECTION_HEADER,
	INI_ASSIGNMENT,
	INI_COMMENT
};

enum INI_LINE ini_line_category(char c);

void ini_parse_section_header(const char* line);
void ini_parse_var_assignment(const char* line);

struct IniLinkedList {
	struct IniLinkedList *next;
	// All char* in this structure point to
	// unique copies of strings (no reference counting)
	char *section;
	size_t section_len;
	char *variable;
	size_t variable_len;
	char *value;
	size_t value_len;
};

extern struct IniLinkedList *global_ini_state;

#endif
