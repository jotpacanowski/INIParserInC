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
		fprintf(stderr, "[Err] Expected '.'\n");
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
		return false;
	}
	if(!is_valid_identifier(name_of_key, end_of_key)){
		fprintf(stderr, "[Err] Invalid key name \n");
		return false;
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

static struct IniLinkedList* lookup_ini_data(const char* section, const char* key)
{
	size_t sec_len = strlen(section);
	size_t key_len = strlen(key);

	bool section_exists = false;

	struct IniLinkedList *iter;
	for(iter = global_ini_state; iter != NULL; iter = iter->next){
		if(!section_exists && strcmp(section, iter->section) == 0)
			section_exists = true;

		if(iter->section_len != sec_len || iter->variable_len != key_len)
			continue;
		if(strcmp(iter->section, section) != 0)
			continue;
		if(strcmp(iter->variable, key) != 0)
			continue;
		return iter;
	}
	if(section_exists)
		fprintf(stderr, "[Warn] Failed to find key \"%s\" in section [%s]\n",
			key, section);
	else
		fprintf(stderr, "[Warn] Failed to find section \"%s\"\n", section);
	return NULL;
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

	struct IniLinkedList* it = lookup_ini_data(buf_section, buf_key);
	if(it == NULL)
		fprintf(stderr, "[Err] Variable not found\n");
	else
		printf("%s\n", it->value);

	free(buf_section);
	free(buf_key);

	if(!it)
		return 1;
	return 0;
}

static bool is_this_an_integer(const char* const s)
{
	if(s == NULL || s[0] == '\0') return false;
	bool neg = s[0] == '-';
	const char* p = neg? s+1 : s;
	while(*p != '\0' && isdigit(*p))
		p++;
	// p -> end of digits (maybe end of s)
	if(*p == '\0' && p-s == 1 && neg)
		return false; // "-"
	return *p == '\0';
}

static bool evaluate_expression(const char* lhs, char op, const char* rhs)
{
	// Check type (!float, int, string)
	bool is_lhs_int = is_this_an_integer(lhs);
	bool is_rhs_int = is_this_an_integer(rhs);
	bool both_ints = is_lhs_int && is_rhs_int;

	long long lhs_i = 0, rhs_i = 0;
	if(both_ints){
		lhs_i = atoll(lhs);
		rhs_i = atoll(rhs);
	}

	if(op == '+' && both_ints){
		fprintf(stderr, "[OK] integer addition\n");
		printf("%lld\n", lhs_i + rhs_i);
		return true;
	}else if(op == '+' && !both_ints){
		if(is_lhs_int || is_rhs_int)
			fprintf(stderr, "[Warn] One of the arguments "
				"is not an integer\n");
		fprintf(stderr, "[OK] string concatenation\n");
		printf("%s%s\n", lhs, rhs);
		return true;
	}

	// The rest of operators, "-*/" need integer arguments (maybe float (?))
	if(!both_ints){
		fprintf(stderr, "[Err] Wrong arguments for \'%c\'\n", op);
		if(!is_lhs_int)
			fprintf(stderr, "[Err] -> \"%s\"\n", lhs);
		if(!is_rhs_int)
			fprintf(stderr, "[Err] -> \"%s\"\n", rhs);
		return false;
	}

	switch(op){
		case '-':
			printf("%lld\n", lhs_i - rhs_i);
			break;
		case '*':
			printf("%lld\n", lhs_i * rhs_i);
			break;
		case '/':
			printf("%lld\n", lhs_i / rhs_i);
			printf("fp: %lf\n", lhs_i / (double)rhs_i);
			break;
		default:
			fprintf(stderr, "[Err] Unknown operation \'%c\'", op);
			return false;
	}
	return true;
}

// static const char EXPR_OPS[] = "+-*/";

static inline int main4(const char* arg_expression)
{
	int status = 0;

	const char* buf_expr = arg_expression;
	char* p1_sec;
	char* p1_key;
	bool good = parse_section_key_str(&buf_expr, &p1_sec, &p1_key);
	const char* const oper = buf_expr;
	char o = *oper; // Skipped whitespace.
	if(!good){
		fprintf(stderr, "Invalid INI variable name: \"%s\"\n", buf_expr);
		exit(2);
	}

	// asm("int3");

	buf_expr = oper + 1;
	char* p2_sec;
	char* p2_key;
	good = parse_section_key_str(&buf_expr, &p2_sec, &p2_key);
	if(!good){
		fprintf(stderr, "Invalid INI variable name: \"%s\"\n", buf_expr);
		exit(2);
	}

	fprintf(stderr, "# EXPRESSION:\n");
	fprintf(stderr, "#     %s.%s\n", p1_sec, p1_key);
	fprintf(stderr, "# [%c]  \n", o);
	fprintf(stderr, "#     %s.%s\n", p2_sec, p2_key);

	// Find the values in the linked list
	struct IniLinkedList *v1 = lookup_ini_data(p1_sec, p1_key);
	struct IniLinkedList *v2 = lookup_ini_data(p2_sec, p2_key);

	if(v1 == NULL && v2 == NULL){
		fprintf(stderr, "Could not find both %s.%s and %s.%s\n",
			p1_sec, p1_key, p2_sec, p2_key);
		status = 2;
		goto main4_error;
	}
	if(v1 == NULL){
		fprintf(stderr, "Could not find %s.%s\n", p1_sec, p1_key);
		status = 2;
		goto main4_error;
	}
	if(v2 == NULL){
		fprintf(stderr, "Could not find %s.%s\n", p2_sec, p2_key);
		status = 2;
		goto main4_error;
	}

	// We have got the values
	fprintf(stderr, "# EXPRESSION:\n");
	fprintf(stderr, "# \"%15s\" [%c] \"%15s\" \n", v1->value, o, v2->value);

	bool ok = evaluate_expression(v1->value, o, v2->value);
	if(!ok)
		status = 1;

main4_error:
	free(p1_sec);
	free(p1_key);
	free(p2_sec);
	free(p2_key);
	return status;
}

int main(int argc, char** argv)
{
	PROG_NAME = argv[0];
	if(argc <= 1 || argc > 4){
		usage();
		return 1;
	}

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
