/* 2022 Jędrzej Pacanowski */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buf_line_reader.h"
#include "ini_parser.h"

// TODO Reading of long lines

static char LN_FGETS_BUFFER[99];
static const size_t LN_FGETS_BUFFER_LEN = 98;

extern void usage(void); // Used on error

static void line_handler(const char* line, const int lineno){
	const char* ptr = line;
	while(*ptr != '\0' && (*ptr == ' ' || *ptr == '\t'))
		ptr++;

	fprintf(stderr, "\x1b[32;1m%2d:\x1b[0m (len=%3zu)", lineno, strlen(ptr));
	if(*ptr == '\0'){
		fprintf(stderr, "\n  Empty line\n");
		return;
	}else
		fprintf(stderr, " %s\n", ptr);

	fprintf(stderr, "  First non-ws char: %c (%2x) at %d:%zd\n",
		*ptr, *ptr, lineno, ptr-line);

	enum INI_LINE categ = ini_line_category(ptr[0]);

	switch(categ){
		case INI_SECTION_HEADER: ini_parse_section_header(line); break;
		case INI_ASSIGNMENT: ini_parse_var_assignment(line); break;
		default: break;
	}
}

// Links:
// https://en.cppreference.com/w/c/io/fgets

void read_file_line_by_line(const char* fname){
	FILE* f;
	if(strcmp(fname, "-") == 0)
		f = stdin;
	else
		f = fopen(fname, "r");

	if(!f){
		fprintf(stderr, "Could not open file \"%s\": ", fname);
		perror("");
		exit(2);
	}

	int lineno = 0;
	while(fgets(LN_FGETS_BUFFER, LN_FGETS_BUFFER_LEN, f) != NULL){
		lineno++;
		size_t len = strlen(LN_FGETS_BUFFER);
		if(len > 0 && LN_FGETS_BUFFER[len-1] != '\n'){
			// continue; // read another time to the same buffer
			fprintf(stderr, "\x1b[32;1m%2d:\x1b[0m (len=0)\n", lineno);
			fprintf(stderr, "\n Please recompile with larger buffer.\n  :(\n");
			exit(2);
		}else if(len==0){
			fprintf(stderr, "\x1b[32;1m%2d:\x1b[0m (len=0)\n", lineno);
			continue;
		}
		LN_FGETS_BUFFER[len-1] = '\0'; // Replace end of line char

		// Process the line
		line_handler(LN_FGETS_BUFFER, lineno);

		// Clear buffer
		LN_FGETS_BUFFER[0] = '\0';
	}
	if(!feof(f)){
		fclose(f);
		fprintf(stderr, "Error while reading \"%s\"\n", fname);
		perror("Reading file");
		exit(2);
	}

	fprintf(stderr, "Read %d lines.\n", lineno);
	fclose(f);
}
