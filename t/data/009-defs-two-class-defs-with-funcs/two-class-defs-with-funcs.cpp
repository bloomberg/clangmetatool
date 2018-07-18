class Foo {
private:
    static int uninitialized_static_class_var_of_foo;
    const static int initialized_static_class_var_of_foo = 5;
    int member_field_of_foo;
public:
    int member_func_of_foo() { return uninitialized_static_class_var_of_foo + member_field_of_foo; }
};

class Bar {
private:
    int a;
    int b;
public:
    Bar(int a, int b) : a(a), b(b) {}
};

int uninitialized_global_var;
int initialized_global_var = 2;
