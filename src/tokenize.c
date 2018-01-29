#include <ctype.h>
#include <string.h>

#include "tokenize.h"
#include "utils.h"

bool is_identifier_char(char c) {
  return isalnum(c) || c == '_';
}

bool is_open_paren(char c) {
  return c == '(' ||  c == '{';
}

bool is_close_paren(char c) {
  return c == ')' ||  c == '}';
}

Token* new_Token(String str, TokenType ty) {
  Token* t = malloc(sizeof(Token));
  t->string = str;
  t->type = ty;
  init_Token_hook(t);
  return t;
}

Token* read_identifier(FILE* fp) {
  int c;
  String str = new_String();
  while(c = getc(fp), is_identifier_char(c)) {
    append_char(str, c);
  }
  ungetc(c, fp);
  return new_Token(str, IDENTIFIER_T);
}

Token* read_integer(FILE* fp) {
  int c;
  String str = new_String();
  while(c = getc(fp), isdigit(c)) {
    append_char(str, c);
  }
  ungetc(c, fp);
  return new_Token(str, INTEGER_LITERAL_T);
}

Token* read_paren_impl(FILE* fp, bool open) {
  int c = getc(fp);
  assert(is_open_paren(c) || is_close_paren(c));
  return new_Token(from_char(c), open ? OPEN_PAREN_T : CLOSE_PAREN_T);
}

Token* read_open_paren(FILE* fp) {
  return read_paren_impl(fp, true);
}

Token* read_close_paren(FILE* fp) {
  return read_paren_impl(fp, false);
}

Token* read_token(FILE* fp) {
  int c = peek(fp);
  Token* t = NULL;
  if(is_identifier_char(c)) {
    t = read_identifier(fp);
  } else if(isdigit(c)){
    t = read_integer(fp);
  } else if(is_open_paren(c)) {
    t = read_open_paren(fp);
  } else if(is_close_paren(c)) {
    t = read_close_paren(fp);
  } else if(c == ',') {
    getc(fp);
    t = new_Token(from_char(','), COMMA_T);
  } else if(c == ';') {
    getc(fp);
    t = new_Token(from_char(';'), SEMICOLON_T);
  } else {
    printf("got %s", show_char(c));
    puts("here");
  }
  assert(t != NULL);
  return t;
}

INTRUSIVE_LIST_OF(Token) tokenize(FILE* fp) {
  INTRUSIVE_LIST_OF(Token) tokens = new_list_of_Token();
  int c;
  skip(fp);
  while(c = peek(fp), c != EOF) {
    Token* t = read_token(fp);
    assert(t != NULL);
    list_of_Token_append(tokens, t);
    skip(fp);
  }
  return tokens;
}

Token pop_Token(Tokens ts) {
  return *list_of_Token_pop(ts);
}

Token peek_Token(Tokens ts) {
  return *(ts->head);
}

void print_Token(Token const* t) {
  printf("%s: %s\n", show_TokenType(t->type), c_str(t->string));
}

void print_Tokens(INTRUSIVE_LIST_OF(Token) ts) {
  FOREACH(Token, ts, t) {
    print_Token(t);
  }
}