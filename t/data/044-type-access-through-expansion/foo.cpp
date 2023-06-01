#include "foo.h"
#include "macro.h"

#define CALL_FOO2(v) FOO2(v)

void function() {
  FOO1() f1;
  FOO2(v);
  CALL_FOO2(f3);
}
