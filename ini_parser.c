/* 2022 Jędrzej Pacanowski */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_parser.h"
#include "common.h"

struct IniLinkedList *global_ini_state = NULL;

enum INI_LINE ini_line_category(char c)
{
	if(isspace(c))
		return INI_IGNORE;
	if(c == ';' || c == '#')
		return INI_COMMENT;
	if(c == '[')
		return INI_SECTION_HEADER;
	if(isalnum(c) || c == '-')
		return INI_ASSIGNMENT;

	fprintf(stderr, "\x1b[31;1mUnclassified char %02x ! \x1b[0m\n", c);
	return INI_IGNORE;
}

static char* global_ini_current_section = NULL;  // Owning pointer

static void ini_add_parsed_variable(const char* const varname, const char* const varvalue)
{
	struct IniLinkedList *new_el = malloc(sizeof(struct IniLinkedList));
	new_el->section_len = strlen(global_ini_current_section);
	new_el->section = strdup(global_ini_current_section);
	new_el->variable_len = strlen(varname);
	new_el->variable = strdup(varname);
	new_el->value = strdup(varvalue);
	new_el->value_len = strlen(varvalue);

	// Add to the beginning of the list
	new_el->next = global_ini_state;
	global_ini_state = new_el;
}

void ini_parse_section_header(const char* line, int lineno)
{
	(void)lineno;
	int len = strlen(line);
	if(len < 3){
		fprintf(stderr, "Error: line is too short\n");
		exit(2);
	}
	char* begin = strchr(line, '[');
	char* end = strrchr(line, ']');
	if(begin == NULL){
		fprintf(stderr, "Error: '[' expected.\n");
		exit(2);
	}
	if(end == NULL){
		fprintf(stderr, "Error: ']' expected.\n");
		exit(2);
	}
	if(strchr(line, '[') != strrchr(line, '[')){
		fprintf(stderr, "Error: '[' appears more than once\n");
		exit(2);
	}
	if(strchr(line, ']') != strrchr(line, ']')){
		fprintf(stderr, "Error: ']' appears more than once\n");
		exit(2);
	}

	int section_name_len = end - begin - 1;
	char* const secname =  str_without_ws(begin+1, section_name_len);
	printf("Parsed section name: [%s]\n", secname);

	if(!is_valid_identifier(secname, secname + strlen(secname))){
		fprintf(stderr, "[Err] Invalid section name:\n");
		fprintf(stderr, "      \"%s\"\n", secname);
		exit(2);
	}

	// Handle parsed section name

	if(global_ini_current_section != NULL) // Fix memory leak
		free(global_ini_current_section);
	global_ini_current_section = secname;
}

void ini_parse_var_assignment(const char* line, int lineno)
{
	(void)lineno;
	int len = strlen(line);
	if(len < 3){
		fprintf(stderr, "Error: line is too short\n");
		exit(2);
	}
	char* eq_sign = strchr(line, '=');
	if(eq_sign == NULL){
		fprintf(stderr, "Error: '=' expected.\n");
		exit(2);
	}

	char* const varname = str_without_ws(line, eq_sign - line);
	printf("Parsed variable name: \"%s\"\n", varname);

	if(!is_valid_identifier(varname, varname + strlen(varname))){
		fprintf(stderr, "[Err] Invalid variable name:\n");
		fprintf(stderr, "      \"%s\"\n", varname);
		exit(2);
	}

	char* const value = str_without_ws(eq_sign+1, len - (eq_sign-line+1));
	printf("Parsed variable value: \"%s\"\n", value);

	ini_add_parsed_variable(varname, value);

	free(varname);
	free(value);
}

// __attribute__((destructor))  // does not work on MSVC ;f
 void ini_free_global_state(void)
 {
	fprintf(stderr, "Freeing global state.\n");
	struct IniLinkedList *ptr, *old;
	for(ptr = global_ini_state; ptr != NULL; ){
		free(ptr->section);
		free(ptr->variable);
		free(ptr->value);

		old = ptr;
		ptr = ptr->next;
		free(old);
	}
}
