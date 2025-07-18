#include <stdio.h>
#include <stdlib.h>

#define XCEP_IMPLEMENTATION
#include <XCEP.h>

enum Exception_Codes {
	EXCEPTION_CODE_FAIL = -1,
	EXCEPTION_CODE_FAIL_ON_2 = -2,
	EXCEPTION_CODE_FAIL_ON_42 = -42,
	EXCEPTION_CODE_FAIL_ON_500 = -500
};

void uncaught_exception_handler_local(const XCEP_t_Exception exception) {
	printf("Handler LOCAL !!! Uncaught exception: %d: %s\n", exception.code, exception.message);
	exit(exception.code);
}

void uncaught_exception_handler(const XCEP_t_Exception exception) {
	printf("Handler!!! Uncaught exception: %d: %s\n", exception.code, exception.message);
	exit(exception.code);
}

void may_fail_also(const int x) {
	printf("\t\tSTART(also): x=%d\n", x);
	if (x == 2) Throw(EXCEPTION_CODE_FAIL_ON_2, "fail on 2");
	printf("\t\tFINISH(also): x=%d\n", x);
}

void may_fail(const int x) {
	printf("\tSTART(may)\n");
	Try {
		printf("\tTRY(may): x=%d\n", x);
		if (x == 42) Throw(EXCEPTION_CODE_FAIL_ON_42, "fail on 42");
		may_fail_also(x);
		Try {
			may_fail_also(2);
		} CatchAll {
			printf("\t\tCATCH_ALL(may): x=%d, msg=%s, exp=%d\n", x, Exception.message, Exception.code);
		}
		EndTry;
		printf("\tOK(may): x=%d\n", x);
	}
	CatchAll {
		printf("\tCATCH_ALL(may): x=%d, msg=%s, exp=%d\n", x, Exception.message, Exception.code);
		Rethrow;
	}
	EndTry;
	printf("\tFINISH(may): x=%d\n", x);
}

int main() {
	XCEP_SetUncaughtExceptionHandler(uncaught_exception_handler);
	//XCEP_SetThreadUncaughtExceptionHandler(uncaught_exception_handler_local);

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