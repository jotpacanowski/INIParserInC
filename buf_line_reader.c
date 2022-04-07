/* 2022 Jędrzej Pacanowski */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buf_line_reader.h"
#include "ini_parser.h"

static char LN_FGETS_BUFFER[99];
static const size_t LN_FGETS_BUFFER_LEN = 98;

// Allocate the buffer on the heap if needed
static char* linereadbuf = NULL;
static size_t linereadbuf_sz = 0;

void usage(void); // Used on error - TODO move?

void line_handler(const char* line, const int lineno){

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
// https://en.cppreference.com/w/c/experimental/dynamic/getline
// https://manpages.debian.org/bullseye/manpages-dev/getline.3.en.html
// https://manpages.debian.org/bullseye/manpages-dev/setlocale.3.en.html
// https://elixir.bootlin.com/glibc/latest/source/libio/iogetdelim.c#L40

static ptrdiff_t max_line_size = 1; // TODO Set to the size of static buf.

static void enlarge_linereadbuf(void){
	if(linereadbuf == LN_FGETS_BUFFER){
		linereadbuf_sz = 2 * LN_FGETS_BUFFER_LEN;
		linereadbuf = malloc(linereadbuf_sz);
		strcpy(linereadbuf, LN_FGETS_BUFFER);
	}else{
		linereadbuf_sz *= 2;
		linereadbuf = realloc(linereadbuf, linereadbuf_sz);
	}
}

void read_file_line_by_line(const char* fname){
	FILE* f;
	if(strcmp(fname, "-") == 0)
		f = stdin;
	else
		f = fopen(fname, "r");

	if(!f){
		perror("Failed to open file");
		//fprintf(stderr, "Could not open file %s.\n", fname);
		usage();
	}
	// TODO - atexit fclose

	if(f != stdin){
		int pos = 0;
		int last_pos = 0;
		while(!feof(f)){
			int c = fgetc(f);
			pos++;
			if(c == '\n'){
				if(max_line_size < pos - last_pos)
					max_line_size = pos - last_pos;
				last_pos = pos;
			}
		}
		fprintf(stderr, "Maximum line length is %zu\n", max_line_size);
		rewind(f);

		// Allocate the line buffer
		linereadbuf_sz = 1 + max_line_size;
	}else{
		// linereadbuf = LN_FGETS_BUFFER;
		linereadbuf_sz = 2 * LN_FGETS_BUFFER_LEN;
	}
	linereadbuf = calloc(1, linereadbuf_sz);

	int lineno = 0;
	// while(fgets(LN_FGETS_BUFFER, LN_FGETS_BUFFER_LEN, f) != NULL){
	char* bufptr = linereadbuf;
	while(fgets(bufptr, linereadbuf_sz - (bufptr-linereadbuf), f) != NULL){
		lineno++;
		size_t len = strlen(linereadbuf);
		if(len > 0 && linereadbuf[len-1] != '\n'){
			// read another time to the same buffer
			ptrdiff_t bufoff = bufptr - linereadbuf;
			enlarge_linereadbuf();  // bufptr may not point into the buffer
			lineno--;
			bufptr = linereadbuf + bufoff + len;
			continue;
		}else if(len==0){
			continue;
		}
		linereadbuf[len-1] = '\0'; // Replace end of line char

		// Process the line
		line_handler(linereadbuf, lineno);

		// Clear buffer
		linereadbuf[0] = '\0';
		bufptr = linereadbuf;
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
