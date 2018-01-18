#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

Ast* to_ast(Type t, void*);
Ast* parse_expr(FILE* fp, Env* env, int prio);
Ast* parse_funcall(FILE* fp, Env* env, char const* name);
int const MAX_ARGC = 6;
char const* const REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
  return malloc(sizeof(Ast) * (size + 1));
}

Env* new_Env() {
  Env* e = malloc(sizeof(Env));
  e->parent = NULL;
  e->vars = new_list_of_Var();
  return e;
}

Var* new_Var() {
  Var* v = malloc(sizeof(Var));
  v->name = NULL;
  v->offset = 0;
  init_Var_hook(v);
  return v;
}

FunCall* new_FunCall() {
  FunCall* f = malloc(sizeof(FunCall));
  f->name = NULL;
  f->argc = 0;
  f->args = NULL;
  return f;
}

Var* copy_var(Var const* _v) {
  Var* v = malloc(sizeof(Var));
  memcpy(v, _v, sizeof(Var));
  return v;
}

Statement* new_Statement() {
  Statement* s = malloc(sizeof(Statement));
  s->val = NULL;
  init_Statement_hook(s);
  return s;
}

Statements* new_Statements() {
  Statements* s = malloc(sizeof(Statements));
  s->val = NULL;
  return s;
}

int const MAX_BUF_LEN = 256;

char const * op_from_type(Type t) {
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

Ast* make_ast_symbol(Env* env, char const* name) {
  FOREACH(Var, env->vars, v) {
    if(!strcmp(v->name, name)) {
      return to_ast(AST_SYM, copy_var(v));
    }
  }
  Ast* const ast = new_Ast();
  ast->type = AST_SYM;
  ast->var = new_Var();
  ast->var->name = name;
  list_of_Var_append(env->vars, ast->var);
  ast->var->offset = list_of_Var_length(env->vars) * 4;
  return ast;
}

Ast* make_ast_bi_op(Type const t, Ast const* lhs, Ast const* rhs) {
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

Ast* to_ast(Type t, void* v) {
  Ast* const ast = new_Ast();
  ast->type = t;
  switch(t) {
  case AST_SYM:
    ast->var = v;
    break;
  case AST_STATEMENT:
    ast->statement = v;
    break;
  default:
    warn("unimpled type(%d)", t);
  }
  return ast;
}

Ast* make_ast_statement(Statement* s) {
  return to_ast(AST_STATEMENT, s);
}

Ast* make_ast_statements(INTRUSIVE_LIST_OF(Statement) ss) {
  Ast* const ast = new_Ast();
  ast->type = AST_STATEMENTS;
  ast->statements = new_Statements();
  ast->statements->val = ss;
  return ast;
}

Ast* make_ast_funcall(char const* name, int argc, Ast** args) {
  assert(args != NULL);
  Ast* const ast = new_Ast();
  Ast** const _args = new_Ast_array(argc);
  for(int i = 0; i < argc; ++i) {
    assert(args[i] != NULL);
    _args[i] = args[i];
  }
  ast->type = AST_FUNCALL;
  ast->funcall = new_FunCall();
  ast->funcall->name = name;
  ast->funcall->argc = argc;
  ast->funcall->args = _args;
  return ast;
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
  while(c = getc(fp), isalnum(c)) {
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
  skip(fp);
  int const c = peek(fp);
  if(c == '(') {
    getc(fp);
    return parse_funcall(fp, env, name);
  }
  return make_ast_symbol(env, name);
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
  Ast* args[MAX_ARGC];
  int argc = 0;
  for(; argc < MAX_ARGC; ++argc) {
    skip(fp);
    int const c = peek(fp);
    if(c == ')') {
      break;
    }
    if(c == EOF) { warn("unexpected EOF\n"); return NULL; }
    int const prio = 0;
    args[argc] = parse_expr(fp, env, prio);
    skip(fp);
    int const c2 = getc(fp);
    if(c2 == EOF) { warn("unexpected EOF\n"); return NULL; }
    if(c2 == ')') { break; }
    if(c2 == ',') { /* nop */ }
    else { warn("unexpected char(%c)\n", c2); return NULL; }
  }
  if(argc == MAX_ARGC) {
    warn("too many arg(max argc is %d)\n", MAX_ARGC);
    return NULL;
  }
  return make_ast_funcall(name, argc + 1, args);
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
      Type const t = detect_bi_op(c);
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
      ast = make_ast_bi_op(AST_OP_ASSIGN, lhs, rhs);
      break;
    }
    case ';':
    case ')':
    case ',':
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
    int const c = getc(fp);
    if(c == ';') { // empty statement
      Ast* const ast = new_Ast();
      ast->type = AST_EMPTY;
      return make_statement(ast);
    } else {
      ungetc(c, fp);
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

Ast* parse(FILE* fp, Env* env) {
  INTRUSIVE_LIST_OF(Statement) ss = new_list_of_Statement();
  int c;
  while(c = peek(fp), c != EOF) {
    skip(fp);
    Statement* s = parse_statement(fp, env);
    skip(fp);
    assert(s != NULL);
    list_of_Statement_append(ss, s);
  }
  return make_ast_statements(ss);
}

Ast* make_ast(Env* env) {
  Ast* const ast = parse(stdin, env);
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
  case AST_SYM:
    printf("%s", ast->var->name);
    break;
  case AST_OP_ASSIGN:
    printf("(let ");
    print_ast(ast->bi_op.lhs);
    printf(" ");
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
    printf("(%s ", ast->funcall->name);
    int const argc = ast->funcall->argc;
    for(int i = 0; i < argc; ++i) {
      print_ast(ast->funcall->args[i]);
      if(i != argc - 1) {
        printf(" ");
      }
    }
    printf(")");
    break;
  }
  default:
    warn("never come!!!(type: %d)\n", t);
  }
}

void print_env(Env const* env) {
  assert(env != NULL);
  if(env->parent != NULL) {
    printf("parent:");
    print_env(env->parent);
  }
  FOREACH(Var, env->vars, v) {
    printf("%s\n", v->name);
  }
}
