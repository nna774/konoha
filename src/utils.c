#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utils.h"

int peek(FILE* fp) {
  int const c = getc(fp);
  if(c == EOF) {
    return EOF;
  }
  ungetc(c, fp);
  return c;
}

void skip(FILE* fp) {
  int c;
  while(c = getc(fp), c != EOF) {
    if(!isspace(c)) {
      break;
    }
  }
  ungetc(c, fp);
}

void _warn_impl(int line, char const* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%d: ", line);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fflush(stderr);
}

#ifndef NDEBUG
char const* show_char(int c) {
  if(c == EOF) {
    return "EOF";
  }
  int const s = 8;
  char* buf = malloc(s);
  snprintf(buf, s, "%c(0x%02x)", c, c);
  return buf;
}
#endif
