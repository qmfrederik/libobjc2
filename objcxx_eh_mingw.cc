
#include "objc/runtime.h"
#include "objc/objc-arc.h"
#include "objc/objc-exception.h"
#include "objc/hooks.h"
#include <unwind.h>
#include <exception>
#include <bits/atomic_word.h>
#include <stdio.h>
#include <stdlib.h>
#include "objcxx_eh_typeinfo.h"

#define DEBUG_EXCEPTIONS

#ifndef DEBUG_EXCEPTIONS
#define DEBUG_LOG(...)
#else
#define DEBUG_LOG(str, ...) fprintf(stderr, str, ## __VA_ARGS__)
#endif

/**
 * Public interface to the Objective-C++ exception mechanism
 */
extern "C"
{
    /**
     * The public symbol that the compiler uses to indicate the Objective-C id type.
     */
    OBJC_PUBLIC gnustep::libobjc::__objc_id_type_info __objc_id_type_info;
} // extern "C"

// https://github.com/gcc-mirror/gcc/blob/2ca373b7e8adf9cc0c17aecab5e1cc6c76a92f4c/libstdc%2B%2B-v3/libsupc%2B%2B/unwind-cxx.h#L100
namespace __cxxabiv1
{
    // Each thread in a C++ program has access to a __cxa_eh_globals object.
    struct __cxa_eh_globals
    {
    __cxa_exception *caughtExceptions;
    unsigned int uncaughtExceptions;
    #ifdef __ARM_EABI_UNWINDER__
    __cxa_exception* propagatingExceptions;
    #endif
    };

    struct __cxa_exception
    {
    // Manage the exception object itself.
    std::type_info *exceptionType;
    void (_GLIBCXX_CDTOR_CALLABI *exceptionDestructor)(void *);

    // The C++ standard has entertaining rules wrt calling set_terminate
    // and set_unexpected in the middle of the exception cleanup process.
    std::terminate_handler unexpectedHandler;
    std::terminate_handler terminateHandler;

    // The caught exception stack threads through here.
    __cxa_exception *nextException;

    // How many nested handlers have caught this exception.  A negated
    // value is a signal that this object has been rethrown.
    int handlerCount;

    #ifdef __ARM_EABI_UNWINDER__
    // Stack of exceptions in cleanups.
    __cxa_exception* nextPropagatingException;

    // The number of active cleanup handlers for this exception.
    int propagationCount;
    #else
    // Cache parsed handler data from the personality routine Phase 1
    // for Phase 2 and __cxa_call_unexpected.
    int handlerSwitchValue;
    const unsigned char *actionRecord;
    const unsigned char *languageSpecificData;
    _Unwind_Ptr catchTemp;
    void *adjustedPtr;
    #endif

    // The generic exception header.  Must be last.
    _Unwind_Exception unwindHeader;
    };

    struct __cxa_refcounted_exception
    {
        // Manage this header.
        _Atomic_word referenceCount;
        // __cxa_exception must be last, and no padding can be after it.
        __cxa_exception exc;
    };
}

namespace gnustep
{
	namespace libobjc
	{
        id dereference_thrown_object_pointer(void** obj) {
            /* libc++-abi does not have  __is_pointer_p and won't do the double dereference 
            * required to get the object pointer. We need to do it ourselves if we have
            * caught an exception with libc++'s exception class. */
    #ifdef _LIBCPP_VERSION
            return **(id**)obj;
    #else
            return *(id*)obj;
    #endif // _LIBCPP_VERSION
        }
    }
};

using namespace __cxxabiv1;

static void eh_cleanup(void *exception)
{
	DEBUG_LOG("eh_cleanup: Releasing 0x%x\n", *(id*)exception);
	objc_release(*(id*)exception);
}

extern "C"
OBJC_PUBLIC
void objc_exception_throw(id object)
{
	id *exc = (id *)__cxxabiv1::__cxa_allocate_exception(sizeof(id));
	*exc = object;
	objc_retain(object);
	DEBUG_LOG("objc_exception_throw: Throwing 0x%x\n", *exc);

    __cxa_eh_globals *globals = __cxa_get_globals ();
    globals->uncaughtExceptions += 1;
    __cxa_refcounted_exception *header =
        __cxa_init_primary_exception(exc, & __objc_id_type_info, eh_cleanup);
    header->referenceCount = 1;

    _Unwind_Reason_Code err = _Unwind_RaiseException (&header->exc.unwindHeader);

	DEBUG_LOG("objc_exception_throw: _Unwind_RaiseException returned 0x%x for exception 0x%x\n", err, *exc);

	if (_URC_END_OF_STACK == err && 0 != _objc_unexpected_exception)
	{
	    DEBUG_LOG("Invoking _objc_unexpected_exception\n");
		_objc_unexpected_exception(object);
	}
	DEBUG_LOG("Throw returned %d\n",(int) err);
	abort();
}

OBJC_PUBLIC extern objc_uncaught_exception_handler objc_setUncaughtExceptionHandler(objc_uncaught_exception_handler handler)
{
	objc_uncaught_exception_handler previousHandler = __atomic_exchange_n(&_objc_unexpected_exception, handler, __ATOMIC_SEQ_CST);
	return previousHandler;
}
