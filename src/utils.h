#ifndef NNA774_KONOHA_UTILS_H
#define NNA774_KONOHA_UTILS_H

#include <stdio.h>
#include <stdarg.h>

int const true = 1;
int const false = 0;

int peek(FILE* fp) {
  int const c = getc(fp);
  if(c == EOF) {
    return EOF;
  }
  ungetc(c, fp);
  return c;
}

void warn(char const* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

#endif // NNA774_KONOHA_UTILS_H
