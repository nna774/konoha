#include <stdlib.h>
#include "utils.h"

#define INTRUSIVE_LIST_HOOK(Type) \
  struct {\
    Type* next;\
  } _hook;

#define INTRUSIVE_LIST_TYPE(Type) \
  struct _list_of_ ## Type

#define INTRUSIVE_LIST_OF(Type) \
  INTRUSIVE_LIST_TYPE(Type)*

#define DEFINE_INTRUSIVE_LIST(Type) \
  INTRUSIVE_LIST_TYPE(Type) {\
    int count;\
    Type* head;\
  };\
\
  INTRUSIVE_LIST_OF(Type) new_list_of_ ## Type();\
  void init_ ## Type ## _hook(Type*);\
  void list_of_ ## Type ## _append(INTRUSIVE_LIST_OF(Type), Type*);\
  int list_of_ ## Type ## _length(INTRUSIVE_LIST_OF(Type));\
  Type* list_of_ ## Type ## _find(INTRUSIVE_LIST_OF(Type), Type*); \
  Type* list_of_ ## Type ## _find_cond(INTRUSIVE_LIST_OF(Type), Type*, bool (*f)(Type const*, Type const*)); \

#define USE_INTRUSIVE_LIST(Type) \
  struct _list_of_ ## Type* new_list_of_ ## Type() {\
    INTRUSIVE_LIST_OF(Type) l = malloc(sizeof(INTRUSIVE_LIST_TYPE(Type)));\
    l->count = 0;\
    l->head = NULL;\
    return l;\
  }\
\
  void init_ ## Type ## _hook(Type* t) { \
    t->_hook.next = NULL;\
  }\
\
  void list_of_ ## Type ## _append(INTRUSIVE_LIST_OF(Type) l, Type* app) {\
    assert(l != NULL);\
    assert(app != NULL);\
\
    l->count++;\
    app->_hook.next = NULL;\
    Type* v = l->head;\
    if (v == NULL) {\
      l->head = app;\
      return;\
    }\
    Type* pre;\
    while(v != NULL) {\
      pre = v;\
      v = v->_hook.next;\
    }\
    pre->_hook.next = app;\
  }\
\
  int list_of_ ## Type ## _length(INTRUSIVE_LIST_OF(Type) l) {\
    return l->count;\
  }\
\
  Type* list_of_ ## Type ## _find(INTRUSIVE_LIST_OF(Type) l, Type* t) {\
    FOREACH(Type, l, e) {\
      if(e == t) return e;\
    }\
    return NULL;\
  }\
\
  Type* list_of_ ## Type ## _find_cond(INTRUSIVE_LIST_OF(Type) l, Type* t, bool (*f)(Type const*, Type const*)) {\
    FOREACH(Type, l, e) {\
      if(f(e, t)) return e;\
    }\
    return NULL;\
  }\

#define FOREACH(Type, list, val) \
  for(Type* val = list->head; val != NULL; val = val->_hook.next)
