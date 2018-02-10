#ifndef NNA774_KONOHA_AST_H
#define NNA774_KONOHA_AST_H

#include "enum.h"
#include "tokenize.h"
#include "list.h"

ENUM_WITH_SHOW(
  AstType,
  AST_INT,
  AST_OP_PLUS,
  AST_OP_MINUS,
  AST_OP_MULTI,
  AST_OP_DIV,
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
  AST_GLOBAL,

  AST_EMPTY,
  AST_UNKNOWN,
)

ENUM_WITH_SHOW(
  StatementType,
  NORMAL_STATEMENT,
  RETURN_STATEMENT,
  IF_STATEMENT,
  WHILE_STATEMENT,
)

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
struct Global;
typedef struct Global Global;

DEFINE_INTRUSIVE_LIST(Type);
DEFINE_INTRUSIVE_LIST(Var);
DEFINE_INTRUSIVE_LIST(Statement);
DEFINE_INTRUSIVE_LIST(Ast);

typedef struct Bi_op {
  Ast const* lhs;
  Ast const* rhs;
} Bi_op;

struct Type {
  char const* name;
  INTRUSIVE_LIST_HOOK(Type);
};

struct Var {
  char const* name;
  Type* type;
  bool initialized;
  bool defined;
  int offset;
  INTRUSIVE_LIST_HOOK(Var);
};

struct Env {
  Env* parent;
  INTRUSIVE_LIST_OF(Var) vars;
  INTRUSIVE_LIST_OF(Type) types;
};

struct Statement {
  StatementType type;
  union {
    Ast* val;
    struct {
      Ast* cond;
      Statement* body;
      Statement* else_body;
    } if_val;
    struct {
      Ast* cond;
      Statement* body;
    } while_val;
  };
  INTRUSIVE_LIST_HOOK(Statement);
};

struct Statements {
  INTRUSIVE_LIST_OF(Statement) val;
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

struct Global {
  INTRUSIVE_LIST_OF(Ast) list;
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
    Global* global;
  };
  INTRUSIVE_LIST_HOOK(Ast);
};

Env* new_Env();
Type* new_Type(char const* name);
Ast* make_ast(Env*, Tokens);
Ast* make_ast_statement(Statement*);
void print_ast(Ast const*);
void print_env(Env const*);
char const * op_from_type(AstType t);

#endif // NNA774_KONOHA_AST_H
