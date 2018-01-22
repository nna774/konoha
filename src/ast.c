#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

Ast* to_ast(AstType t, void*);
Ast* parse_expr(FILE* fp, Env* env, int prio);
Ast* parse_funcall(FILE* fp, Env* env, char const* name);
Ast* parse_block(FILE* fp, Env* env);
char const* show_AstType(AstType);
int const MAX_ARGC = 6;

void skip(FILE* fp) {
  int c;
  while(c = getc(fp), c != EOF) {
    if(!isspace(c)) {
      break;
    }
  }
  ungetc(c, fp);
}

Ast* new_Ast() {
  return malloc(sizeof(Ast));
}

Ast** new_Ast_array(size_t size) {
  Ast** const arr = malloc(sizeof(Ast*) * (size));
  for(size_t i = 0; i < size; ++i) {
    arr[i] = new_Ast();
  }
  return arr;
}

Env* new_Env_impl(Env* env) {
  Env* const e = malloc(sizeof(Env));
  e->parent = env;
  e->types = new_list_of_Type();
  e->vars = new_list_of_Var();
  return e;
}

Env* new_Env() {
  return new_Env_impl(NULL);
}

Env* expand_Env(Env* parent) {
  return new_Env_impl(parent);
}

Var* new_Var(Type* t, char const* name) {
  assert(t != NULL);
  Var* const v = malloc(sizeof(Var));
  v->name = name;
  v->type = t;
  v->initialized = false;
  v->defined = false;
  v->offset = 0;
  init_Var_hook(v);
  return v;
}

FunCall* new_FunCall() {
  FunCall* const f = malloc(sizeof(FunCall));
  f->name = NULL;
  f->argc = 0;
  f->args = NULL;
  return f;
}

Var* copy_var(Var const* _v) {
  Var* const v = malloc(sizeof(Var));
  memcpy(v, _v, sizeof(Var));
  return v;
}

Statement* new_Statement() {
  Statement* const s = malloc(sizeof(Statement));
  s->val = NULL;
  init_Statement_hook(s);
  return s;
}

Statements* new_Statements() {
  Statements* const s = malloc(sizeof(Statements));
  s->val = NULL;
  return s;
}

Block* new_Block(Env* env) {
  Block* const b = malloc(sizeof(Block));
  b->val = NULL;
  assert(env != NULL);
  b->env = env;
  return b;
}

Type* new_Type(char const* name) {
  assert(name != NULL);
  Type* const t = malloc(sizeof(Type));
  t->name = name;
  init_Type_hook(t);
  return t;
}

int const MAX_BUF_LEN = 256;

char const * op_from_type(AstType t) {
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

bool same_name_var(Var const* lhs, Var const* rhs) {
  return !strcmp(lhs->name, rhs->name);
}

Ast* make_ast_symbol_ref(Env* env, Var* var) {
  Var* const v = list_of_Var_find_cond(env->vars, var, same_name_var);
  assert(v != NULL);
  return to_ast(AST_SYM, v);
}

Ast* make_ast_bi_op(AstType const t, Ast const* lhs, Ast const* rhs) {
  Ast* const ast = new_Ast();
  ast->type = t;
  ast->bi_op.lhs = lhs;
  ast->bi_op.rhs = rhs;
  return ast;
}

Statement* make_statement(Ast* st) {
  Statement* const s = new_Statement();
  s->val = st;
  return s;
}

Ast* to_ast(AstType t, void* v) {
  Ast* const ast = new_Ast();
  ast->type = t;
  switch(t) {
  case AST_SYM:
    ast->var = v;
    break;
  case AST_STATEMENT:
    ast->statement = v;
    break;
  case AST_STATEMENTS:
    ast->statements = new_Statements();
    ast->statements->val = v;
    break;
  case AST_BLOCK:
    warn("block can't make from to_ast\n");
    return NULL;
  case AST_SYM_DEFINE:
    ast->var = v;
    break;
  default:
    warn("unimpled type(%s)", show_AstType(t));
  }
  return ast;
}

Ast* make_ast_statement(Statement* s) {
  return to_ast(AST_STATEMENT, s);
}

Ast* make_ast_statements(INTRUSIVE_LIST_OF(Statement) ss) {
  return to_ast(AST_STATEMENTS, ss);
}

Ast* make_ast_block(Env* env, Ast* b) {
  assert(b->type == AST_STATEMENTS);
  Ast* const ast = new_Ast();
  ast->type = AST_BLOCK;
  ast->block = new_Block(env);
  ast->block->val = b;
  return ast;
}

Ast* make_ast_funcall(char const* name, int argc, Ast** args) {
  Ast* const ast = new_Ast();
  ast->type = AST_FUNCALL;
  ast->funcall = new_FunCall();
  ast->funcall->name = name;
  ast->funcall->argc = argc;
  if (argc == 0) {
    return ast;
  }
  assert(args != NULL);
  ast->funcall->args = args;
  return ast;
}

Type* find_type_by_name(Env const* env, char const* name) {
  assert(env->types != NULL);
  FOREACH(Type, env->types, t) {
    if(!strcmp(t->name, name)) {
      return t;
    }
  }
  if(env->parent != NULL) {
    return find_type_by_name(env->parent, name);
  }
  return NULL;
}

Var* find_var_by_name(Env* env, char const* name) {
  FOREACH(Var, env->vars, v) {
    if(!strcmp(v->name, name)) {
      return v;
    }
  }
  return NULL;
}

Var* add_sym_to_env(Env* env, Type* type, char const* sym_name) {
  assert(find_type_by_name(env, type->name) != NULL);
  if(find_var_by_name(env, sym_name)) {
    warn("identifier %s is already declared\n", sym_name);
  }

  Var* const v = new_Var(type, sym_name);
  list_of_Var_append(env->vars, v);
  v->offset = list_of_Var_length(env->vars) * 4;
  return v;
}

Ast* make_ast_val_define(Env* env, Type* t, char const* sym_name) {
  Var* v = add_sym_to_env(env, t, sym_name);
  return to_ast(AST_SYM_DEFINE, v);
}

Ast* parse_int(FILE* fp, int sign) {
  int sum = 0;
  int c;
  while(c = getc(fp), isdigit(c)) {
    sum  = sum * 10 + (c - '0');
  }
  ungetc(c, fp);
  return make_ast_int(sum * sign);
}

char const* read_symbol(FILE* fp) {
  char* const buf = malloc(MAX_BUF_LEN); // todo: expand buf when needed
  int c;
  int at = 0;
  while(c = getc(fp), isalnum(c) || c == '_') {
    if (at >= MAX_BUF_LEN - 1) {
      warn("symbol too long! giving up!\n");
      return NULL;
    }
    buf[at++] = c;
  }
  ungetc(c, fp);
  buf[at] = '\0';
  return buf;
}

Ast* parse_symbol_or_funcall(FILE* fp, Env* env) {
  char const* name = read_symbol(fp);
  Type* t;
  if(t = find_type_by_name(env, name), t != NULL) {
    skip(fp);
    char const* sym_name = read_symbol(fp);
    skip(fp);
    int const c = peek(fp);
    if(c == ';') {
      // sym define
      return make_ast_val_define(env, t, sym_name);
    }
    if(c == '=') {
      // sym define with inti val
      return NULL;
    }
    warn("unexpected char(%c)\n", c);
    return NULL;
  }
  skip(fp);
  int const c = peek(fp);
  if(c == '(') {
    getc(fp);
    return parse_funcall(fp, env, name);
  }

  Var* const v = find_var_by_name(env, name);
  if(v != NULL) {
    return make_ast_symbol_ref(env, v);
  }
  warn("identifier %s is not declared\n", name);
  return NULL;
}

Ast* parse_prim(FILE* fp, Env* env) {
  int const c = peek(fp);
  if(isdigit(c)) {
    int const sign = 1;
    return parse_int(fp, sign);
  } else if(isalpha(c)) {
    return parse_symbol_or_funcall(fp, env);
  } else if(c == '(') {
    getc(fp);
    int const prio = 0;
    Ast* const ast = parse_expr(fp, env, prio);
    int const c = getc(fp);
    if(c != ')') {
      if(c == EOF) { warn("unterminated expr(got unexpeced EOF)\n"); }
      else { warn("unterminated expr(got %c)\n", c); }
      return NULL;
    }
    return ast;
  } else if(c == '+' || c == '-') {
    getc(fp);
    int const c2 = peek(fp);
    if(isdigit(c2)) {
      return parse_int(fp, c == '+' ? 1 : -1);
    }
    int const prio = 0;
    Ast* const subseq = parse_expr(fp, env, prio);
    if(c == '+') {
      return subseq;
    }
    // -
    Ast* const neg = make_ast_int(-1);
    return make_ast_bi_op(AST_OP_MULTI, neg, subseq);
  } else {
    if(c == EOF) { warn("unexpected EOF\n"); }
    else { warn("unknown char: %c\n", c); }
    return NULL;
  }
}

Ast* parse_funcall(FILE* fp, Env* env, char const* name) {
  Ast** const args = new_Ast_array(MAX_ARGC);
  int argc = 0;
  for(; argc <= MAX_ARGC; ++argc) {
    skip(fp);
    int const c = peek(fp);
    if(c == ')') {
      getc(fp);
      break;
    }
    if(argc == 0) { continue; }
    if(c == EOF) { warn("unexpected EOF\n"); return NULL; }
    int const prio = 0;
    args[argc - 1] = parse_expr(fp, env, prio);
    skip(fp);
    int const c2 = getc(fp);
    if(c2 == EOF) { warn("unexpected EOF\n"); return NULL; }
    if(c2 == ')') { break; }
    if(c2 == ',') { /* nop */ }
    else { warn("unexpected char(%c)\n", c2); return NULL; }
  }
  if(argc > MAX_ARGC) {
    warn("too many arg(max argc is %d)\n", MAX_ARGC);
    return NULL;
  }
  return make_ast_funcall(name, argc, args);
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

AstType detect_bi_op(char c) {
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

bool compare_with_name(Var* lhs, Var* rhs) {
  return !strcmp(lhs->name, rhs->name);
}

Ast* parse_expr(FILE* fp, Env* env, int prio) {
  Ast* ast = parse_prim(fp, env);
  assert(ast != NULL);
  while(true) {
    skip(fp);
    int const c = getc(fp);
    switch(c) {
    case '+':
    case '-':
    case '*':
    {
      int const c_prio = priority(c);
      if(c_prio < prio) {
        ungetc(c, fp);
        return ast;
      }
      AstType const t = detect_bi_op(c);
      skip(fp);
      Ast* const lhs = ast;
      Ast* const rhs = parse_expr(fp, env, c_prio + 1);
      ast = make_ast_bi_op(t, lhs, rhs);
      break;
    }
    case '=':
    {
      skip(fp);
      Ast* const lhs = ast;
      Ast* const rhs = parse_expr(fp, env, prio);
      assert(lhs->type == AST_SYM);
      lhs->var->initialized = true;
      ast = make_ast_bi_op(AST_OP_ASSIGN, lhs, rhs);
      break;
    }
    case ';':
    case ')':
    case ',':
    case '}':
    {
      ungetc(c, fp);
      return ast;
    }
    case EOF:
      warn("unexpected EOF\n");
      return ast;
    default:
      warn("never come!!!(got: %c)\n", c);
      return NULL;
    }
  }
  warn("reached to unreachable path");
  return NULL; // never come
}

Statement* parse_statement(FILE* fp, Env* env) {
  {
    int const c = peek(fp);
    if(c == ';') { // empty statement
      getc(fp);
      Ast* const ast = new_Ast();
      ast->type = AST_EMPTY;
      return make_statement(ast);
    } else if(c == '{') {
      return make_statement(parse_block(fp, env));
    }
  }
  int const prio = 0;
  Ast* const ast = parse_expr(fp, env, prio);
  assert(ast != NULL);
  skip(fp);

  int const c = getc(fp);
  if(c != ';') {
    if(c == EOF) { warn("unterminated expr(got unexpeced EOF)\n"); }
    else { warn("unterminated expr(got %c)\n", c); }
    return NULL;
  }

  return make_statement(ast);
}

Ast* parse_statements(FILE* fp, Env* env) {
  INTRUSIVE_LIST_OF(Statement) ss = new_list_of_Statement();
  int c;
  while(c = peek(fp), c != EOF) {
    if(c == '}') {
      // end of block
      break;
    }
    skip(fp);
    Statement* s = parse_statement(fp, env);
    skip(fp);
    assert(s != NULL);
    list_of_Statement_append(ss, s);
  }
  return make_ast_statements(ss);
}

Ast* parse_block(FILE* fp, Env* env) {
  skip(fp);
  int c = getc(fp);
  Env* expanded = expand_Env(env);
  if(c != '{') { warn("unexpected char(%c)\n", c); return NULL; }
  Ast* const ss = parse_statements(fp, expanded);
  c = getc(fp);
  if(c != '}') { warn("unexpected char(%c)\n", c); return NULL; }
  assert(ss->type == AST_STATEMENTS);
  assert(ss->statements != NULL);
  return make_ast_block(expanded, ss);
}

Ast* parse(FILE* fp, Env* env) {
  Ast* const ast = parse_block(fp, env);
  return ast;
}

Ast* make_ast(Env* env) {
  Ast* const ast = parse(stdin, env);
  return ast;
}

void print_ast(Ast const* ast) {
  assert(ast != NULL);
  AstType const t = ast->type;
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
    if(!ast->var->initialized) {
      warn("%s is not initialized, but evaled\n", ast->var->name);
    }
    printf("(eval %s)", ast->var->name);
    break;
  case AST_SYM_DECLER:
    warn("unimpled");
    break;
  case AST_SYM_DEFINE:
    printf("(defvar %s)", ast->var->name);
    break;
  case AST_OP_ASSIGN:
    assert(ast->bi_op.lhs->type == AST_SYM);
    printf("(let %s ", ast->bi_op.lhs->var->name);
    print_ast(ast->bi_op.rhs);
    printf(")");
    break;
  case AST_STATEMENT:
    print_ast(ast->statement->val);
    break;
  case AST_STATEMENTS:
    FOREACH(Statement, ast->statements->val, s) {
      print_ast(make_ast_statement(s));
    }
    break;
  case AST_FUNCALL:
  {
    printf("(%s", ast->funcall->name);
    int const argc = ast->funcall->argc;
    if (argc == 0) {
      printf(")");
      break;
    }
    printf(" ");
    for(int i = 0; i < argc; ++i) {
      Ast const* arg = ast->funcall->args[i];
      assert(arg != NULL);
      print_ast(arg);
      if(i != argc - 1) {
        printf(" ");
      }
    }
    printf(")");
    break;
  }
  case AST_BLOCK:
  {
    printf("(do ");
    print_ast(ast->block->val);
    printf(")");
    break;
  }
  case AST_EMPTY:
    break;
  default:
    warn("never come!!!(type: %s)\n", show_AstType(t));
  }
}

char const* show_AstType(AstType t) {
  switch(t) {
  case AST_INT:
    return "AST_INT";
  case AST_OP_PLUS:
    return "AST_OP_PLUS";
  case AST_OP_MINUS:
    return "AST_OP_MINUS";
  case AST_OP_MULTI:
    return "AST_OP_MULTI";
  case AST_OP_ASSIGN:
    return "AST_OP_ASSIGN";
  case AST_SYM:
    return "AST_SYM";
  case AST_SYM_DECLER:
    return "AST_SYM_DECLER";
  case AST_SYM_DEFINE:
    return "AST_SYM_DEFINE";
  case AST_STATEMENT:
    return "AST_STATEMENT";
  case AST_STATEMENTS:
    return "AST_STATEMENTS";
  case AST_FUNCALL:
    return "AST_FUNCALL";
  case AST_EMPTY:
    return "AST_EMPTY";
  case AST_BLOCK:
    return "AST_BLOCK";
  case AST_UNKNOWN:
    return "AST_UNKNOWN";
  default:
    warn("?(type: %d)", t);
    return "?";
  }
}

void print_env(Env const* env) {
  assert(env != NULL);
  if(env->parent != NULL) {
    printf("parent:");
    print_env(env->parent);
  }
  FOREACH(Var, env->vars, v) {
    printf("%s %s\n", v->type->name, v->name);
  }
}
