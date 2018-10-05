struct NakedStruct {
  int foo();
  int bar(char *yo);
};

class NakedClass {
public:
  char foo();
  char bar(int *yo);
};

namespace suit {

struct BusinessStruct {
  long foo();
  long long bar(int *drinks);
};

class BusinessClass {
public:
  unsigned long book();
  unsigned long long diatribe(char **words) const;
};

} // namespace suit

int foo(NakedStruct &nakedS, NakedClass *nakedC,
        suit::BusinessStruct *businessS, suit::BusinessClass &businessC) {
  nakedS.foo();
  nakedC->foo();
  businessS->foo();
  businessC.book();

  businessC.diatribe(0);
  businessS->bar(0);
  nakedC->bar(0);
  nakedS.bar(0);

  return 777;
};
