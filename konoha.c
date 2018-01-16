#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

typedef enum {
  AST_INT,
  AST_OP_PLUS,
  AST_OP_MINUS,

  AST_UNKNOWN,
} Type;

struct Ast;
typedef struct Ast Ast;

typedef struct Bi_op {
  Ast* lhs;
  Ast* rhs;
} Bi_op;

struct Ast {
  Type type;
  union {
    int int_val;
    Bi_op bi_op;
  };
};

int peek(FILE* fp) {
  int c = getc(fp);
  if(c == EOF) {
    return EOF;
  }
  ungetc(c, fp);
  return c;
}

Ast* new_ast() {
  return malloc(sizeof(Ast));
}

Ast* make_ast_int(int n) {
  Ast *ast = new_ast();
  ast->type = AST_INT;
  ast->int_val = n;
  return ast;
}

Ast* make_ast_bi_op(Type t, Ast* lhs, Ast* rhs) {
  Ast *ast = new_ast();
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
  Ast* ast = parse_int(fp);
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

Type detect_bi_op(char c) {
  switch(c) {
  case '+':
    return AST_OP_PLUS;
  case '-':
    return AST_OP_MINUS;
  default:
    return AST_UNKNOWN;
  }
}

Ast* parse(FILE* fp) {
  Ast* ast = parse_prim(fp);
  skip(fp);
  int c = getc(fp);
  switch(c) {
  case EOF:
    return ast;
  case '+':
  case '-':
  {
    Type t = detect_bi_op(c);
    skip(fp);
    Ast* rhs = parse_prim(fp);
    return make_ast_bi_op(t, ast, rhs);
  }
  default:
    printf("!!!%c!!!", c);
    puts("never come!!!");
    return NULL;
  }
}

Ast* make_ast() {
  Ast* ast = parse(stdin);
  return ast;
}

void emit_int(Ast* ast) {
  printf("mov $%d, %%rax\n", ast->int_val);
}

char const * op_from_type(Type t) {
  switch(t) {
  case AST_OP_PLUS:
    return "add";
  case AST_OP_MINUS:
    return "sub";
  default:
    return ";";
  }
}

void emit_ast(Ast* ast) {
  Type t = ast->type;
  switch(t) {
  case AST_INT:
    emit_int(ast);
    break;
  case AST_OP_PLUS:
  case AST_OP_MINUS:
  {
    char const * const op = op_from_type(t);
    emit_ast(ast->bi_op.rhs);
    printf("mov %%eax, %%ebx\n");
    emit_ast(ast->bi_op.lhs);
    printf("%s %%ebx, %%eax\n", op);
    break;
  }
  default:
    puts("never come!!!");
  }
}

void emit(Ast* ast) {
  assert(ast != NULL);
  printf("\t.text\n"
         "\t.global mymain\n"
         "mymain:\n");
  emit_ast(ast);
  printf("ret\n");
}

void print_ast(Ast* ast) {
  assert(ast != NULL);
  switch(ast->type) {
  case AST_INT:
    printf("%d", ast->int_val);
    break;
  default:
    puts("never come!!!");
  }
}

int main(int argc, char** argv) {
  Ast* ast = make_ast();
  if (argc > 1 && !strcmp(argv[1], "-a")) {
    print_ast(ast);
  } else {
    emit(ast);
  }
  return 0;
}
