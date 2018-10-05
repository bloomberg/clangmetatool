int foo(char *);

int bar(char *baz) {
  char *v1 = "I guess some";
  char *v2 = "is copied...";
  char *v3 = "but that's OK!";

  foo(v1);

  v3 = "yep";

  if (foo(v2)) {
    v1 = "oh no!";
  }

  return 1337;
}

int main(int argc, char *argv[]) {
  char *v1 = "hello";
  char *v2 = "this";
  char *v3 = "is";
  char *v4 = "dog";

  if (4 == foo(v1)) {
    v2 = "eat more asparagus";
  } else {
    v3 = "do it";
  }

  v1 = "goodbye";

  foo(v4);
  bar(v1);
}
