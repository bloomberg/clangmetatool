class Foo { // match
private:
  static int uninitialized_static_class_var_of_foo; // (decl isn't matched, but
                                                    // definition is)
  const static int const_initialized_static_class_var_of_foo = 5;
  int member_field_of_foo;

public:
  int member_func_of_foo() {
    return uninitialized_static_class_var_of_foo + member_field_of_foo;
  } // match
};

class Bar { // match
private:
  int a;
  int b;

public:
  Bar(int a, int b) : a(a), b(b) {} // match
};

int Foo::uninitialized_static_class_var_of_foo; // match
int uninitialized_global_var;                   // match
int initialized_global_var = 2;                 // match
