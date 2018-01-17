#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "ast.h"
#include "utils.h"
#define DEFINE_NEW(T) \
  T* new_ ## T() {\
    return malloc(sizeof(T));\
  }

DEFINE_NEW(Ast);

Env* new_Env() {
  Env* e = malloc(sizeof(Env));
  e->parent = NULL;
  e->vars = new_list_of_Var();
  return e;
}

Var* new_Var() {
  Var* v = malloc(sizeof(Var));
  v->name = NULL;
  init_Var_hook(v);
  return v;
}

int const MAX_BUF_LEN = 256;

char const * op_from_type(Type t) {
  switch(t) {
  case AST_OP_PLUS:
    return "add";
  case AST_OP_MINUS:
    return "sub";
  case AST_OP_MULTI:
    return "imul";
  default:
    warn("wrong type(%d)\n", t);
    return ";";
  }
}

Ast* make_ast_int(int n) {
  Ast* const ast = new_Ast();
  ast->type = AST_INT;
  ast->int_val = n;
  return ast;
}

Ast* make_ast_symbol(char const* buf) {
  Ast* const ast = new_Ast();
  ast->type = AST_SYM;
  ast->var = new_Var();
  ast->var->name = buf;
  return ast;
}

Ast* make_ast_bi_op(Type const t, Ast const* lhs, Ast const* rhs) {
  Ast* const ast = new_Ast();
  ast->type = t;
  ast->bi_op.lhs = lhs;
  ast->bi_op.rhs = rhs;
  return ast;
}

Ast* parse_int(FILE* fp) {
  int sum = 0;
  int c;
  while(c = getc(fp), isdigit(c)) {
    sum  = sum * 10 + (c - '0');
  }
  ungetc(c, fp);
  return make_ast_int(sum);
}

Ast* parse_symbol(FILE* fp) {
  char* const buf = malloc(MAX_BUF_LEN);
  int c;
  int at = 0;
  while(c = getc(fp), isalnum(c)) {
    if (at >= MAX_BUF_LEN - 1) {
      warn("symbol too long! truncated!\n");
      break;
    }
    buf[at++] = c;
  }
  ungetc(c, fp);
  buf[at] = '\0';
  return make_ast_symbol(buf);
}

Ast* parse_prim(FILE* fp) {
  int const c = peek(fp);
  Ast* ast;
  if(isdigit(c)) {
    ast = parse_int(fp);
  } else if(isalpha(c)) {
    ast = parse_symbol(fp);
  } else {
    warn("unknown char: %c\n", c);
    ast = NULL;
  }
  return ast;
}

void skip(FILE* fp) {
  int c;
  while(c = getc(fp), c != EOF) {
    if(!isspace(c)) {
      break;
    }
  }
  ungetc(c, fp);
}

int priority(char op) {
  switch (op) {
  case '+':
  case '-':
    return 1;
  case '*':
    return 2;
  default:
    warn("unknown bi-op(got: %c)", op);
    return -1;
  }
}

Type detect_bi_op(char c) {
  switch(c) {
  case '+':
    return AST_OP_PLUS;
  case '-':
    return AST_OP_MINUS;
  case '*':
    return AST_OP_MULTI;
  default:
    return AST_UNKNOWN;
  }
}

Ast* parse(FILE* fp, Env* env, int prio) {
  Ast* ast = parse_prim(fp);
  while(true) {
    skip(fp);
    int const c = getc(fp);
    switch(c) {
    case EOF:
      return ast;
    case '+':
    case '-':
    case '*':
    {
      int const c_prio = priority(c);
      if(c_prio < prio) {
        ungetc(c, fp);
        return ast;
      }
      Type const t = detect_bi_op(c);
      skip(fp);
      Ast* const lhs = ast;
      Ast* const rhs = parse(fp, env, c_prio + 1);
      ast = make_ast_bi_op(t, lhs, rhs);
      break;
    }
    case '=':
    {
      list_of_Var_append(env->vars, ast->var);
      skip(fp);
      Ast* const lhs = ast;
      Ast* const rhs = parse(fp, env, prio);
      ast = make_ast_bi_op(AST_OP_ASSIGN, lhs, rhs);
      break;
    }
    default:
      warn("never come!!!(got: %c)\n", c);
      return NULL;
    }
  }
  warn("reached to unreachable path");
  return NULL; // never come
}

Ast* make_ast() {
  int const prio = 0;
  Env* env = new_Env();
  Ast* const ast = parse(stdin, env, prio);
  return ast;
}

void print_ast(Ast const* ast) {
  assert(ast != NULL);
  Type const t = ast->type;
  switch(t) {
  case AST_INT:
    printf("%d", ast->int_val);
    break;
  case AST_OP_PLUS:
  case AST_OP_MINUS:
  case AST_OP_MULTI:
    printf("(%s ", op_from_type(t));
    print_ast(ast->bi_op.lhs);
    printf(" ");
    print_ast(ast->bi_op.rhs);
    printf(")");
    break;
  case AST_SYM:
    printf("%s", ast->var->name);
    break;
  case AST_OP_ASSIGN:
    printf("(let ");
    print_ast(ast->bi_op.lhs);
    printf(" ");
    print_ast(ast->bi_op.rhs);
    printf(")");
    break;
  default:
    warn("never come!!!(type: %d)\n", t);
  }
}

