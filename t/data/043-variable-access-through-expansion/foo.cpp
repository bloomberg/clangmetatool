#include "paste.h"
#include "macro.h"
#include "global.h"

int bar()
{
  return PASTE(GLO, BAL1) +  PASTE(GL, OBAL2);
}

int baz()
{
  return ANOTHER_PASTE(GL, OBAL4) +  REFERENCE_GLOBAL3();
}
