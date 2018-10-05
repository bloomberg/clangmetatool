class NakedClass {
public:
  char foo();
  char bar(int *yo);
};

char hurdur(int *, char *, unsigned *);

namespace suit {

class BusinessClass {
public:
  unsigned long book();
  unsigned long long diatribe(char **words) const;
};

} // namespace suit

int foo(NakedClass *nakedC, suit::BusinessClass &businessC) {
  nakedC->foo();
  businessC.book();

  businessC.diatribe(0);
  nakedC->bar(0);

  hurdur(0, 0, 0);

  return 777;
};
