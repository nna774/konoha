#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

int const true = 1;
int const false = 0;

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

int peek(FILE* fp) {
  int const c = getc(fp);
  if(c == EOF) {
    return EOF;
  }
  ungetc(c, fp);
  return c;
}

void warn(char const* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
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

Ast* parse(FILE* fp) {
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
      Type const t = detect_bi_op(c);
      skip(fp);
      Ast* const lhs = ast;
      Ast* const rhs = parse_prim(fp);
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
  Ast* const ast = parse(stdin);
  return ast;
}

void emit_int(Ast const* ast) {
  printf("\tmov $%d, %%rax\n", ast->int_val);
}

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

void emit_ast(Ast const* ast) {
  Type const t = ast->type;
  switch(t) {
  case AST_INT:
    emit_int(ast);
    break;
  case AST_OP_PLUS:
  case AST_OP_MULTI:
  {
    char const * const op = op_from_type(t);
    emit_ast(ast->bi_op.lhs);
    printf("\tmov %%eax, %%ebx\n");
    emit_ast(ast->bi_op.rhs);
    printf("\t%s %%ebx, %%eax\n", op);
    break;
  }
  case AST_OP_MINUS:
    emit_ast(ast->bi_op.rhs);
    printf("\tmov %%eax, %%ebx\n");
    emit_ast(ast->bi_op.lhs);
    printf("\tsub %%ebx, %%eax\n");
    break;
  default:
    warn("never come!!!\n");
  }
}

void emit(Ast const* ast) {
  assert(ast != NULL);
  printf("\t.text\n"
         "\t.global mymain\n"
         "mymain:\n");
  emit_ast(ast);
  printf("\tret\n");
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

int main(int argc, char** argv) {
  Ast* const ast = make_ast();
  if (argc > 1 && !strcmp(argv[1], "-a")) {
    print_ast(ast);
  } else {
    emit(ast);
  }
  return 0;
}
