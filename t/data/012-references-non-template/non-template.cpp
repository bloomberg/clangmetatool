int global_var = 5;
int global_var_2 = global_var; // captured by varContextMatcher

extern int extern_var;

int called_func(int cf_arg);

int called_func_2(int cf2_arg) {
    return 350;
}

int global_var_3 = called_func_2(1) + global_var; // both deps captured by varContextMatcher

int (*global_uncalled_func)(int) = called_func_2; // captured by varContextMatcher

int func(int f_arg) {
    int local_var = called_func(1); // captured by funcContextMatcher
    called_func(2); // captured by funcContextMatcher
    int local_var_2 = global_var; // captured by funcContextMatcher
    int local_var_3 = local_var_2;
    int local_var_4 = extern_var; // captured by funcContextMatcher
    static int static_local_var = global_var; // captured by both varContextMatcher and funcContextMatcher

    return 0;
}

struct Foo {
    int foo_mem;
};

class Bar {
private:
    Foo f;
public:
    bool bar_func(Foo foo_param) {
        return f.foo_mem == foo_param.foo_mem;
    }
};

void func2() {
    Foo f; // captured by funcContextMatcher
}

Foo global_foo; // captured by varContextMatcher
