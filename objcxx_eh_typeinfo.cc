
#include "objc/runtime.h"
#include "objc/objc-arc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "objcxx_eh_typeinfo.h"

#ifndef DEBUG_EXCEPTIONS
#define DEBUG_LOG(...)
#else
#define DEBUG_LOG(str, ...) fprintf(stderr, str, ## __VA_ARGS__)
#endif

using namespace std;
using namespace __cxxabiv1;

static BOOL isKindOfClass(Class thrown, Class type)
{
	do
	{
		if (thrown == type)
		{
			return YES;
		}
		thrown = class_getSuperclass(thrown);
	} while (Nil != thrown);

	return NO;
}


static bool AppleCompatibleMode = true;
extern "C" int objc_set_apple_compatible_objcxx_exceptions(int newValue)
{
	bool old = AppleCompatibleMode;
	AppleCompatibleMode = newValue;
	return old;
}

gnustep::libobjc::__objc_class_type_info::~__objc_class_type_info() {}
gnustep::libobjc::__objc_id_type_info::~__objc_id_type_info() {}
bool gnustep::libobjc::__objc_class_type_info::__do_catch(const type_info *thrownType,
                                                          void **obj,
                                                          unsigned outer) const
{
	id thrown = nullptr;
	bool found = false;
	// Id throw matches any ObjC catch.  This may be a silly idea!
	if (dynamic_cast<const __objc_id_type_info*>(thrownType)
	    || (AppleCompatibleMode && 
	        dynamic_cast<const __objc_class_type_info*>(thrownType)))
	{
		thrown = dereference_thrown_object_pointer(obj);
		// nil only matches id catch handlers in Apple-compatible mode, or when thrown as an id
		if (0 == thrown)
		{
			return false;
		}
		// Check whether the real thrown object matches the catch type.
		found = isKindOfClass(object_getClass(thrown),
		                      (Class)objc_getClass(name()));
	}
	else if (dynamic_cast<const __objc_class_type_info*>(thrownType))
	{
		thrown = dereference_thrown_object_pointer(obj);
		found = isKindOfClass((Class)objc_getClass(thrownType->name()),
		                      (Class)objc_getClass(name()));
	}
	if (found)
	{
		*obj = (void*)thrown;
	}

	return found;
};

bool gnustep::libobjc::__objc_id_type_info::__do_catch(const type_info *thrownType,
                                                       void **obj,
                                                       unsigned outer) const
{
	// Id catch matches any ObjC throw
	if (dynamic_cast<const __objc_class_type_info*>(thrownType))
	{
		*obj = dereference_thrown_object_pointer(obj);
		DEBUG_LOG("gnustep::libobjc::__objc_id_type_info::__do_catch caught 0x%x\n", *obj);
		return true;
	}
	if (dynamic_cast<const __objc_id_type_info*>(thrownType))
	{
		*obj = dereference_thrown_object_pointer(obj);
		DEBUG_LOG("gnustep::libobjc::__objc_id_type_info::__do_catch caught 0x%x\n", *obj);
		return true;
	}
	DEBUG_LOG("gnustep::libobjc::__objc_id_type_info::__do_catch returning false\n");
	return false;
};