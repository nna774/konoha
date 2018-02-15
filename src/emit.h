#ifndef NNA774_KONOHA_EMIT_H
#define NNA774_KONOHA_EMIT_H

#include <stdio.h>
#include "ast.h"

void emit(FILE* outfile, Ast const* ast, Env const* env);

#endif // NNA774_KONOHA_EMIT_H
