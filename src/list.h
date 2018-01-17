#include <stdlib.h>

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
  void list_of_ ## Type ## _append(INTRUSIVE_LIST_OF(Type), Type*);\
  INTRUSIVE_LIST_OF(Type) new_list_of_ ## Type();\
  void init_ ## Type ## _hook(Type*);\

#define USE_INTRUSIVE_LIST(Type) \
  void list_of_ ## Type ## _append(INTRUSIVE_LIST_OF(Type) l, Type* app) {\
    assert(l != NULL);\
\
    l->count++;\
    Type* v = l->head;\
    if (v == NULL) {\
      l->head = app;\
      return;\
    }\
    Type* pre;\
    while(v != NULL) {\
      v = v->_hook.next;\
      pre = v;\
    }\
    app->_hook.next = NULL;\
    pre->_hook.next = app;\
  }\
\
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

#define FOREACH(Type, list, val) \
  for(Type* val = list->head; val != NULL; val = val->_hook.next)
