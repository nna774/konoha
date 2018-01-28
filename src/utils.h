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

int peek(FILE* fp);
void skip(FILE* fp);
void warn(char const* fmt, ...);

#ifndef NDEBUG
char const* show_char(int c);
#endif

#endif // NNA774_KONOHA_UTILS_H
