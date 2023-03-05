#include "parser.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct token* consume_token(struct token** tokens) {
	
	struct token* tok = *tokens;
	*tokens = (struct token*)((size_t)(*tokens) + (*tokens)->size + sizeof(struct token));
	return tok;
	
}

struct statement* parse_expression(struct token** tokens);

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
	case TOKEN_IDENTIFIER: {
		struct token* token = consume_token(tokens);
		struct identifier* stmt = malloc(sizeof(struct identifier) + strlen(token->data) + 1);
		if(!stmt) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		stmt->stmt.type = IDENTIFIER;
		memcpy(stmt->symbol, token->data, strlen(token->data) + 1);
		return (struct statement*)stmt;
	}
	case TOKEN_PUNCTUATOR:
		if((*tokens)->data[0] != '(') return 0;
		consume_token(tokens);
		struct statement* stmt = parse_expression(tokens);
		struct token* tok = consume_token(tokens);
		if(tok->type != TOKEN_PUNCTUATOR || tok->data[0] != ')') {
			fprintf(stderr, "Unclosed parenthesis in line %d!\n", tok->line);
			exit(1);
		}
		return stmt;
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

struct statement* parse_statement(struct token** tokens, const char* func_prefix);

struct statement_list* parse_block(struct token** tokens, const char* func_prefix) {
	
	struct statement_list* list;
	struct statement_list** next = &list;
	
	struct token* open_brace = consume_token(tokens);
	if(open_brace->type != TOKEN_PUNCTUATOR || open_brace->data[0] != '{') {
		fprintf(stderr, "E: Opening brace expected in line %d!\n", open_brace->line);
		exit(1);
	}
	
	while((*tokens)->type != TOKEN_PUNCTUATOR && (*tokens)->data[0] != '}') {
		
		struct statement* stmt = parse_statement(tokens, func_prefix);
		*next = malloc(sizeof(struct statement_list));
		if(!*next) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		(*next)->statement = stmt;
		(*next)->next = 0;
		next = &(*next)->next;
		
	}
	consume_token(tokens); // consume closing brace
	
	*next = malloc(sizeof(struct statement_list));
	if(!*next) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	(*next)->next = 0;
	
	return list;
	
}

struct statement* parse_function_declaration(struct token** tokens, const char* prefix) {
	
	consume_token(tokens); // FN keyword
	if((*tokens)->type != TOKEN_IDENTIFIER) {
		fprintf(stderr, "E: Expected identifier (function name) in line %d!\n", (*tokens)->line);
		exit(1);
	}
	
	struct token* name = consume_token(tokens); // function name
	
	struct token* open_paren = consume_token(tokens);
	if(open_paren->type != TOKEN_PUNCTUATOR || open_paren->data[0] != '(') {
		fprintf(stderr, "E: Expected opening parenthesis after function name in line %d!\n", (*tokens)->line);
		exit(1);
	}
	
	/// TODO: function arguments
	
	struct token* close_paren = consume_token(tokens);
	if(close_paren->type != TOKEN_PUNCTUATOR || close_paren->data[0] != ')') {
		fprintf(stderr, "E: Expected closing parenthesis after function name in line %d!\n", (*tokens)->line);
		exit(1);
	}
	
	int prefixed = 0;
	if(strlen(prefix)) prefixed = 1;
	
	struct function_declaration* stmt = malloc(sizeof(struct function_declaration) + strlen(name->data) + strlen(prefix) + 1 + prefixed);
	if(!stmt) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	
	stmt->stmt.type = FUNCTION_DECLARATION;
	memcpy(stmt->name, prefix, strlen(prefix));
	if(prefixed) stmt->name[strlen(prefix)] = '_';
	memcpy(stmt->name + strlen(prefix) + prefixed, name->data, strlen(name->data) + 1);
	stmt->body = parse_block(tokens, stmt->name);
	
	return (struct statement*)stmt;
	
}

struct statement* parse_statement(struct token** tokens, const char* func_prefix) {
	
	if((*tokens)->type == TOKEN_KEYWORD && *(enum keyword*)((*tokens)->data) == KEYWORD_FN) return parse_function_declaration(tokens, func_prefix);
	
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
		
		struct statement* stmt = parse_statement(&tokens, "");
		*next = malloc(sizeof(struct statement_list));
		if(!*next) {
			fprintf(stderr, "E: Failed to allocate memory!\n");
			exit(1);
		}
		(*next)->statement = stmt;
		(*next)->next = 0;
		next = &(*next)->next;
		
	}
	
	*next = malloc(sizeof(struct statement_list));
	if(!*next) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	(*next)->next = 0;
	
	return ast;
	
}
