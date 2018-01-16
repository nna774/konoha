#include <stdio.h>

int main(int argc, char** argv) {
  printf("\t.text\n"
         "\t.global main\n"
         "main:\n"
         "\tmov $42, %%eax\n"
         "ret\n");
  return 0;
}
