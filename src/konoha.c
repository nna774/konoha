#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

char const* const REGS[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

void emit_int(Ast const* ast) {
  printf("\tmov $%d, %%eax\n", ast->int_val);
}

void emit_ast(Ast const* ast, Env const* env, int depth) {
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
    emit_ast(ast->bi_op.lhs, env, depth + 1);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast(ast->bi_op.rhs, env, depth + 2);
    printf("\t%s -%d(%%rbp), %%eax\n", op, offset);
    break;
  }
  case AST_OP_MINUS:
  {
    int const offset = depth * 4;
    emit_ast(ast->bi_op.rhs, env, depth + 1);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast(ast->bi_op.lhs, env, depth + 2);
    printf("\tsub -%d(%%rbp), %%eax\n", offset);
    break;
  }
  case AST_OP_ASSIGN:
    emit_ast(ast->bi_op.rhs, env, depth + 1);
    printf("\tmov %%eax, -%d(%%rbp)\n", ast->bi_op.lhs->var->offset);
    break;
  case AST_SYM:
    printf("\tmov -%d(%%rbp), %%eax\n", ast->var->offset);
    break;
  case AST_STATEMENT:
    emit_ast(ast->statement->val, env, depth);
    break;
  case AST_STATEMENTS:
    FOREACH(Statement, ast->statements->val, s) {
      emit_ast(make_ast_statement(s), env, depth);
    }
    break;
  case AST_FUNCALL:
  {
    int const argc = ast->funcall->argc;
    char const* const name = ast->funcall->name;
    if(argc > 6) {
      warn("argc over 6 is not impled now");
      break;
    }
    for(int i = 0; i < argc; ++i) {
      emit_ast(ast->funcall->args[i], env, depth);
      printf("\tmov %%eax, %%%s\n", REGS[i]);
    }
    printf("\tcall %s\n", name);
    break;
  }
  default:
    warn("never come!!!(type: %s)\n", show_Type(t));
    break;
  }
}

void emit(Ast const* ast, Env const* env) {
  assert(ast != NULL);
  printf(
    "\t.text\n"
    "\t.global main\n"
    "main:\n"
    "\tpushq %%rbp\n"
    "\tmovq %%rsp, %%rbp\n"
  );
  emit_ast(ast, env, list_of_Var_length(env->vars) + 1);
  printf("\tmov $0, %%eax\n");
  printf("\tpopq %%rbp\n");
  printf("\tret\n");
}

int main(int argc, char** argv) {
  Env* env = new_Env();
  Ast* const ast = make_ast(env);
  if (argc > 1 && !strcmp(argv[1], "-a")) {
    print_ast(ast);
  } else if (argc > 1 && !strcmp(argv[1], "-d")) {
    printf("ast:\n");
    print_ast(ast);
    printf("\nenv:\n");
    print_env(env);
  } else {
    emit(ast, env);
  }
  return 0;
}
