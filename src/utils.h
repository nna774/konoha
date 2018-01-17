#ifndef NNA774_KONOHA_UTILS_H
#define NNA774_KONOHA_UTILS_H

#include <stdio.h>

typedef int bool;
int const true;
int const false;

int peek(FILE* fp);
void warn(char const* fmt, ...);

#endif // NNA774_KONOHA_UTILS_H
