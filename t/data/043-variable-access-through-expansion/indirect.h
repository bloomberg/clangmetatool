#include "paste.h"

#define INDIRECT_PASTE(a, b) (                  \
                              PASTE(a, b)       \
                                                )

// Does not counts as an access to GLOBAL6
#define INDIRECT_REFERENCE_GLOBAL6() INDIRECT_PASTE(GLOB, AL6)
