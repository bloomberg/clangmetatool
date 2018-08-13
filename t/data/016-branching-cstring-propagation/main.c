int foo(char*);
int bar(char*);

int main(int argc, char* argv[]) {
  char* v1 = "hello";
  char* v2 = "this";
  char* v3 = "is";
  char* v4 = "dog";

  if(4 == foo(v1)) {
    v2 = "eat more asparagus";
  } else {
    v3 = "do it";
  }

  v1 = "goodbye";

  foo(v4);
  bar(v1);
}
