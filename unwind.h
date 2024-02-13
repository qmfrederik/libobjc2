#ifdef __MINWGW32__
#error Oops
#endif

#ifdef __arm__
#include "unwind-arm.h"
#else
#include "unwind-itanium.h"
#endif
