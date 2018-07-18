  bool bar(long double a) {
      float b;
      a += 1.0;
      return true;
  }

  int foo(int a, int b) {
      int fi = 1;
      float ff = 1.00;
      double fd = 1.00;
      bool fb;
      bar(fi);
      return 0;
  }
