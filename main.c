#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

#define DEFAULT_OUTPUT "caro.out"

struct compilation_options {
	const char* input;
	const char* output;
};

void help() {
	
	printf("Usage: caro [options] file\n");
	printf("\t[-h | --help] - print this help message\n");
	printf("\t[-o | --output] file - set output file (default: \"%s\")\n", DEFAULT_OUTPUT);
	exit(1);
	
}

struct compilation_options parse_args(int argc, char** argv) {
	
	struct compilation_options opt = {0};
	
	for(int i = 1; i < argc; i++) {
		
		if(!strcmp("-h", argv[i]) || !strcmp("--help", argv[i])) help();
		if(!strcmp("-o", argv[i]) || !strcmp("--output", argv[i])) {
			if(opt.output) {
				fprintf(stderr, "E: More than one output file specified!\n");
				exit(1);
			}
			i++;
			if(i == argc) {
				fprintf(stderr, "E: Expected file name after \"%s\"!\n", argv[i - 1]);
				exit(1);
			}
			opt.output = argv[i];
		} else {
			if(opt.input) {
				fprintf(stderr, "E: More than one input file specified!\n");
				exit(1);
			}
			opt.input = argv[i];
		}
		
	}
	
	if(!opt.input) {
		fprintf(stderr, "E: No input file specified!\n");
		exit(1);
	}
	if(!opt.output) opt.output = DEFAULT_OUTPUT;
	
	return opt;
	
}

char* slurp_file(const char* path) {
	
	FILE* file = fopen(path, "r");
	if(!file) {
		fprintf(stderr, "E: Failed to open file \"%s\"!\n", path);
		exit(1);
	}
	
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* data = malloc(size + 1);
	if(!data) {
		fclose(file);
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	
	fread(data, size, 1, file);
	fclose(file);
	
	if(ferror(file)) {
		fprintf(stderr, "E: Failed to read from file \"%s\"!\n", path);
		exit(1);
	}
	
	data[size] = 0;
	
	return data;
	
}

char bin_op_char[] = {'+', '-', '*', '/', '%'};

void print_statement(struct statement* stmt, int indentation) {
	
	for(int i = 0; i < indentation; i++) printf("\t");
	if(!stmt) {
		printf("NULL\n");
		return;
	}
	switch(stmt->type) {
	case NUMERIC_LITERAL:
		printf("%d\n", ((struct numeric_literal*)stmt)->num);
		break;
	case BINARY_EXPRESSION:
		printf("BINARY EXPRESSION %c\n", bin_op_char[((struct binary_expression*)stmt)->operator]);
		print_statement(((struct binary_expression*)stmt)->left, indentation + 1);
		print_statement(((struct binary_expression*)stmt)->right, indentation + 1);
		break;
	case IDENTIFIER:
		printf("%s\n", ((struct identifier*)stmt)->symbol);
		break;
	}
	
}

int main(int argc, char** argv) {
	
	struct compilation_options opt = parse_args(argc, argv);
	char* source = slurp_file(opt.input);
	struct token* tokens = tokenize(source);
	struct ast* ast = parse(tokens);
	
	for(struct statement_list* node = ast->body; node->next; node = node->next) {
		
		print_statement(node->statement, 0);
		
	}
	
}
