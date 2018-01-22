#include <stdio.h>

extern int mymain(void);

int return42() {
  return 42;
}

int id(int n) {
  return n;
}

int add(int n, int m) {
  return n + m;
}
int add3(int n, int m, int o) {
  return n + m + o;
}
int add4(int n, int m, int o, int p) {
  return n + m + o + p;
}
int add5(int n, int m, int o, int p, int q) {
  return n + m + o + p + q;
}
int add6(int n, int m, int o, int p, int q, int r) {
  return n + m + o + p + q + r;
}

int main(int argc, char **argv) {
  int val = mymain();
  printf("%d\n", val);
  return 0;
}
