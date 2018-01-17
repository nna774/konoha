#ifndef NNA774_KONOHA_AST_H
#define NNA774_KONOHA_AST_H

#include "list.h"

typedef enum {
  AST_INT,
  AST_OP_PLUS,
  AST_OP_MINUS,
  AST_OP_MULTI,
  AST_OP_ASSIGN,
  AST_SYM,

  AST_UNKNOWN = 999,
} Type;

struct Ast;
typedef struct Ast Ast;
struct Var;
typedef struct Var Var;
struct Env;
typedef struct Env Env;

DEFINE_INTRUSIVE_LIST(Var);

typedef struct Bi_op {
  Ast const* lhs;
  Ast const* rhs;
} Bi_op;

struct Var {
  char const* name;
  INTRUSIVE_LIST_HOOK(Var);
};

struct Env {
  Env const* parent;
  INTRUSIVE_LIST_OF(Var) vars;
};

struct Ast {
  Type type;
  union {
    int int_val;
    Bi_op bi_op;
    Var* var;
  };
};

Ast* make_ast();
void print_ast(Ast const* ast);
char const * op_from_type(Type t);

#endif // NNA774_KONOHA_AST_H
