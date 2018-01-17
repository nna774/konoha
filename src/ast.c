#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "ast.h"
#include "utils.h"

char const * op_from_type(Type t) {
  switch(t) {
  case AST_OP_PLUS:
    return "add";
  case AST_OP_MINUS:
    return "sub";
  case AST_OP_MULTI:
    return "imul";
  default:
    warn("wrong type\n");
    return ";";
  }
}

Ast* new_ast() {
  return malloc(sizeof(Ast));
}

Ast* make_ast_int(int n) {
  Ast* const ast = new_ast();
  ast->type = AST_INT;
  ast->int_val = n;
  return ast;
}

Ast* make_ast_bi_op(Type const t, Ast const* lhs, Ast const* rhs) {
  Ast* const ast = new_ast();
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

Ast* parse_prim(FILE* fp) {
  Ast* const ast = parse_int(fp);
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
    warn("unknown bi-op");
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

Ast* parse(FILE* fp, int prio) {
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
      Ast* const rhs = parse(fp, c_prio + 1);
      ast = make_ast_bi_op(t, lhs, rhs);
      break;
    }
    default:
      warn("never come!!!\n");
      return NULL;
    }
  }
  return NULL; // never come
}

Ast* make_ast() {
  int const prio = 0;
  Ast* const ast = parse(stdin, prio);
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
  default:
    warn("never come!!!\n");
  }
}

