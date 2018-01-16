#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  AST_INT,
} Types;

typedef struct {
  Types type;
  union {
    int int_val;
  };
} Ast;

int peek(FILE* fp) {
  int c = getc(fp);
  if(c == EOF) {
    return EOF;
  }
  ungetc(c, fp);
  return c;
}

Ast* make_ast_int(int n) {
  Ast *ast = malloc(sizeof(Ast));
  ast->type = AST_INT;
  ast->int_val = n;
  return ast;
}

Ast* parse_int(FILE* fp) {
  int sum = 0;
  int c;
  while(c = peek(fp), isdigit(c)) {
    sum  = sum * 10 + (c - '0');
    getc(fp);
  }
  return make_ast_int(sum);
}

Ast* make_ast() {
  Ast* ast = parse_int(stdin);
  return ast;
}

void emit(Ast* ast) {
  printf("\t.text\n"
         "\t.global mymain\n"
         "mymain:\n"
         "\tmov $%d, %%eax\n"
         "ret\n", ast->int_val);
}

void print_ast(Ast* ast) {
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
