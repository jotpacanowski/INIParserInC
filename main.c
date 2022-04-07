/* INI parser main.c
 * Copyright (c) 2022 Jędrzej Pacanowski
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buf_line_reader.h"
#include "common.h"
#include "ini_parser.h"

// Parse section.key string with whitespace
bool parse_section_key_str(const char** sptr, char** out_section, char** out_key)
{
	register const char* ptr = *sptr;

	char *dot = strchr(ptr, '.');
	if(dot == NULL){
		fprintf(stderr, "Error: expected '.'\n");
		return false;
	}

	// Parse section name
	while(*ptr && isspace(*ptr))ptr++;
	const char *name_of_section = ptr;
	while(*ptr && !isspace(*ptr) && *ptr != '.')ptr++;
	const char *end_of_section = ptr;
	ptr++;

	// Parse key name
	while(*ptr && (isspace(*ptr) || *ptr == '.'))ptr++;
	const char *name_of_key = ptr;
	while(*ptr && !isspace(*ptr))ptr++;
	const char *end_of_key = ptr;

	if(!is_valid_identifier(name_of_section, end_of_section+1-1)){
		fprintf(stderr, "[Err] Invalid section name \n");
		exit(2);
	}
	if(!is_valid_identifier(name_of_key, end_of_key)){
		fprintf(stderr, "[Err] Invalid key name \n");
		exit(2);
	}

	char* const buf_section = calloc(1, 1 + end_of_section - name_of_section);
	strncpy(buf_section, name_of_section, end_of_section - name_of_section);
	char* const buf_key = calloc(1, 1 + end_of_key - name_of_key);
	strncpy(buf_key, name_of_key, end_of_key - name_of_key);

	*out_section = buf_section;
	*out_key = buf_key;

	// Omit ending whitespace
	while(*ptr && isspace(*ptr)) ptr++;
	*sptr = ptr;
	return true;
}

void dump_ini_data(void)
{
	struct IniLinkedList *ptr = global_ini_state;
	int count = 0;
	while(ptr != NULL){
		printf("%s.%s = %s\n",
			ptr->section, ptr->variable, ptr->value);
		count++;
		ptr = ptr->next;
	}
	fprintf(stderr, "Parsed %d variables.\n", count);
}

static inline int main3(const char* arg_var_name)
{
	const char* sec_key_str = arg_var_name;
	char* buf_section;
	char* buf_key;
	bool good = parse_section_key_str(&sec_key_str, &buf_section, &buf_key);
	if(!good){
		fprintf(stderr, "Invalid INI variable name: \"%s\"\n", arg_var_name);
		exit(2);
	}

	size_t our_sec_len = strlen(buf_section);
	size_t our_key_len = strlen(buf_key);

	struct IniLinkedList *iter = global_ini_state;
	bool found = false;
	for(; iter != NULL; iter = iter->next){
		if(iter->section_len != our_sec_len || strcmp(iter->section, buf_section) != 0)
			continue;
		if(iter->variable_len != our_key_len || strcmp(iter->variable, buf_key) != 0)
			continue;
		fprintf(stderr, "Found %s.%s = \"%s\"\n",
			iter->section, iter->variable, iter->value);
		found = true;
		printf("%s\n", iter->value);
		break;
	}

	free(buf_section);
	free(buf_key);

	if(!found){
		fprintf(stderr, "[Err] Variable not found\n");
		return 1;
	}
	return 0;
}

static inline int main4(const char* arg_expression)
{
	(void)(arg_expression); puts("TODO"); exit(123);
}

#ifdef __WIN32__
static void _fix_win32_ansi_escape(void);
#endif

int main(int argc, char** argv)
{
	PROG_NAME = argv[0];
	if(argc <= 1 || argc > 4){
		usage();
		return 1;
	}

	#ifdef __WIN32__
	_fix_win32_ansi_escape();
	#endif

	// TODO: Complain about arguments before reading the file

	read_file_line_by_line(argv[1]);

	int ec = 0;

	if(argc == 2){
		dump_ini_data();
	}else if(argc == 3){
		if(strcmp(argv[2], "expression") == 0){
			fprintf(stderr, "Expected second argument after \"expression\".");
			ec = 1;
		}else{
			ec = main3(argv[2]);
		}

	}else if(argc == 4){
		if(strcmp(argv[2], "expression") != 0){
			fprintf(stderr, "Expected second argument to be \"expression\".");
			ec = 1;
		}else{
			ec = main4(argv[3]);
		}
	}else{
		fprintf(stderr, "???\n");
		usage();
		ec = 1;
	}

	// Free allocated memory
	ini_free_global_state();

	return ec;
}

void usage(void)
{
	fprintf(stderr, "Usage: %s FILE\n", PROG_NAME);
	fprintf(stderr, "       %s FILE section.key\n", PROG_NAME);
	fprintf(stderr, "       %s FILE expression EXPR\n", PROG_NAME);
	exit(1);
}

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// Enable ANSI escape sequences in Windows consoles
// See https://docs.microsoft.com/en-us/windows/console/getconsolemode#parameters
// __attribute__((constructor))
static void _fix_win32_ansi_escape(void)
{
	const int handle_arr[] = {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
	for(int i=0; i < 2; i++){
		const HANDLE _H = GetStdHandle(handle_arr[i]);
		DWORD mode;
		GetConsoleMode(_H, &mode);
		mode = mode | 4; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
		SetConsoleMode(_H, mode);
	}
}
#endif
