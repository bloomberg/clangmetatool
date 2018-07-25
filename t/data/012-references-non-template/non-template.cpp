int callee(int a);

int caller(int a) {
    return callee(a);
}
