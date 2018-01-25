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
  AST_SYM_DECLER,
  AST_SYM_DEFINE,
  AST_STATEMENT,
  AST_STATEMENTS,
  AST_FUNCALL,
  AST_FUNDECLAR,
  AST_FUNDEFIN,
  AST_BLOCK,

  AST_EMPTY,
  AST_UNKNOWN = 999,
} AstType;

struct Ast;
typedef struct Ast Ast;
struct Var;
typedef struct Var Var;
struct Env;
typedef struct Env Env;
struct Statement;
typedef struct Statement Statement;
struct Statements;
typedef struct Statements Statements;
struct Block;
typedef struct Block Block;
struct FunCall;
typedef struct FunCall FunCall;
struct Type;
typedef struct Type Type;
struct FunType;
typedef struct FunType FunType;
struct FunDef;
typedef struct FunDef FunDef;

DEFINE_LIST(Type);
DEFINE_LIST(Var);
DEFINE_LIST(Statement);

typedef struct Bi_op {
  Ast const* lhs;
  Ast const* rhs;
} Bi_op;

struct Type {
  char const* name;
};

struct Var {
  char const* name;
  Type* type;
  bool initialized;
  bool defined;
  int offset;
};

struct Env {
  Env const* parent;
  LIST_OF(Var) vars;
  LIST_OF(Type) types;
};

struct Statement {
  Ast* val;
};

struct Statements {
  LIST_OF(Statement) val;
};

struct Block {
  Ast* val;
  Env* env;
};

struct FunCall {
  char const* name;
  int argc;
  Ast** args;
};

struct FunType {
  Type const* return_type;
  int argc;
  Type** arg_types;
};

struct FunDef {
  FunType type;
  char const* name;
  Var** args;
  Ast* body;
};

struct Ast {
  AstType type;
  union {
    int int_val;
    Bi_op bi_op;
    Var* var;
    Statement* statement;
    Statements* statements;
    FunCall* funcall;
    Block* block;
    FunDef* fundef;
  };
};

Env* new_Env();
Type* new_Type(char const* name);
Ast* make_ast(Env*);
Ast* make_ast_statement(Statement*);
void print_ast(Ast const*);
void print_env(Env const*);
char const * op_from_type(AstType t);
char const* show_AstType(AstType t);

#endif // NNA774_KONOHA_AST_H
