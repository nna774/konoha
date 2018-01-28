#include <ctype.h>
#include "enum.h"

void _setup_show_enum(char* table, size_t* map) {
  size_t i = 0;
  for (char* it = table + 1; *it; ++it) {
    if (isalnum(*it) || *it == '_') {
      if (*(it - 1) == '\0') {
        map[++i] = it - table;
      }
    } else {
      *it = '\0';
    }
  }
}
