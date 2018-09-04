int foo(int);
int bar();

int main(int argc, char* argv[]) {
  int v1 = 1;
  short v2 = 2;
  long v3 = 3;
  unsigned int v4 = 4;

  while(4 == foo(v1)) {
    v2 = 5;
  }

  for(int i = 0; i < 12; ++i) {
    v3 = 6;
  }

  v1 = 0;

  do {
    v4 = 7;
  } while(foo(v3) == bar());
}
