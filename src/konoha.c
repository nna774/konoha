#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

void emit_int(Ast const* ast) {
  printf("\tmov $%d, %%eax\n", ast->int_val);
}

void emit_ast(Ast const* ast, int depth) {
  Type const t = ast->type;
  switch(t) {
  case AST_INT:
    emit_int(ast);
    break;
  case AST_OP_PLUS:
  case AST_OP_MULTI:
  {
    char const * const op = op_from_type(t);
    int const offset = depth * 4;
    emit_ast(ast->bi_op.lhs, depth + 1);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast(ast->bi_op.rhs, depth + 2);
    printf("\t%s -%d(%%rbp), %%eax\n", op, offset);
    break;
  }
  case AST_OP_MINUS:
    emit_ast(ast->bi_op.rhs, depth + 1);
    printf("\tmov %%eax, %%ebx\n");
    emit_ast(ast->bi_op.lhs, depth + 2);
    printf("\tsub %%ebx, %%eax\n");
    break;
  default:
    warn("never come!!!(type: %d)\n", t);
  }
}

void emit(Ast const* ast) {
  assert(ast != NULL);
  printf(
    "\t.text\n"
    "\t.global mymain\n"
    "mymain:\n"
    "\tpushq %%rbp\n"
    "\tmovq %%rsp, %%rbp\n"
  );
  emit_ast(ast, 1);
  printf("\tpopq %%rbp\n");
  printf("\tret\n");
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
