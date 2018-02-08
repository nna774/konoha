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

bool is_operator_char(char c) {
  char const operators[] = "+-*/=";
  int const coo = sizeof(operators) - 1;
  for(int i = 0; i < coo; ++i) {
    if(c == operators[i]) {
      return true;
    }
  }
  return false;
}

bool is_keyword(String str_) {
  char const* const str = c_str(str_);
  char const* const keywords[] = {
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
  };
  for(int i = 0; i < (int)(sizeof(keywords) / sizeof(*keywords)); ++i) {
    if(strcmp(str, keywords[i])) {
      return true;
    }
  }
  return false;
}

Token* new_Token(String str, TokenType ty) {
  Token* t = malloc(sizeof(Token));
  t->string = str;
  t->type = ty;
  init_Token_hook(t);
  return t;
}

Token* copy_Token(Token _t) {
  Token* t = malloc(sizeof(Token));
  t->string = _t.string;
  t->type = _t.type;
  init_Token_hook(t);
  return t;
}

Token* read_identifier(FILE* fp) {
  int c;
  String str = new_String();
  while(c = getc(fp), is_identifier_char(c)) {
    append_char(str, c);
  }
  if(is_keyword(str)) {
    return new_Token(str, KEYWORD_T);
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

Token* read_operator_and_comment(FILE* fp) {
  int const c = getc(fp);
  if(c == '/') {
    int const next = peek(fp);
    if(next == '/') {
      //
      int c;
      while(c = getc(fp), c != '\n');
      return new_Token(from_char('/'), COMMENT_T);
    } else if(next == '*') {
      /* */
      int c;
      while(true) {
        while(c = getc(fp), c != '*');
        if(c = getc(fp), c == '/') {
          break;
        }
      }
      return new_Token(from_char('*'), COMMENT_T);
    }
  }
  return new_Token(from_char(c), OP_T);
}

Token* read_token(FILE* fp) {
  int const c = peek(fp);
  Token* t = NULL;
  if(isdigit(c)) {
    t = read_integer(fp);
  } else if(is_identifier_char(c)){
    t = read_identifier(fp);
  } else if(is_open_paren(c)) {
    t = read_open_paren(fp);
  } else if(is_close_paren(c)) {
    t = read_close_paren(fp);
  } else if(is_operator_char(c)) {
    t = read_operator_and_comment(fp);
  } else if(c == ',') {
    getc(fp);
    t = new_Token(from_char(','), COMMA_T);
  } else if(c == ';') {
    getc(fp);
    t = new_Token(from_char(';'), SEMICOLON_T);
  } else {
    printf("got %s\n", show_char(c));
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
    if(t->type != COMMENT_T) {
      list_of_Token_append(tokens, t);
    }
    skip(fp);
  }
  Token* eof_t = new_Token(new_String(), EOF_T);
  list_of_Token_append(tokens, eof_t);
  return tokens;
}

Token pop_Token(Tokens ts) {
  return *list_of_Token_pop(ts);
}

void push_Token(Tokens ts, Token t) {
  list_of_Token_push(ts, copy_Token(t));
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
