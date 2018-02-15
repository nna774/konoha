#include <stdio.h>
#include <unistd.h>
#include "ast.h"
#include "emit.h"
#include "tokenize.h"

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
