#include "generator.h"
#include <stdlib.h>

#define OUTPUT_WRITE(...) if(!fprintf(file, __VA_ARGS__)) { \
			fprintf(stderr, "E: Failed to write to output file!\n"); \
			fclose(file); \
			exit(1); \
		} \

char bin_op_char[] = {'+', '-', '*', '/', '%'};

void generate_c_statement(struct statement* stmt, FILE* file) {
	
	switch(stmt->type) {
	case NUMERIC_LITERAL: {
		OUTPUT_WRITE("%d", ((struct numeric_literal*)stmt)->num);
		break;
	}
	case BINARY_EXPRESSION:
		OUTPUT_WRITE("(");
		generate_c_statement(((struct binary_expression*)stmt)->left, file);
		OUTPUT_WRITE("%c", bin_op_char[((struct binary_expression*)stmt)->operator]);
		generate_c_statement(((struct binary_expression*)stmt)->right, file);
		OUTPUT_WRITE(")");
		break;
	case IDENTIFIER:
		OUTPUT_WRITE("%s", ((struct identifier*)stmt)->symbol);
		break;
	case FUNCTION_DECLARATION:
		for(struct statement_list* node = ((struct function_declaration*)stmt)->body; node->next; node = node->next) {
			if(node->statement->type == FUNCTION_DECLARATION) generate_c_statement(node->statement, file);
		}
		OUTPUT_WRITE("void %s(){", ((struct function_declaration*)stmt)->name);
		for(struct statement_list* node = ((struct function_declaration*)stmt)->body; node->next; node = node->next) {
			if(node->statement->type != FUNCTION_DECLARATION) {
				generate_c_statement(node->statement, file);
				OUTPUT_WRITE(";");
			}
		}
		OUTPUT_WRITE("}");
		break;
	default:
		fprintf(stderr, "Unimplemented statement: %d\n", stmt->type);
		fclose(file);
		exit(1);
	}
	
}

void generate_c(struct ast* ast, const char* path) {
	
	FILE* file = fopen(path, "w");
	if(!file) {
		fprintf(stderr, "E: Failed to open/create file \"%s\"!\n", path);
		exit(1);
	}
	
	for(struct statement_list* node = ast->body; node->next; node = node->next) {
		
		generate_c_statement(node->statement, file);
		
	}
	
	fclose(file);
	
}
