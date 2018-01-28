#include <stdlib.h>
#include "string.h"

struct _String_impl {
  size_t length;
  size_t capacity;
  char* top;
};

int const DEFAULT_CAPACITY = 4;

struct _String_impl* new_si() {
  return malloc(sizeof(struct _String_impl));
}

String new_String() {
  String str = {};
  str._si = new_si();
  str._si->length = 0;
  str._si->capacity = DEFAULT_CAPACITY;
  str._si->top = malloc(DEFAULT_CAPACITY + 1);
  str._si->top[0] = '\0';
  init_String_hook(&str);
  return str;
}

String to_String(size_t size, char* s) {
  String str = {};
  str._si = new_si();
  str._si->length = size;
  str._si->capacity = size;
  str._si->top = s;
  init_String_hook(&str);
  return str;
}

String from_char(char c) {
  String str = {};
  str._si = new_si();
  str._si->length = 1;
  str._si->capacity = 1;
  char* buf = malloc(2);
  buf[0] = c;
  buf[1] = '\0';
  str._si->top = buf;
  init_String_hook(&str);
  return str;
}

void append_char(String s, char c) {
  if(s._si->length + 1 < s._si->capacity) {
    s._si->top[s._si->length] = c;
    ++s._si->length;
    s._si->top[s._si->length] = '\0';
    return;
  }
  s._si->capacity *= 2;
  char* newbuf = realloc(s._si->top, s._si->capacity);
  if(newbuf != NULL) {
    s._si->top = newbuf;
    s._si->top[s._si->length] = c;
    ++s._si->length;
    s._si->top[s._si->length] = '\0';
    return;
  }
  warn("append char: realloc failed(maybe unrecoverable)");
  return;
}

size_t String_length(String const s) {
  return s._si->length;
}

char const* c_str(String const s) {
  return s._si->top;
}
