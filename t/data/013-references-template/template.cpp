int global_var = 5;
int global_var_2 = global_var; // captured by varContextMatcher

extern int extern_var;

template <typename T>
T called_func(T cf_arg);

template <typename T>
T called_func_2(T cf2_arg) {
    return cf2_arg;
}

int global_var_3 = called_func_2(1) + global_var; // both deps captured by varContextMatcher

int (*global_uncalled_func)(int) = called_func_2<int>; // captured by varContextMatcher

template <typename T>
T func(T f_arg) {
    int local_var = called_func(1); // captured by funcContextMatcher
    called_func(2); // captured by funcContextMatcher
    int local_var_2 = global_var; // captured by funcContextMatcher
    int local_var_3 = local_var_2;
    int local_var_4 = extern_var; // captured by funcContextMatcher
    static int static_local_var = global_var; // captured by both varContextMatcher and funcContextMatcher

    return f_arg;
}

template <typename T>
struct Foo { // (constructor) captured by funcInRecordMatcher 2x (2 specializations)
    T foo_mem; // captured by fieldInRecordMatcher 3x (one templated, 2 specializations)
};

template <typename T>
class Bar {
private:
    Foo<int> f; // captured by fieldInRecordMatcher
    T bar_mem;
public:
    bool bar_func(Foo<int> foo_param) { // captured by funcInRecordMatcher
        return f.foo_mem == foo_param.foo_mem;
    }
};

void func2() {
    Foo<int> f; // captured by funcContextMatcher and varContextMatcher (TODO : why? this isn't a var context)
}

Foo<bool> global_foo; // not captured (TODO : why? should be captured by varContextMatcher)
