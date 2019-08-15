#include "warnings.h"

void foo()
{
  int value = 1;
  short narrowing[1] = {value};
}
