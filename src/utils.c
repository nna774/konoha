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

void warn(char const* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
