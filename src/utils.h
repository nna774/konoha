#ifndef NNA774_KONOHA_UTILS_H
#define NNA774_KONOHA_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#ifdef DEBUG
#define MEMWATCH
#define MW_STDIO
#include "memwatch.h"
#endif

#define CONCAT(x, y) _CONCAT_I(x, y)
#define _CONCAT_I(x, y) x ## y
#define CONCAT3(x, y, z) _CONCAT3_I(x, y, z)
#define _CONCAT3_I(x, y, z) x ## y ## z
#define CONCAT4(x, y, z, w) _CONCAT4_I(x, y, z, w)
#define _CONCAT4_I(x, y, z, w) x ## y ## z ## w

int peek(FILE* fp);
void skip(FILE* fp);
void _warn_impl(char const* file, int line, char const* func, char const* fmt, ...);

#define warn(...) \
  _warn_impl(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#ifndef NDEBUG
char const* show_char(int c);
#endif

#endif // NNA774_KONOHA_UTILS_H
