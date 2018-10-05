int foo(int);
int bar(int);

int main(int argc, char *argv[]) {
  int v1 = 1;
  int v2 = 2;
  int v3 = 3;
  int v4 = 4;

  if (4 == foo(v1)) {
    v2 = 5;
  } else {
    v3 = 6;
  }

  v1 = 0;

  foo(v4);
  bar(v1);
}
