#ifndef NNA774_KONOHA_AST_H
#define NNA774_KONOHA_AST_H

typedef enum {
  AST_INT,
  AST_OP_PLUS,
  AST_OP_MINUS,
  AST_OP_MULTI,

  AST_UNKNOWN,
} Type;

struct Ast;
typedef struct Ast Ast;

typedef struct Bi_op {
  Ast const* lhs;
  Ast const* rhs;
} Bi_op;

struct Ast {
  Type type;
  union {
    int int_val;
    Bi_op bi_op;
  };
};

Ast* make_ast();
void print_ast(Ast const* ast);
char const * op_from_type(Type t);

#endif // NNA774_KONOHA_AST_H
