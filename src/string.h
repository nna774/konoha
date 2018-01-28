#ifndef NNA774_KONOHA_STRING_H
#define NNA774_KONOHA_STRING_H

#include <stddef.h>
#include "list.h"

struct String;
typedef struct String String;

DEFINE_INTRUSIVE_LIST(String);

struct _String_impl;
struct String {
  struct _String_impl* _si;
  INTRUSIVE_LIST_HOOK(String);
};

String new_String();
String to_String(size_t, char*);
String from_char(char);
void append_char(String, char);
size_t String_length(String const);
char const* c_str(String const);

#endif // NNA774_KONOHA_STRING_H
