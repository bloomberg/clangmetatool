int foo(int);
int bar(int*);
int qux(int&);

int main(int argc, char* argv[]) {
  int v1 = 0;

  // v1 retains its old value in this current scope
  // Due to pass by value semantics
  // This is ignored by the propagator
  foo(v1);

  v1 = 1;

  // Passed as pointer, v1 might change inside bar(int*)
  bar(&v1);

  v1 = 2;

  qux(v1);

  return 0;
}
