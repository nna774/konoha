#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "tokenize.h"
#include "utils.h"

char const* const REGS[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
int const MAX_REG_LEN = 16;
int const MAX_OP_LEN = 16;

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

void emit_ast_impl(FILE* outfile, Ast const* ast, Env const* env, int depth, char const* to) {
  if(to == NULL) { to = "%eax"; }
  AstType const t = ast->type;
  fprintf(outfile, "# begin of %s\n", show_AstType(t));
  switch(t) {
  case AST_INT:
    emit_int_to(outfile, ast, to);
    break;
  case AST_OP_PLUS:
  case AST_OP_MULTI:
  {
    char const * const op = op_from_type(t);
    char op_with_suffix[MAX_OP_LEN];
    snprintf(op_with_suffix, MAX_OP_LEN, "%sl", op);
    int const offset = depth * 4;
    emit_ast_impl(outfile, ast->bi_op.lhs, env, depth + 1, NULL);
    fprintf(outfile, "\tmov %%eax, -%d(%%rbp)\n", offset);
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 2, NULL);
    fprintf(outfile, "\t%s -%d(%%rbp), %%eax\n", op_with_suffix, offset);
    if(strcmp(to, "%eax")) {
      fprintf(outfile, "\tmov %%eax, %s\n", to);
    }
    break;
  }
  case AST_OP_MINUS:
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
  case AST_OP_DIV:
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
  case AST_OP_ASSIGN:
  {
    char reg[MAX_REG_LEN];
    snprintf(reg, MAX_REG_LEN, "-%d(%%rbp)", ast->bi_op.lhs->var->offset);
    emit_ast_impl(outfile, ast->bi_op.rhs, env, depth + 1, reg);
    break;
  }
  case AST_SYM:
    fprintf(outfile, "\tmov -%d(%%rbp), %s\n", ast->var->offset, to);
    break;
  case AST_SYM_DEFINE:
    break;
  case AST_STATEMENT:
    if(ast->statement->type == RETURN_STATEMENT){
      fprintf(outfile, "# return statement\n");
      emit_ast_impl(outfile, ast->statement->val, env, depth, NULL);
      fprintf(outfile, "\tpopq %%rbp\n");
      fprintf(outfile, "\tret\n");
      break;
    }
    emit_ast_impl(outfile, ast->statement->val, env, depth, to);
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

void emit_func(FILE* outfile, Ast const* ast, Env const* env) {
  assert(ast != NULL);
  assert(ast->type == AST_FUNDEFIN);
  FunDef const* const func = ast->fundef;

  fprintf(
    outfile,
    "\t.text\n"
    "\t.global %s\n"
    "%s:\n"
    "\tpushq %%rbp\n"
    "\tmovq %%rsp, %%rbp\n"
    , func->name
    , func->name
  );
  emit_ast(outfile, func->body, env, list_of_Var_length(env->vars) + 1);
  fprintf(outfile, "\tpopq %%rbp\n");
  fprintf(outfile, "\tret\n");
}

void emit(FILE* outfile, Ast const* ast, Env const* env) {
  emit_func(outfile, ast, env);
}

enum Mode {
  TOKENIZE,
  AST,
  DUMP,
  EMIT,
};

int main(int argc, char** argv) {
  int opt;
  enum Mode mode = EMIT;
  FILE* infile = stdin;
  FILE* outfile = stdout;
  while ((opt = getopt(argc, argv, "tado:")) != -1) {
    switch (opt) {
    case 't':
      mode = TOKENIZE;
      break;
    case 'a':
      mode = AST;
      break;
    case 'd':
      mode = DUMP;
      break;
    case 'o':
      outfile = fopen(optarg, "w+");
      assert(outfile != NULL);
      break;
    default: /* '?' */
      printf("Usage: %s\n", argv[0]);
      break;
    }
  }
  if(optind < argc) {
    // src file
    if(argc - optind > 2) {
      warn("now, only one src file acceptable\n");
    }
    infile = fopen(argv[optind], "r");
    assert(infile != NULL);
  }

  INTRUSIVE_LIST_OF(Token) ts = tokenize(infile);
  fclose(infile);
  if(mode == TOKENIZE) {
    printf("col: %d\n", list_of_Token_length(ts));
    print_Tokens(ts);
    return 0;
  }

  Env* const env = new_Env();
  Type* int_ = new_Type("int"); //
  Type* char_ = new_Type("char"); //
  list_of_Type_append(env->types, int_);
  list_of_Type_append(env->types, char_);
  Ast* const ast = make_ast(env, ts);
  if (mode == AST) {
    print_ast(ast);
  } else if (mode == DUMP) {
    printf("ast:\n");
    print_ast(ast);
    printf("\nenv:\n");
    print_env(env);
  } else {
    emit(outfile, ast, env);
    fclose(outfile);
  }

  return 0;
}
