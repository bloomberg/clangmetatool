int foo(int);

int main(int argc, char *argv[]) {
  int v1 = 0;

  // v1 retains its old value in this current scope
  foo(v1);

  v1 = 42;

  // These do nothing due to ints being passed by value
  foo(v1);

  v1 += 2;
  v1 *= 3;
  v1 /= 4;
  v1 -= 5;

  // Cannot resolve v1 to a value deterministically (from this point on)
  // The next few instances of unresolved will be compressed into a single one
  v1 = foo(6);

  // All of these are non-deterministic too, due to dependending on a v1's
  // history
  v1 += 1;
  v1 -= 1;
  v1 *= 1;
  v1 /= 1;

  // Restore balance to the universe
  v1 = 42;
  return 0;
}
