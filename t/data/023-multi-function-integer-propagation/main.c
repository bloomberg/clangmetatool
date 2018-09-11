int foo(int);

int bar(int baz) {
  int v1 = 10;
  int v2 = 11;
  int v3 = 12;

  foo(v1);

  v3 = 13;

  if(foo(v2)) {
    v1 = 14;
  }

  return 1337;
}

int main(int argc, char* argv[]) {
  int v1 = 1;
  int v2 = 2;
  int v3 = 3;
  int v4 = 4;

  if(4 == foo(v1)) {
    v2 = 5;
  } else {
    v3 = 6;
  }

  v1 = 0;

  foo(v4);
  bar(v1);
}
