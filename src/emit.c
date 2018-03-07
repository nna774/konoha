#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "emit.h"

void emit_ast_impl(FILE* outfile, Ast const* ast, Env const* env, int depth, char const* to);

char const* const REGS[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
int const MAX_REG_LEN = 16;
int const MAX_OP_LEN = 16;
int const MAX_LABEL_LEN = 8;

char const* make_label() {
  int static cnt = 0;
  char* const l = malloc(MAX_LABEL_LEN);
  snprintf(l, MAX_LABEL_LEN, ".L%d", cnt++);
  return l;
}

void save_regs(FILE* outfile, int argc, int depth) {
  for(int i = 0; i < argc; ++i) {
    int const offset = (depth + i) * 4;
    fprintf(outfile, "\tmov %%%s, -%d(%%rbp)\n", REGS[i], offset);
  }
}
void restore_regs(FILE* outfile, int argc, int depth) {
  for(int i = 0; i < argc; ++i) {
    int const offset = (depth + i) * 4;
    fprintf(outfile, "\tmov -%d(%%rbp), %%%s\n", offset, REGS[i]);
  }
}

void emit_int_to(FILE* outfile, Ast const* ast, char const* reg) {
  fprintf(outfile, "\tmovl $%d, %s\n", ast->int_val, reg);
}

void emit_bi_op(FILE* outfile, Ast const* ast, Env const* env, int depth, char const* to) {
  TokenType const t = ast->bi_op.op_type;
  switch(t) {
  case OP_PLUS_T:
  case OP_MULTI_T:
  case OP_EQUAL_T:
  {
    char const * const op = op_from_type(t);
    char op_with_suffix[MAX_OP_LEN];
    snprintf(op_with_suffix, MAX_OP_LEN, "%sl", op);
    int const offset = depth * 4;
    emit_ast_impl(outfile, ast->bi_op.lhs, env, depth + 1, NULL);
    fprintf(outfile, "\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 2, NULL);
    fprintf(outfile, "\t%s -%d(%%rbp), %%eax\n", op_with_suffix, offset);
    if(t == OP_EQUAL_T) {
      fprintf(outfile,
              "\tsete %%al\n"
              "\tmovzbl %%al, %%eax\n");
    }
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
    }
    break;
  }
  case OP_MINUS_T:
  {
    int const offset = depth * 4;
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 1, NULL);
    fprintf(outfile, "\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(outfile, ast->bi_op.lhs, env, depth + 2, NULL);
    fprintf(outfile, "\tsub -%d(%%rbp), %%eax\n", offset);
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
    }
    break;
  }
  case OP_DIV_T:
  {
    int const offset = depth * 4;
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 1, NULL);
    fprintf(outfile, "\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(outfile, ast->bi_op.lhs, env, depth + 2, NULL);
    fprintf(outfile, "\tcltd\n");
    fprintf(outfile, "\tidivl -%d(%%rbp)\n", offset);
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
    }
    break;
  }
  case OP_ASSIGN_T:
  {
    char reg[MAX_REG_LEN];
    snprintf(reg, MAX_REG_LEN, "-%d(%%rbp)", ast->bi_op.lhs->var->offset);
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 1, reg);
    break;
  }
  default:
    // never come
    warn("unknown token type(%s)\n", show_TokenType(t));
  }
}

void emit_ast_impl(FILE* outfile, Ast const* ast, Env const* env, int depth, char const* to) {
  if(to == NULL) { to = "%eax"; }
  AstType const t = ast->type;
  fprintf(outfile, "# begin of %s\n", show_AstType(t));
  switch(t) {
  case AST_INT:
    emit_int_to(outfile, ast, to);
    break;
  case AST_BI_OP:
    emit_bi_op(outfile, ast, env, depth, to);
    break;
  case AST_SYM:
    fprintf(outfile, "\tmov -%d(%%rbp), %s\n", ast->var->offset, to);
    break;
  case AST_SYM_DEFINE:
    break;
  case AST_STATEMENT:
    switch(ast->statement->type) {
    case NORMAL_STATEMENT:
      emit_ast_impl(outfile, ast->statement->val, env, depth, to);
      break;
    case RETURN_STATEMENT:
      fprintf(outfile, "# return statement\n");
      emit_ast_impl(outfile, ast->statement->val, env, depth, NULL);
      fprintf(outfile, "\tmovq %%rbp, %%rsp\n");
      fprintf(outfile, "\tpopq %%rbp\n");
      fprintf(outfile, "\tret\n");
      break;
    case IF_STATEMENT:
    {
      char const* const join = make_label();
      emit_ast_impl(outfile, ast->statement->if_val.cond, env, depth, NULL);
      fprintf(outfile, "\tcmpl $0, %%eax\n");
      if(ast->statement->if_val.else_body != NULL) {
        char const* const else_l = make_label();
        fprintf(outfile, "\tje %s\n", else_l);
        emit_ast_impl(outfile, make_ast_statement(ast->statement->if_val.body), env, depth, NULL);
        fprintf(outfile, "\tjmp %s\n", join);
        fprintf(outfile, "%s:\n", else_l);
        emit_ast_impl(outfile, make_ast_statement(ast->statement->if_val.else_body), env, depth, NULL);
      } else {
        fprintf(outfile, "\tje %s\n", join);
        emit_ast_impl(outfile, make_ast_statement(ast->statement->if_val.body), env, depth, NULL);
      }
      fprintf(outfile, "%s:\n", join);
      break;
    }
    case WHILE_STATEMENT:
    {
      char const* const init = make_label();
      char const* const join = make_label();
      fprintf(outfile, "%s:\n", init);
      emit_ast_impl(outfile, ast->statement->while_val.cond, env, depth, NULL);
      fprintf(outfile, "\tcmpl $0, %%eax\n");
      fprintf(outfile, "\tje %s\n", join);
      emit_ast_impl(outfile, make_ast_statement(ast->statement->if_val.body), env, depth, NULL);
      fprintf(outfile, "\tjmp %s\n", init);
      fprintf(outfile, "%s:\n", join);
      break;
    }
    default:
      warn("unimpled statement type(%s)\n", show_StatementType(ast->statement->type));
    }
    break;
  case AST_STATEMENTS:
    FOREACH(Statement, ast->statements->val, s) {
      emit_ast_impl(outfile, make_ast_statement(s), env, depth + 1, NULL);
    }
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
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
    save_regs(outfile, argc, depth + 1);
    for(int i = 0; i < argc; ++i) {
      Ast* const arg = ast->funcall->args[i];
      if(arg->type == AST_INT) {
        char reg[MAX_REG_LEN];
        snprintf(reg, MAX_REG_LEN, "%%%s", REGS[i]);
        emit_int_to(outfile, arg, reg);
      } else {
        emit_ast_impl(outfile, arg, env, depth+7, NULL);
        fprintf(outfile, "\tmov %%eax, %%%s\n", REGS[i]);
      }
    }
    fprintf(outfile, "\tcall %s\n", name);
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
    }
    restore_regs(outfile, argc, depth + 1);
    break;
  }
  case AST_BLOCK:
    emit_ast_impl(outfile, ast->block->val, env, depth, to);
    break;
  default:
    warn("never come!!!(type: %s)\n", show_AstType(t));
    break;
  }
  fprintf(outfile, "# end of %s\n", show_AstType(t));
}

void emit_ast(FILE* outfile, Ast const* ast, Env const* env, int depth) {
  emit_ast_impl(outfile, ast, env, depth, NULL);
}

int round16(int n) {
  if(n % 16 == 0) { return n; }
  return (n / 16 + 1) * 16;
}

void assign_parameter(FILE* outfile, int argn, Var** args) {
  char const* const reg = REGS[argn];
  fprintf(outfile, "\tmov %%%s, -%d(%%rbp)\n", reg, args[argn]->offset);
}

void emit_func(FILE* outfile, Ast const* ast, Env const* env) {
  assert(ast != NULL);
  assert(ast->type == AST_FUNDEFIN);
  FunDef const* const func = ast->fundef;
  int const var_cnt = var_count(env) + 1;
  int const stack = round16(var_cnt * 4);
  fprintf(
    outfile,
    "\t.global %s\n"
    "%s:\n"
    "\tpushq %%rbp\n"
    "\tmovq %%rsp, %%rbp\n"
    "\tsubq $%d, %%rsp\n"
    , func->name
    , func->name
    , stack
  );
  for(int i = 0; i < func->type.argc; ++i) {
    assign_parameter(outfile, i, func->args);
  }
  emit_ast(outfile, func->body, env, var_cnt + func->type.argc);
  fprintf(outfile, "\tmovq %%rbp, %%rsp\n");
  fprintf(outfile, "\tpopq %%rbp\n");
  fprintf(outfile, "\tret\n");
}

void emit(FILE* outfile, Ast const* ast, Env const* env) {
  assert(ast != NULL);
  assert(ast->type == AST_GLOBAL);
  fprintf(outfile, "\t.text\n");
  FOREACH(Ast, ast->global->list, s) {
    emit_func(outfile, s, env);
  }
}
