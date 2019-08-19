int foo(char*);

int main() {
  char *v1 = "It's over 9000";
  foo(v1);

  v1 = "I am the walrus";
  foo(v1);

  v1 = 0;
  foo(v1);

  return 0;
}
