int foo(char*);

int main(int argc, char* argv[]) {
  char* v1 = "hello, world!";

  foo(v1);

  v1 = "#amazingAssignment";

  foo(v1);
  foo(v1);
  foo(v1);
}
