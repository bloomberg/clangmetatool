#include "b.h"

int zee()
{
  B::Y var;
  const B::Y &var2 = var;
  const B::Y &var3 = var;
  return 0;
}
