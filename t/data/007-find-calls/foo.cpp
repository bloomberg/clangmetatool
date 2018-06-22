
  bool bar(long double a) {
      float b;
      a += 1.0;
      return true;
  }

  int foo(int a, int b) {
      long double f = 1.00;
      bool g;
      g = bar(f);
      return 0;
  }
