#include <stdlib.h>
#include <string.h>
#include "lexer.h"

int is_whitespace(char ch) {
	return (ch == ' ' || ch == '\t' || ch == '\n');
}

int is_letter(char ch) { // underscores also count as letters, its just easier this way
	return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_');
}

int is_digit(char ch) {
	return (ch >= '0' && ch <= '9');
}

int is_punctuator(char ch) {
	
	switch(ch) {
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
	case ';':
	case ',':
		return 1;
	default:
		return 0;
	}
	
}

int is_operator(char ch) {
	
	switch(ch) {
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '=':
	case '!':
	case '<':
	case '>':
		return 1;
	default:
		return 0;
	}
	
}

enum keyword get_keyword(const char* start, int size) {
	if(size == 2 && memcmp(start, "fn", 2)) return KEYWORD_FN;
	if(size == 6 && memcmp(start, "return", 6)) return KEYWORD_RETURN;
	return KEYWORD_INVALID;
}

int check_integer_literal(const char* start) {
	
	int base = 10;
	
	if(start[0] == '0') {
		if(start[1] == 'b') {
			base = 2;
			start += 2;
		} else if(start[1] == 'o') {
			base = 8;
			start += 2;
		} else if(start[1] == 'x') {
			base = 16;
			start += 2;
		}
	}
	
	for(int i = 0; is_digit(start[i]) || is_letter(start[i]); i++) {
		if(base == 2 && (start[i] < '0' || start[i] > '1')) return 0;
		if(base == 8 && (start[i] < '0' || start[i] > '7')) return 0;
		if(base == 10 && (start[i] < '0' || start[i] > '9')) return 0;
		if(base == 16 && ((start[i] < '0' || start[i] > '9') && (start[i] < 'a' || start[i] > 'f') && (start[i] < 'A' || start[i] > 'F'))) return 0;
	}
	
	return 1;
	
}

int parse_integer_literal(const char* start) {
	
	int ret = 0;
	int base = 10;
	
	if(start[0] == '0') {
		if(start[1] == 'b') {
			base = 2;
			start += 2;
		} else if(start[1] == 'o') {
			base = 8;
			start += 2;
		} else if(start[1] == 'x') {
			base = 16;
			start += 2;
		}
	}
	
	for(int i = 0; is_digit(start[i]) || is_letter(start[i]); i++) {
		
		ret *= base;
		if(start[i] >= '0' && start[i] <= '9') ret += start[i] - '0';
		else if(start[i] >= 'a' && start[i] <= 'f') ret += start[i] - 'a' + 0xa;
		else ret += start[i] - 'A' + 0xa;
		
	}
	
	return ret;
	
}

struct token* tokenize(const char* source) {
	
	int line = 1;
	size_t buf_size = sizeof(struct token);
	
	for(int i = 0; source[i];) {
		
		if(is_whitespace(source[i])) {
			if(source[i] == '\n') line++;
			i++;
			continue;
		}
		if(is_letter(source[i])) {
			int start = i;
			while(is_letter(source[i]) || is_digit(source[i])) i++;
			if(get_keyword(&source[i], i - start) != KEYWORD_INVALID) buf_size += sizeof(struct token) + sizeof(enum keyword);
			else buf_size += sizeof(struct token) + i - start + 1; 
			continue;
		}
		if(is_digit(source[i])) {
			if(!check_integer_literal(&source[i])) {
				fprintf(stderr, "E: Invalid integer literal in line %d!\n", line);
				exit(1);
			}
			while(is_letter(source[i]) || is_digit(source[i])) i++;
			buf_size += sizeof(struct token) + sizeof(int);
			continue;
		}
		if(is_punctuator(source[i])) {
			i++;
			buf_size += sizeof(struct token) + 1;
			continue;
		}
		if(is_operator(source[i])) {
			int start = i;
			while(is_operator(source[i])) i++;
			buf_size += sizeof(struct token) + i - start + 1;
			continue;
		}
		if(source[i] == '"') {
			i++;
			while(1) {
				if(source[i] == '\\') { // ESCAPE SEQUENCES
					i++;
					switch(source[i]) {
					case 'n':
					case 't':
					case '\\':
					case '"':
						buf_size++;
						i++;
						break;
					default:
						fprintf(stderr, "E: Invalid escape sequence in line %d!\n", line);
						exit(1);
					}
					continue;
				}
				if(source[i] == 0 || source[i] == '\n') { // END
					fprintf(stderr, "Unclosed string literal in line %d!\n", line);
					exit(1);
				} else if(source[i] == '"') { // CLOSE
					break;
				}
				buf_size++;
				i++;
			}
			buf_size += sizeof(struct token) + 1;
			i++;
			continue;
		}
		if(source[i] == '#') {
			while(source[i] != '\n' && source[i]) i++;
			continue;
		}
		fprintf(stderr, "E: Invalid token in line %d: %c\n", line, source[i]);
		exit(1);
		
	}
	
	struct token* tokens = malloc(buf_size);
	if(!tokens) {
		fprintf(stderr, "E: Failed to allocate memory!\n");
		exit(1);
	}
	struct token* token = tokens;
	line = 1;
	
	for(int i = 0; source[i];) {
		
		if(is_whitespace(source[i])) {
			if(source[i] == '\n') line++;
			i++;
			continue;
		}
		if(is_letter(source[i])) {
			int start = i;
			while(is_letter(source[i]) || is_digit(source[i])) i++;
			enum keyword keyword = get_keyword(&source[i], i - start);
			if(keyword != KEYWORD_INVALID) {
				token->type = TOKEN_KEYWORD;
				token->size = sizeof(enum keyword);
				*(enum keyword*)token->data = keyword;
			}
			else {
				token->type = TOKEN_IDENTIFIER;
				token->size = i - start + 1;
				memcpy(token->data, &source[start], i - start);
				token->data[i - start] = 0;
			}
			token->line = line;
			token = (struct token*)((size_t)token + sizeof(struct token) + token->size);
			continue;
		}
		if(is_digit(source[i])) {
			int start = i;
			while(is_letter(source[i]) || is_digit(source[i])) i++;
			token->type = TOKEN_INTEGER_LITERAL;
			token->size = sizeof(int);
			*(int*)token->data = parse_integer_literal(&source[start]);
			token->line = line;
			token = (struct token*)((size_t)token + sizeof(struct token) + token->size);
			continue;
		}
		if(is_punctuator(source[i])) {
			token->type = TOKEN_PUNCTUATOR;
			token->size = 1;
			token->data[0] = source[i];
			i++;
			token->line = line;
			token = (struct token*)((size_t)token + sizeof(struct token) + token->size);
			continue;
		}
		if(is_operator(source[i])) {
			int start = i;
			while(is_operator(source[i])) i++;
			token->type = TOKEN_OPERATOR;
			token->size = i - start + 1;
			memcpy(token->data, &source[start], i - start);
			token->data[i - start] = 0;
			token->line = line;
			token = (struct token*)((size_t)token + sizeof(struct token) + token->size);
			continue;
		}
		if(source[i] == '"') {
			token->type = TOKEN_STRING_LITERAL;
			int j = 0;
			i++;
			while(1) {
				if(source[i] == '\\') { // ESCAPE SEQUENCES
					i++;
					switch(source[i]) {
					case 'n':
						token->data[j] = '\n';
						j++;
						i++;
						break;
					case 't':
						token->data[j] = '\t';
						j++;
						i++;
						break;
					case '\\':
						token->data[j] = '\\';
						j++;
						i++;
						break;
					case '"':
						token->data[j] = '"';
						j++;
						i++;
						break;
					}
					continue;
				}
				if(source[i] == '"') { // CLOSE
					break;
				}
				token->data[j] = source[i];
				j++;
				i++;
			}
			token->data[j] = 0;
			token->size = j;
			i++;
			token->line = line;
			token = (struct token*)((size_t)token + sizeof(struct token) + token->size);
			continue;
		}
		if(source[i] == '#') {
			while(source[i] != '\n' && source[i]) i++;
			continue;
		}
		
	}
	
	token->type = TOKEN_END;
	token->size = 0;
	token->line = line;
	
	return tokens;
	
}
