#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "ast.h"
#include "utils.h"

Ast* to_ast(AstType t, void*);
Type* read_type(Env* env, Tokens ts);
char const* read_symbol(Tokens ts);
Ast* parse_expr(Env* env, Tokens ts, int prio);
Ast* parse_funcall(Env* env, Tokens ts, char const* name);
Ast* parse_block(Env* env, Tokens ts);
char const* show_AstType(AstType);
int const MAX_ARGC = 6;

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

FunDef* new_FunDef(FunType type, char const* name, Var** args, Ast* body) {
  assert(name != NULL);
  assert(args != NULL);
  assert(body != NULL);
  assert(body->type == AST_BLOCK);
  FunDef* const t = malloc(sizeof(FunDef));
  t->type = type;
  t->name = name;
  t->args = args;
  t->body = body;
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
  case AST_OP_DIV:
    return "idivl";
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

Ast* parse_int(Token t, int sign) {
  assert(t.type == INTEGER_LITERAL_T);
  String const s = t.string;
  int sum = 0;
  int max = String_length(s);
  char const* ss = c_str(s);
  for(int i = 0; i < max; ++i) {
    sum  = sum * 10 + (ss[i] - '0');
  }
  return make_ast_int(sum * sign);
}

Ast* parse_symbol_or_funcall(Env* env, Tokens ts) {
  char const* name = read_symbol(ts);
  Type* const t = find_type_by_name(env, name);
  if(t != NULL) {
    // type
    char const* sym_name = read_symbol(ts);
    Token const token = peek_Token(ts);
    char const c = head_char(token.string);
    if(token.type == SEMICOLON_T) {
      // sym define
      return make_ast_val_define(env, t, sym_name);
    }
    if(c == '=') {
      // sym define with init val
      return NULL;
    }
    warn("unexpected token(%s)\n", c_str(token.string));
    return NULL;
  }
  Token const token = peek_Token(ts);
  char const c = head_char(token.string);
  if(token.type == OPEN_PAREN_T && c == '(') {
    // funcall
    return parse_funcall(env, ts, name);
  }

  // var
  Var* const v = find_var_by_name(env, name);
  if(v != NULL) {
    return make_ast_symbol_ref(env, v);
  }
  warn("identifier %s is not declared\n", name);
  return NULL;
}

Ast* parse_prim(Env* env, Tokens ts) {
  Token const t = peek_Token(ts);
  char const c = head_char(t.string);
  if(t.type == INTEGER_LITERAL_T) {
    pop_Token(ts);
    int const sign = 1;
    return parse_int(t, sign);
  } else if(t.type == IDENTIFIER_T) {
    return parse_symbol_or_funcall(env, ts);
  } else if(t.type == OPEN_PAREN_T && c == '(') {
    pop_Token(ts);
    int const prio = 0;
    Ast* const ast = parse_expr(env, ts, prio);
    Token const t2 = pop_Token(ts);
    char const c2 = head_char(t2.string);
    if(t2.type != CLOSE_PAREN_T || c2 != ')') {
      if(t2.type == EOF_T) { warn("unterminated expr(got unexpeced EOF)\n"); }
      else { warn("unterminated token(got %s)\n", c_str(t2.string)); }
      return NULL;
    }
    return ast;
  } else if(c == '+' || c == '-') {
    pop_Token(ts);
    Token const t2 = peek_Token(ts);
    if(t2.type == INTEGER_LITERAL_T) {
      pop_Token(ts);
      return parse_int(t2, c == '+' ? 1 : -1);
    }
    int const prio = 0;
    Ast* const subseq = parse_expr(env, ts, prio);
    if(c == '+') {
      return subseq;
    }
    // -
    Ast* const neg = make_ast_int(-1);
    return make_ast_bi_op(AST_OP_MULTI, neg, subseq);
  } else {
    if(t.type == EOF_T) { warn("unexpected EOF\n"); }
    else { warn("unknown token: %s\n", c_str(t.string)); }
    return NULL;
  }
}

Ast* parse_funcall(Env* env, Tokens ts, char const* name) {
  Ast** const args = new_Ast_array(MAX_ARGC);
  Token t = pop_Token(ts);
  if(t.type != OPEN_PAREN_T || head_char(t.string) != '(') {
    warn("###");
  }
  int argc = 0;
  for(; argc <= MAX_ARGC; ++argc) {
    t = peek_Token(ts);
    if(t.type == CLOSE_PAREN_T && head_char(t.string) == ')') {
      pop_Token(ts);
      break;
    }
    if(argc == 0) { continue; }
    if(t.type == EOF_T) { warn("unexpected EOF\n"); return NULL; }
    int const prio = 0;
    args[argc - 1] = parse_expr(env, ts, prio);
    t = pop_Token(ts);
    if(t.type == EOF_T) { warn("unexpected EOF\n"); return NULL; }
    if(head_char(t.string) == ')') { break; }
    if(head_char(t.string) == ',') { /* nop */ }
    else { warn("unexpected token(%s)\n", c_str(t.string)); return NULL; }
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
  case '/':
    return 2;
  default:
    warn("unknown bi-op(got: %c)\n", op);
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
  case '/':
    return AST_OP_DIV;
  default:
    warn("unknown bi-op(got: %c)\n", c);
    return AST_UNKNOWN;
  }
}

bool compare_with_name(Var* lhs, Var* rhs) {
  return !strcmp(lhs->name, rhs->name);
}

Ast* parse_expr(Env* env, Tokens ts, int prio) {
  Ast* ast = parse_prim(env, ts);
  assert(ast != NULL);
  while(true) {
    Token const t = pop_Token(ts);
    char const c = head_char(t.string);
    switch(c) {
    case '+':
    case '-':
    case '*':
    case '/':
    {
      int const c_prio = priority(c);
      if(c_prio < prio) {
        push_Token(ts, t);
        return ast;
      }
      AstType const type = detect_bi_op(c);
      Ast* const lhs = ast;
      Ast* const rhs = parse_expr(env, ts, c_prio + 1);
      ast = make_ast_bi_op(type, lhs, rhs);
      break;
    }
    case '=':
    {
      Ast* const lhs = ast;
      Ast* const rhs = parse_expr(env, ts, prio);
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
      push_Token(ts, t);
      return ast;
    }
    default:
      warn("never come!!!(got: %s)(token type: %s)\n", show_char(c), show_TokenType(t.type));
      return NULL;
    }
  }
  warn("reached to unreachable path");
  return NULL; // never come
}

Statement* parse_statement(Env* env, Tokens ts) {
  {
    Token t = peek_Token(ts);
    if(t.type == SEMICOLON_T) { // empty statement
      pop_Token(ts);
      Ast* const ast = new_Ast();
      ast->type = AST_EMPTY;
      return make_statement(ast);
    } else if(t.type == OPEN_PAREN_T && head_char(t.string) == '{') {
      return make_statement(parse_block(env, ts));
    }
  }
  int const prio = 0;
  Ast* const ast = parse_expr(env, ts, prio);
  assert(ast != NULL);

  Token t = pop_Token(ts);
  if(t.type != SEMICOLON_T) {
    if(t.type == EOF_T) { warn("unterminated expr(got unexpeced EOF)\n"); }
    else {
      warn("unterminated expr(got %s)\n", c_str(t.string));
    }
    return NULL;
  }

  return make_statement(ast);
}

Ast* parse_statements(Env* env, Tokens ts) {
  INTRUSIVE_LIST_OF(Statement) ss = new_list_of_Statement();
  Token t;
  while(t = peek_Token(ts), t.type != EOF_T) {
    char const c = head_char(t.string);
    if(t.type == CLOSE_PAREN_T && c == '}') {
      // end of block
      break;
    }
    Statement* s = parse_statement(env, ts);
    assert(s != NULL);
    list_of_Statement_append(ss, s);
  }
  return make_ast_statements(ss);
}

Ast* parse_block(Env* env, Tokens ts) {
  Token t = pop_Token(ts);
  Env* expanded = expand_Env(env);
  char c = head_char(t.string);
  if(t.type != OPEN_PAREN_T || c != '{') {
    warn("unexpected token(%s)\n", c_str(t.string));
    return NULL;
  }
  Ast* const ss = parse_statements(expanded, ts);
  t = pop_Token(ts);
  c = head_char(t.string);
  if(t.type != CLOSE_PAREN_T || c != '}') {
    warn("unexpected token(%s)\n", c_str(t.string));
    return NULL;
  }
  assert(ss->type == AST_STATEMENTS);
  assert(ss->statements != NULL);
  return make_ast_block(expanded, ss);
}

Type* read_type(Env* env, Tokens ts) {
  Token const name = pop_Token(ts);
  assert(name.type == IDENTIFIER_T);
  Type* const type = find_type_by_name(env, c_str(name.string));
  assert(type != NULL);
  return type;
}

char const* read_symbol(Tokens ts) {
  Token const t = pop_Token(ts);
  assert(t.type == IDENTIFIER_T);
  return c_str(t.string);
}

Ast* parse_fundef(Env* env, Tokens ts) {
  Type* const ret_type = read_type(env, ts);
  char const* const name = read_symbol(ts);
  Token open = pop_Token(ts);
  char const o = head_char(open.string);
  if(open.type != OPEN_PAREN_T
     || o != '(') {
    warn("unexpected char(%c)\n", o);
    return NULL;
  }
  Env* const expanded = expand_Env(env);
  Type** arg_types = malloc(sizeof(Type*) * MAX_ARGC);
  Var** args = malloc(sizeof(Var*) * MAX_ARGC);
  int argc = 0;
  for(; argc <= MAX_ARGC + 1; ++argc) {
    Token const t = peek_Token(ts);
    char const c = head_char(t.string);
    if(t.type == CLOSE_PAREN_T && c == ')') {
      pop_Token(ts);
      break;
    }
    if(argc == 0) { continue; }
    Type* const type = read_type(env, ts);
    arg_types[argc - 1] = type;
    char const* const name = read_symbol(ts);
    args[argc - 1] = add_sym_to_env(expanded, type, name);
    Token const t2 = peek_Token(ts);
    char const c2 = head_char(t2.string);
    if(t2.type == COMMA_T) {
      pop_Token(ts);
    }
    if(t2.type == CLOSE_PAREN_T && c2 == ')') {
      pop_Token(ts);
      break;
    }
  }

  Ast* const body = parse_block(expanded, ts);
  FunType t = {
    ret_type,
    argc,
    arg_types,
  };

  Ast* const ast = new_Ast();
  ast->type = AST_FUNDEFIN;
  ast->fundef = new_FunDef(t, name, args, body);
  return ast;
}

Ast* parse(Env* env, Tokens ts) {
  Ast* const ast = parse_fundef(env, ts);
  return ast;
}

Ast* make_ast(Env* env, Tokens ts) {
  Ast* const ast = parse(env, ts);
  if(list_of_Token_length(ts) != 1) {
    warn("token remains! possible parser bug. rest tokens are here:\n");
    print_Tokens(ts);
  }
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
  case AST_OP_DIV:
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
  case AST_FUNDEFIN:
  {
    FunDef const* const func = ast->fundef;
    assert(func != NULL);
    printf("(defun %s<%s(", func->name, func->type.return_type->name);
    for(int i = 0; i < func->type.argc; ++i) {
      Type const* const type = func->type.arg_types[i];
      assert(type != NULL);
      printf("%s", type->name);
      if(i != func->type.argc - 1) {
        printf(", ");
      }
    }
    printf(")> (");

    for(int i = 0; i < func->type.argc; ++i) {
      assert(func->args != NULL);
      Var const* const arg = func->args[i];
      assert(arg != NULL);
      assert(arg->name != NULL);
      printf("%s", arg->name);
      if(i != func->type.argc - 1) {
        printf(", ");
      }
    }
    printf(") ");
    print_ast(func->body);
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
  fflush(stdout);
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
