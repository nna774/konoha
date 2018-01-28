#ifndef NNA774_KONOHA_ENUM_H
#define NNA774_KONOHA_ENUM_H

#include "utils.h"

void _setup_show_enum(char*, size_t*);

#define _ENUM_COUNT_OF(Type) CONCAT(_ENUM_COUNT_OF_, Type)

#ifdef ENUM_SHOW_DEFINE
#define ENUM_WITH_SHOW(Type, ...) \
  enum Type { __VA_ARGS__, _ENUM_COUNT_OF(Type) };\
  typedef enum Type Type;\
  char const* CONCAT(show_, Type) (enum Type x) {\
    static char table[] = #__VA_ARGS__;\
    static size_t map[_ENUM_COUNT_OF(Type)] = {0};\
    static bool is_first = 1;\
    if (is_first) {\
      _setup_show_enum(table, map);\
      is_first = 0;\
    }\
    return &table[map[x]];\
  }
#else
#define ENUM_WITH_SHOW(Type, ...) \
  enum Type { __VA_ARGS__, _ENUM_COUNT_OF(Type) };\
  typedef enum Type Type;\
  char const* CONCAT(show_, Type) (enum Type x);
#endif

#endif // NNA774_KONOHA_ENUM_H
