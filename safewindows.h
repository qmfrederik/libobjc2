#ifndef __LIBOBJC_SAFEWINDOWS_H_INCLUDED__
#define __LIBOBJC_SAFEWINDOWS_H_INCLUDED__

#pragma push_macro("BOOL")

// Define _NO_BOOL_TYPEDEF so minwindef.h doesn't re-declare BOOL
#ifndef _NO_BOOL_TYPEDEF
#define _NO_BOOL_TYPEDEF
#endif

#include <Windows.h>

// Windows.h defines interface -> struct
#ifdef interface
#undef interface
#endif

#pragma pop_macro("BOOL")

#endif // __LIBOBJC_SAFEWINDOWS_H_INCLUDED__
