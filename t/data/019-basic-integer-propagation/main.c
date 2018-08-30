int foo(int);
int bar(char*);

int main(int argc, char* argv[]) {
  int v1 = 0;

  // v1 retains its old value in this current scope
  foo(v1);

  v1 = 42;

  // These do nothing due to ints being passed by value
  foo(v1);

  // This also doesn't change this scope's v1
  bar(v1);
}
