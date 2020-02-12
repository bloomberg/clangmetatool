#include "includes_and_macros.h"

extern int foobar2(int lol);

SPEC(45)

int foobar(char b) {
  print(b);
  print(yolo45());
  return TO_INT(b);
}

int yolo_ono(int yeet);

int yolo_ono(int yeet) {
  return 1337 * yeet;
}

namespace foo {

int bar() { return 4324; }

}

namespace yolo {

int bar() { return 4325; }

}

int mwahaha() {
  return foobar(23 && "ME");
}

char foo2() {
  return 0;
}

long foo3(long long ll) {
  return 0;
}
