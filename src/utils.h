#ifndef NNA774_KONOHA_UTILS_H
#define NNA774_KONOHA_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define CONCAT(x, y) _CONCAT_I(x, y)
#define _CONCAT_I(x, y) x ## y

int peek(FILE* fp);
void warn(char const* fmt, ...);

#endif // NNA774_KONOHA_UTILS_H
