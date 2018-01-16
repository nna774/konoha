#include <stdio.h>

int main(int argc, char** argv) {
  int var;
  scanf("%d", &var);
  printf("\t.text\n"
         "\t.global mymain\n"
         "mymain:\n"
         "\tmov $%d, %%eax\n"
         "ret\n", var);
  return 0;
}
