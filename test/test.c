#define XCEP_IMPLEMENTATION
#include <XCEP.h>

#include "other.h"

int main() {
	XCEP_SetUncaughtExceptionHandler(Handler);
	XCEP_SetThreadUncaughtExceptionHandler(Handler_Local);

	printf("START(main)\n");
	Try {
		printf("TRY(main)\n");
		may_fail(1);
		may_fail(2);
		may_fail(3);
		may_fail(42);
		may_fail(4);
	}
	Catch(EXCEPTION_CODE_FAIL_ON_42) {
		printf("CATCH(main): msg=%s, exp=%d\n", Exception.message, Exception.code);
	}
	Finally {
		printf("FINALLY(main)\n");
	}
	EndTry;

	printf("FINISH(main)\n");
	return 0;
}
