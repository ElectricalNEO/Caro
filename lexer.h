#pragma once
#include <stdio.h>

enum keyword {
	KEYWORD_INVALID,
	KEYWORD_FN
};

enum token_type {
	TOKEN_KEYWORD, // e.g. fn
	TOKEN_STRING_LITERAL, // e.g. "Hello world!\n"
	TOKEN_INTEGER_LITERAL, // e.g. 15
	TOKEN_PUNCTUATOR, // e.g. {
	TOKEN_OPERATOR, // e.g. +
	TOKEN_IDENTIFIER, // e.g. main
	TOKEN_END // end of the token list
};

struct token {
	enum token_type type;
	int size;
	int line;
	char data[0];
};

struct token* tokenize(const char* source);
