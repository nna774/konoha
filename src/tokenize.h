#ifndef NNA774_KONOHA_TOKENIZE_H
#define NNA774_KONOHA_TOKENIZE_H

#include "string.h"
#include "list.h"
#include "enum.h"

ENUM_WITH_SHOW(
  TokenType,
  IDENTIFIER_T,
  INTEGER_LITERAL_T,
  OPEN_PAREN_T,
  CLOSE_PAREN_T,
  UNARY_OP_T,
  BI_OP_T,
  SEMICOLON_T,
  COMMA_T,

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
Token peek_Token(Tokens);
void print_Token(Token const*);
void print_Tokens(INTRUSIVE_LIST_OF(Token));

#endif // NNA774_KONOHA_TOKENIZE_H
