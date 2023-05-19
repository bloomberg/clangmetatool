#include "paste.h"
#include "macro.h"
#include "global.h"
#include "indirect.h"

#define REFERENCE_GLOBAL1() GLOBAL1

int bar()
{
  return PASTE(GLO, BAL1) // 1st use of global.h
    +  PASTE(GL, OBAL2)   // 2nd use of global.h
    + REFERENCE_GLOBAL1() // 3rd use of global.h (no scratch space)
    ;
}

int baz()
{
  return ANOTHER_PASTE(GL, OBAL4) // 4th use of global.h
    +  REFERENCE_GLOBAL3()        // not a use of global.h (no scratch space)
    ;
}

int indirect()
{
  return INDIRECT_PASTE(GL, OBAL5);  // 5th use of global.h
}


int indirect2()
{
  return INDIRECT_REFERENCE_GLOBAL6(); // 6th use of global.h
}
