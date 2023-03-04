#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

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

int main(int argc, char** argv) {
	
	struct compilation_options opt = parse_args(argc, argv);
	char* source = slurp_file(opt.input);
	struct token* tokens = tokenize(source);
	
	for(struct token* token = tokens; token->type != TOKEN_END; token = (struct token*)((size_t)token + sizeof(struct token) + token->size)) {
		if(token->type == TOKEN_KEYWORD) printf("KEYWORD\n");
		else if(token->type == TOKEN_STRING_LITERAL || token->type == TOKEN_OPERATOR || token->type == TOKEN_IDENTIFIER) printf("%s\n", token->data);
		else if(token->type == TOKEN_INTEGER_LITERAL) printf("%i\n", *(int*)token->data);
		else if(token->type == TOKEN_PUNCTUATOR) printf("%c\n", token->data[0]);
	}
	
}
