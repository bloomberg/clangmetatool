#define TO_INT(b) (int) b

void print(char b);

#define SPEC(a) \
  int yolo ## a() { return a; }
