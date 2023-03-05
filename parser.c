#include "parser.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct token* consume_token(struct token** tokens) {
	
	struct token* tok = *tokens;
	*tokens = (struct token*)((size_t)(*tokens) + (*tokens)->size + sizeof(struct token));
	return tok;
	
}

struct statement* parse_primary_expression(struct token** tokens) {
	
	switch((*tokens)->type) {
	case TOKEN_INTEGER_LITERAL: {
		struct token* token = consume_token(tokens);
		struct numeric_literal* stmt = malloc(sizeof(struct numeric_literal));
		if(!stmt) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		stmt->stmt.type = NUMERIC_LITERAL;
		stmt->num = *(int*)token->data;
		return (struct statement*)stmt;
	}
	case TOKEN_IDENTIFIER:
		/// TODO: create identifier node
		assert(0);
		break;
	default:
		return 0;
	}
	
}

struct statement* parse_multiplicative_expression(struct token** tokens) {
	
	struct statement* left = parse_primary_expression(tokens);
	if(!left) return 0;
	
	while((*tokens)->type == TOKEN_OPERATOR && (!strcmp((*tokens)->data, "*") || !strcmp((*tokens)->data, "/") || !strcmp((*tokens)->data, "%"))) {
		
		struct token* tok = consume_token(tokens);
		struct statement* right = parse_primary_expression(tokens);
		if(!right) {
			fprintf(stderr, "E: Invalid expression in line %d!\n", tok->line);
			exit(1);
		}
		struct binary_expression* operation = malloc(sizeof(struct binary_expression));
		if(!operation) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		operation->stmt.type = BINARY_EXPRESSION;
		operation->left = left;
		operation->right = right;
		if(!strcmp(tok->data, "*")) operation->operator = OP_MULTIPLY;
		else if(!strcmp(tok->data, "/")) operation->operator = OP_DIVIDE;
		else operation->operator = OP_MODULO;
		left = (struct statement*)operation;
		
	}
	
	return left;
	
}

struct statement* parse_additive_expression(struct token** tokens) {
	
	struct statement* left = parse_multiplicative_expression(tokens);
	if(!left) return 0;
	
	while((*tokens)->type == TOKEN_OPERATOR && (!strcmp((*tokens)->data, "+") || !strcmp((*tokens)->data, "-"))) {
		
		struct token* tok = consume_token(tokens);
		struct statement* right = parse_multiplicative_expression(tokens);
		if(!right) {
			fprintf(stderr, "E: Invalid expression in line %d!\n", tok->line);
			exit(1);
		}
		struct binary_expression* operation = malloc(sizeof(struct binary_expression));
		if(!operation) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		operation->stmt.type = BINARY_EXPRESSION;
		operation->left = left;
		operation->right = right;
		if(!strcmp(tok->data, "+")) operation->operator = OP_ADD;
		else operation->operator = OP_SUBTRACT;
		left = (struct statement*)operation;
		
	}
	
	return left;
	
}

struct statement* parse_expression(struct token** tokens) {
	
	return parse_additive_expression(tokens);
	
}

struct statement* parse_statement(struct token** tokens) {
	
	struct statement* ret = parse_expression(tokens);
	
	struct token* tok = consume_token(tokens);
	if(tok->type != TOKEN_PUNCTUATOR || tok->data[0] != ';') {
		fprintf(stderr, "E: Unexpected token in line %d! Expected semicolon.\n", tok->line);
		exit(1);
	}
	
	return ret;
	
}

struct ast* parse(struct token* tokens) {
	
	struct ast* ast = malloc(sizeof(struct ast));
	if(!ast) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	
	ast->stmt.type = AST;
	struct statement_list** next = &ast->body;
	
	while(tokens->type != TOKEN_END) {
		
		struct statement* stmt = parse_statement(&tokens);
		*next = malloc(sizeof(struct statement_list));
		if(!*next) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		(*next)->statement = stmt;
		(*next)->next = 0;
		next = &(*next)->next;
		
	}
	
	*next = malloc(sizeof(struct statement_list) + sizeof(struct statement));
	if(!*next) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	(*next)->next = 0;
	(*next)->statement->type = AST_END;
	
	return ast;
	
}
