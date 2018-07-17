class Foo {
private:
    int a;
    int b;
public:
    int getSum() { return a+b; }
};

class Bar {
private:
    int a;
    int b;
public:
    Bar(int a, int b) : a(a), b(b) {}
};
