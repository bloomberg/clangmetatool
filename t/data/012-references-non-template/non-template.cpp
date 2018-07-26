int global_var = 5;
int global_var_2 = global_var; // captured by varContextMatcher

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
    static int static_local_var = global_var; // captured by both varContextMatcher and funcContextMatcher

    return 0;
}
