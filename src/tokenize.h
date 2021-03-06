#ifndef NNA774_KONOHA_TOKENIZE_H
#define NNA774_KONOHA_TOKENIZE_H

#include "string.h"
#include "list.h"
#include "enum.h"

ENUM_WITH_SHOW(
  TokenType,
  IDENTIFIER_T,
  INTEGER_LITERAL_T,
  CHARACTER_LITERAL_T,
  OPEN_PAREN_T,
  CLOSE_PAREN_T,
  OP_PLUS_T,
  OP_MINUS_T,
  OP_MULTI_T,
  OP_DIV_T,
  OP_INC_T,
  OP_DEC_T,
  OP_EQUAL_T,
  OP_ASSIGN_T,
  SEMICOLON_T,
  COMMA_T,
  KEYWORD_T,
  EOF_T,

  COMMENT_T,
  UNKNOWN_T,
)

struct Token;
typedef struct Token Token;

struct Token {
  String string;
  TokenType type;
  INTRUSIVE_LIST_HOOK(Token);
};

DEFINE_INTRUSIVE_LIST(Token);

typedef INTRUSIVE_LIST_OF(Token) Tokens;

Tokens tokenize(FILE*);
Token pop_Token(Tokens);
void push_Token(Tokens, Token);
Token peek_Token(Tokens);
void print_Token(Token const*);
void print_Tokens(INTRUSIVE_LIST_OF(Token));
TokenType to_TokenType(char const* str);
bool is_op(TokenType t);

#endif // NNA774_KONOHA_TOKENIZE_H
