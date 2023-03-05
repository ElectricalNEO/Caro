#pragma once
#include "lexer.h"

enum ast_node_type {
	AST,
	NUMERIC_LITERAL,
	IDENTIFIER,
	BINARY_EXPRESSION,
	FUNCTION_DECLARATION
};

struct statement {
	enum ast_node_type type;
};

struct statement_list {
	struct statement_list* next;
	struct statement* statement;
};

struct ast {
	struct statement stmt;
	struct statement_list* body;
};

struct identifier {
	struct statement stmt;
	char symbol[0];
};

struct numeric_literal {
	struct statement stmt;
	int num;
};

enum binary_operation {
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULO
};

struct binary_expression {
	struct statement stmt;
	struct statement* left;
	struct statement* right;
	enum binary_operation operator;
};

struct function_declaration {
	struct statement stmt;
	struct statement_list* body;
	char name[0];
};

struct ast* parse(struct token* tokens);
