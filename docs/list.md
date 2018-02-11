# list

## Declaration

declar in header

```
DEFINE_INTRUSIVE_LIST(T)
```

## Definition

define in `use_list.c`

```
USE_INTRUSIVE_LIST(Type)
```

## use

add list to hook

```
struct Elem {
  ...(Elem members);
  INTRUSIVE_LIST_HOOK(Elem)
}
```

and use it(create by `INTRUSIVE_LIST_OF(T) new_list_of_T()`)

```
INTRUSIVE_LIST_OF(Elem) = new_list_of_Elem();
```

T(inserted to list) shuoud be initilalize its hook by `void init_T_hook(T*)`

```
Elem* e = malloc(sizeof(Elem));
init_Elem_hook(e);
```

useful functions

* `void list_of_T_append(INTRUSIVE_LIST_OF(T) l, T* e)`
  appent `e` to tail of `l`(O(len(l)))

* `int list_of_T_length(INTRUSIVE_LIST_OF(T) l)`
  length of `l`(O(1))

* `T* list_of_T_find(INTRUSIVE_LIST_OF(T) l, T* e)`
  find element with same address of `e`(O(n))

* `T* list_of_T_find_cond(INTRUSIVE_LIST_OF(T) l, T* e, bool(*f)(T const*, T const*))`
  find element with same of `e`(if `f` returns `true`, it means same object)(O(n))

* `T* list_of_T_pop(INTRUSIVE_LIST_OF(T) l)`
  pop elem from front of `l`(if l is empty, return NULL)(O(1))

* `void list_of_T_push(INTRUSIVE_LIST_OF(T) l, T* e)`
  push `e` to fromt of `l`(O(1))

useful macros

* `FOREACH(T, l, v)`
  `l` is `INTRUSIVE_LIST_OF(T)`
  `v` is name of parameter(`T*`)

```
void f(Elem*);
INTRUSIVE_LIST_OF(Elem) list = ...;

FOREACH(Elem, list, v) {
  f(v);
}
```
