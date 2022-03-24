/* INI parser main.c
 * Copyright (c) 2022 Jędrzej Pacanowski
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buf_line_reader.h"
#include "ini_parser.h"

const char* PROG_NAME = "./parse";
void usage(void);

void dump_ini_data(void){
	struct IniLinkedList *ptr = global_ini_state;
	int count = 0;
	while(ptr != NULL){
		printf("[\"%s\"] \"%s\" = \"%s\"\n",
			ptr->section, ptr->variable, ptr->value);
		count++;
		ptr = ptr->next;
	}
	fprintf(stderr, "Parsed %d variables.\n", count);
}

static inline int main3(const char* arg_variable_name){
	return 5; // TODO
}

static inline int main4(const char* arg_expression){
	return 5; // TODO
}

int main(int argc, char** argv){
	PROG_NAME = argv[0];
	if(argc <= 1 || argc > 4){
		usage();
		return 1;
	}
	// TODO: Complain about arguments before reading the file

	read_file_line_by_line(argv[1]);

	if(argc == 2){
		dump_ini_data();
		return 0;
	}else if(argc == 3){
		if(strcmp(argv[2], "expression") == 0){
			fprintf(stderr, "Expected second argument after \"expression\".");
			return 1;
		}
		return main3(argv[2]);
	}else if(argc == 4){
		if(strcmp(argv[2], "expression") != 0){
			fprintf(stderr, "Expected second argument to be \"expression\".");
			return 1;
		}
		return main4(argv[3]);
	}else{
		fprintf(stderr, "???\n");
		usage();
		return 1;
	}
}

void usage(void){
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
__attribute__((constructor)) // Does this work in MSVC?
void _fix_win32_ansi_escape(void){
	const int handle_arr[] = {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
	for(int i=0; i < 2; i++){
		const HANDLE _H = GetStdHandle(handle_arr[i]);
		DWORD mode;
		GetConsoleMode(_H, &mode);
		mode = mode | 4; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
		SetConsoleMode(_H, mode);
	}
	fprintf(stderr, " [win32] Init term\n");
}
#endif
