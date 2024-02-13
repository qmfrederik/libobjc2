#include <assert.h>
#ifdef __MINGW32__
#include <cxxabi.h>
#else
#include <stdlib.h>
/**
 * Our own definitions of C++ ABI functions and types.  These are provided
 * because this file must not include cxxabi.h.  We need to handle subtly
 * different variations of the ABI and including one specific implementation
 * would make that very difficult.
 */
namespace __cxxabiv1
{
	/**
	 * Type info for classes.  Forward declared because the GNU ABI provides a
	 * method on all type_info objects that the dynamic the dynamic cast header
	 * needs.
	 */
	struct __class_type_info;
}

namespace std
{
	/**
	 * std::type_info, containing the minimum requirements for the ABI.
	 * Public headers on some implementations also expose some implementation
	 * details.  The layout of our subclasses must respect the layout of the
	 * C++ runtime library, but also needs to be portable across multiple
	 * implementations and so should not depend on internal symbols from those
	 * libraries.
	 */
	class type_info
	{
				public:
				virtual ~type_info();
				bool operator==(const type_info &) const;
				bool operator!=(const type_info &) const;
				bool before(const type_info &) const;
				type_info();
				private:
				type_info(const type_info& rhs);
				type_info& operator= (const type_info& rhs);
				const char *__type_name;
				protected:
				type_info(const char *name): __type_name(name) { }
				public:
				const char* name() const { return __type_name; }
	};
}
#endif

using namespace __cxxabiv1;

namespace gnustep
{
	namespace libobjc
	{
		/**
		 * Superclass for the type info for Objective-C exceptions.
		 */
		struct OBJC_PUBLIC __objc_type_info : std::type_info
		{
			/**
			 * Constructor that sets the name.
			 */
			__objc_type_info(const char *name) : type_info(name) {}
			/**
			 * Helper function used by libsupc++ and libcxxrt to determine if
			 * this is a pointer type.  If so, catches automatically
			 * dereference the pointer to the thrown pointer in
			 * `__cxa_begin_catch`.
			 */
			virtual bool __is_pointer_p() const { return true; }
			/**
			 * Helper function used by libsupc++ and libcxxrt to determine if
			 * this is a function pointer type.  Irrelevant for our purposes.
			 */
			virtual bool __is_function_p() const { return false; }
			/**
			 * Catch handler.  This is used in the C++ personality function.
			 * `thrown_type` is the type info of the thrown object, `this` is
			 * the type info at the catch site.  `thrown_object` is a pointer
			 * to a pointer to the thrown object and may be adjusted by this
			 * function.
			 */
			virtual bool __do_catch(const type_info *thrown_type,
			                        void **thrown_object,
			                        unsigned) const
			{
				assert(0);
				return false;
			};
			/**
			 * Function used for `dynamic_cast` between two C++ class types in
			 * libsupc++ and libcxxrt.
			 *
			 * This should never be called on Objective-C types.
			 */
			virtual bool __do_upcast(
			                const __class_type_info *target,
			                void **thrown_object) const
			{
				return false;
			};
		};

		/**
		 * Singleton type info for the `id` type.
		 */
		struct OBJC_PUBLIC __objc_id_type_info : __objc_type_info
		{
			/**
			 * The `id` type is mangled to `@id`, which is not a valid mangling
			 * of anything else.
			 */
			__objc_id_type_info() : __objc_type_info("@id") {};
			virtual ~__objc_id_type_info();
			virtual bool __do_catch(const type_info *thrownType,
			                        void **obj,
			                        unsigned outer) const;
		};
		struct OBJC_PUBLIC __objc_class_type_info : __objc_type_info
		{
			virtual ~__objc_class_type_info();
			virtual bool __do_catch(const type_info *thrownType,
			                        void **obj,
			                        unsigned outer) const;
		};
		
		id dereference_thrown_object_pointer(void** obj);
    }
}