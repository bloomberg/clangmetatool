int foo(char*);
int bar();

int main(int argc, char* argv[]) {
  char* v1 = "wham";

  while(7777777 == bar()) {
    v1 = "bam";
  }

  for(int i = 0; i < 34; --i) {
    foo(v1);
  }

  v1 = "thank you mam!";

  do {
    foo("- Abraham Lincoln");
  } while(foo(v1));
}
