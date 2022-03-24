/* INI parser main.c
 * Copyright (c) 2022 Jędrzej Pacanowski
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* PROG_NAME = "./parse";
void usage(void);

int main(int argc, char** argv){
	PROG_NAME = argv[0];
	if(argc <= 1 || argc > 4){
		usage();
		return 1;
	}

	// ...
	return 1;
}

void usage(void){
	fprintf(stderr, "Usage: %s FILE\n", PROG_NAME);
	fprintf(stderr, "       %s FILE section.key\n", PROG_NAME);
	fprintf(stderr, "       %s FILE expression EXPR\n", PROG_NAME);
	exit(1);
}
