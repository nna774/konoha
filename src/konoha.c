#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "tokenize.h"
#include "utils.h"

char const* const REGS[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
int const MAX_REG_LEN = 16;
int const MAX_OP_LEN = 16;

void save_regs(int argc, int depth) {
  for(int i = 0; i < argc; ++i) {
    int const offset = (depth + i) * 4;
    printf("\tmov %%%s, -%d(%%rbp)\n", REGS[i], offset);
  }
}
void restore_regs(int argc, int depth) {
  for(int i = 0; i < argc; ++i) {
    int const offset = (depth + i) * 4;
    printf("\tmov -%d(%%rbp), %%%s\n", offset, REGS[i]);
  }
}

void emit_int_to(Ast const* ast, char const* reg) {
  printf("\tmovl $%d, %s\n", ast->int_val, reg);
}

void emit_ast_impl(Ast const* ast, Env const* env, int depth, char const* to) {
  if(to == NULL) { to = "%eax"; }
  AstType const t = ast->type;
  printf("# begin of %s\n", show_AstType(t));
  switch(t) {
  case AST_INT:
    emit_int_to(ast, to);
    break;
  case AST_OP_PLUS:
  case AST_OP_MULTI:
  {
    char const * const op = op_from_type(t);
    char op_with_suffix[MAX_OP_LEN];
    snprintf(op_with_suffix, MAX_OP_LEN, "%sl", op);
    int const offset = depth * 4;
    emit_ast_impl(ast->bi_op.lhs, env, depth + 1, NULL);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(ast->bi_op.rhs, env, depth + 2, NULL);
    printf("\t%s -%d(%%rbp), %%eax\n", op_with_suffix, offset);
    if(strcmp(to, "%eax")) {
      printf("\tmov %%eax, %s\n", to);
    }
    break;
  }
  case AST_OP_MINUS:
  {
    int const offset = depth * 4;
    emit_ast_impl(ast->bi_op.rhs, env, depth + 1, NULL);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(ast->bi_op.lhs, env, depth + 2, NULL);
    printf("\tsub -%d(%%rbp), %%eax\n", offset);
    if(strcmp(to, "%eax")) {
      printf("\tmov %%eax, %s\n", to);
    }
    break;
  }
  case AST_OP_DIV:
  {
    int const offset = depth * 4;
    emit_ast_impl(ast->bi_op.rhs, env, depth + 1, NULL);
    printf("\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(ast->bi_op.lhs, env, depth + 2, NULL);
    printf("\tcltd\n");
    printf("\tidivl -%d(%%rbp)\n", offset);
    if(strcmp(to, "%eax")) {
      printf("\tmov %%eax, %s\n", to);
    }
    break;
  }
  case AST_OP_ASSIGN:
  {
    char reg[MAX_REG_LEN];
    snprintf(reg, MAX_REG_LEN, "-%d(%%rbp)", ast->bi_op.lhs->var->offset);
    emit_ast_impl(ast->bi_op.rhs, env, depth + 1, reg);
    break;
  }
  case AST_SYM:
    printf("\tmov -%d(%%rbp), %s\n", ast->var->offset, to);
    break;
  case AST_SYM_DEFINE:
    break;
  case AST_STATEMENT:
    emit_ast_impl(ast->statement->val, env, depth, to);
    break;
  case AST_STATEMENTS:
    FOREACH(Statement, ast->statements->val, s) {
      emit_ast_impl(make_ast_statement(s), env, depth + 1, NULL);
    }
    if(strcmp(to, "%eax")) {
      printf("\tmov %%eax, %s\n", to);
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
    save_regs(argc, depth + 1);
    for(int i = 0; i < argc; ++i) {
      Ast* const arg = ast->funcall->args[i];
      if(arg->type == AST_INT) {
        char reg[MAX_REG_LEN];
        snprintf(reg, MAX_REG_LEN, "%%%s", REGS[i]);
        emit_int_to(arg, reg);
      } else {
        emit_ast_impl(arg, env, depth+7, NULL);
        printf("\tmov %%eax, %%%s\n", REGS[i]);
      }
    }
    printf("\tcall %s\n", name);
    if(strcmp(to, "%eax")) {
      printf("\tmov %%eax, %s\n", to);
    }
    restore_regs(argc, depth + 1);
    break;
  }
  case AST_BLOCK:
    emit_ast_impl(ast->block->val, env, depth, to);
    break;
  default:
    warn("never come!!!(type: %s)\n", show_AstType(t));
    break;
  }
  printf("# end of %s\n", show_AstType(t));
}

void emit_ast(Ast const* ast, Env const* env, int depth) {
  emit_ast_impl(ast, env, depth, NULL);
}

void emit_func(Ast const* ast, Env const* env) {
  assert(ast != NULL);
  assert(ast->type == AST_FUNDEFIN);
  FunDef const* const func = ast->fundef;

  printf(
    "\t.text\n"
    "\t.global main\n"
    "%s:\n"
    "\tpushq %%rbp\n"
    "\tmovq %%rsp, %%rbp\n"
    , func->name
  );
  emit_ast(func->body, env, list_of_Var_length(env->vars) + 1);
  printf("\tmov $0, %%eax\n");
  printf("\tpopq %%rbp\n");
  printf("\tret\n");
}

void emit(Ast const* ast, Env const* env) {
  emit_func(ast, env);
}

int main(int argc, char** argv) {
  INTRUSIVE_LIST_OF(Token) ts = tokenize(stdin);
  if(argc > 1 && !strcmp(argv[1], "-t")) {
    printf("col: %d\n", list_of_Token_length(ts));
    print_Tokens(ts);
    return 0;
  }
  Env* const env = new_Env();
  Type* int_ = new_Type("int"); //
  list_of_Type_append(env->types, int_);
  Ast* const ast = make_ast(env, ts);
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
