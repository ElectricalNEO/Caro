#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "generator.h"

#define DEFAULT_OUTPUT "caro.out"

struct compilation_options {
	const char* input;
	const char* output;
	int preserve;
};

void help() {
	
	printf("Usage: caro [options] file\n");
	printf("\t[-h | --help] - print this help message\n");
	printf("\t[-o | --output] file - set output file (default: \"%s\")\n", DEFAULT_OUTPUT);
	printf("\t[-p | --preserve] - don't delete the temporary C file\n");
	exit(1);
	
}

struct compilation_options parse_args(int argc, char** argv) {
	
	struct compilation_options opt = {0};
	
	for(int i = 1; i < argc; i++) {
		
		if(!strcmp("-h", argv[i]) || !strcmp("--help", argv[i])) help();
		if(!strcmp("-p", argv[i]) || !strcmp("--preserve", argv[i])) {
			opt.preserve = 1;
			continue;
		}
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
			continue;
		}
		if(opt.input) {
			fprintf(stderr, "E: More than one input file specified!\n");
			exit(1);
		}
		opt.input = argv[i];
		
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

void build(struct ast* ast, const char* path, int preserve) {
	
	int size = snprintf(0, 0, "%s.c", path);
	char p[size + 1];
	snprintf(p, size + 1, "%s.c", path);
	
	generate_c(ast, p);
	
	size = snprintf(0, 0, "gcc %s -o %s", p, path);
	char cmd[size + 1];
	snprintf(cmd, size + 1, "gcc %s -o %s", p, path);
	
	system(cmd);
	
	if(!preserve) remove(p);
	
}

int main(int argc, char** argv) {
	
	struct compilation_options opt = parse_args(argc, argv);
	char* source = slurp_file(opt.input);
	struct token* tokens = tokenize(source);
	struct ast* ast = parse(tokens);
	build(ast, opt.output, opt.preserve);
	
}
