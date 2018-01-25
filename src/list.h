#include <stdlib.h>
#include "utils.h"

#define _LIST_HOLDER_TYPE(Type) \
  struct _list_holder_ ## Type

#define _LIST_HOLDER_OF(Type) \
  _LIST_HOLDER_TYPE(Type)*

#define _LIST_HOLDER_NEW(Type) \
  _new_list_holder_of_ ## Type

#define _LIST_TYPE(Type) \
  struct _list_of_ ## Type

#define LIST_OF(Type) \
  _LIST_TYPE(Type)*

#define DEFINE_LIST(Type) \
  _LIST_HOLDER_TYPE(Type) {\
    Type* val;\
    _LIST_HOLDER_OF(Type) next;\
  };\
\
  _LIST_TYPE(Type) {\
    int count;\
    _LIST_HOLDER_OF(Type) head;\
  };\
\
  LIST_OF(Type) new_list_of_ ## Type();\
  void list_of_ ## Type ## _append(LIST_OF(Type), Type*);\
  int list_of_ ## Type ## _length(LIST_OF(Type));\
  Type* list_of_ ## Type ## _find(LIST_OF(Type), Type*); \
  Type* list_of_ ## Type ## _find_cond(LIST_OF(Type), Type*, bool (*f)(Type const*, Type const*)); \

#define USE_LIST(Type) \
  LIST_OF(Type) new_list_of_ ## Type() {\
    LIST_OF(Type) l = malloc(sizeof(_LIST_TYPE(Type)));\
    l->count = 0;\
    l->head = NULL;\
    return l;\
  }\
\
  _LIST_HOLDER_OF(Type) _LIST_HOLDER_NEW(Type)() {\
    _LIST_HOLDER_OF(Type) h = malloc(sizeof(_LIST_HOLDER_TYPE(Type)));\
    h->val = NULL;\
    h->next = NULL;\
    return h;\
  }\
\
  void list_of_ ## Type ## _append(LIST_OF(Type) l, Type* v) {\
    assert(l != NULL);\
    assert(v != NULL);\
\
    l->count++;\
    _LIST_HOLDER_OF(Type) h = _LIST_HOLDER_NEW(Type)();\
    h->val = v;\
    h->next = NULL;\
    _LIST_HOLDER_OF(Type) hh = l->head;\
    if (hh == NULL) {\
      l->count = 1;\
      l->head = h;\
      return;\
    }\
    int const len = l->count;\
    for(int i = 0; i < len - 1; ++i) {\
      assert(hh != NULL);\
      hh = hh->next;\
    }\
    hh->next = h;\
    l->count++;\
  }\
\
  int list_of_ ## Type ## _length(LIST_OF(Type) l) {\
    return l->count;\
  }\
\
  Type* list_of_ ## Type ## _find(LIST_OF(Type) l, Type* t) {\
    FOREACH(Type, l, e) {\
      if(e == t) return e;\
    }\
    return NULL;\
  }\
\
  Type* list_of_ ## Type ## _find_cond(LIST_OF(Type) l, Type* t, bool (*f)(Type const*, Type const*)) {\
    FOREACH(Type, l, e) {\
      if(f(e, t)) return e;\
    }\
    return NULL;\
  }\

#define FOREACH(Type, list, v) \
  int _loop_count = 0;\
  int const _loop_max = list->count;\
  _LIST_HOLDER_OF(Type) _holder = list->head;\
  Type* v = _holder == NULL ? NULL : _holder->val;\
  for(; _loop_count < _loop_max ; ++_loop_count, _holder = _holder->next, v = _holder == NULL ? NULL : _holder->val)
