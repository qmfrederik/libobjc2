#include "Test.h"

#if __cplusplus
#error This is not an ObjC++ test!
#endif

int main(void)
{
	BOOL exceptionCaught = NO;

	@try
	{
		HelperClass* helper = [HelperClass alloc];
		[helper helperMethodWhichThrows];
	}
	@catch (Test* localException2)
	{
		exceptionCaught = YES;
	}

	assert(exceptionCaught == YES);
	exceptionCaught = NO;

	@try
	{
		HelperClass* helper = [HelperClass alloc];
		[helper helperMethodWhichRethrows];
	}
	@catch (Test* localException2)
	{
		exceptionCaught = YES;
	}

	assert(exceptionCaught == YES);

	return 0;
}
