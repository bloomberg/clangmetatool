#include <stdarg.h>

int foo(int, int*, int&, ...);

int main(int argc, char* argv[]) {
  int a = 0;
  int b = 1;
  int c = 2;
  int d = 3;
  int e = 4;

  foo(a, &b, c, d, &e);

  return 0;
}
