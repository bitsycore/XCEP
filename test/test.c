#include <stdio.h>
#include <stdlib.h>

#include <Exceptions.h>

enum Exception_Codes {
	EXCEPTION_CODE_FAIL = -1,
	EXCEPTION_CODE_FAIL_ON_2 = -2,
	EXCEPTION_CODE_FAIL_ON_42 = -42,
	EXCEPTION_CODE_FAIL_ON_500 = -500
};

void uncaught_exception_handler_local(const __EXCEPTIONS_t_Exception EXCEPTION) {
	printf("Handler LOCAL !!! Uncaught exception: %d: %s\n", EXCEPTION.code, EXCEPTION.message);
	exit(EXCEPTION.code);
}

void uncaught_exception_handler(const __EXCEPTIONS_t_Exception EXCEPTION) {
	printf("Handler!!! Uncaught exception: %d: %s\n", EXCEPTION.code, EXCEPTION.message);
	exit(EXCEPTION.code);
}

void may_fail_also(const int x) {
	printf("\t\tSTART(also): x=%d\n", x);
	if (x == 2) THROW(EXCEPTION_CODE_FAIL_ON_2, "fail on 2");
	printf("\t\tFINISH(also): x=%d\n", x);
}

void may_fail(const int x) {
	printf("\tSTART(may)\n");
	TRY {
		printf("\tTRY(may): x=%d\n", x);
		if (x == 42) THROW(EXCEPTION_CODE_FAIL_ON_42, "fail on 42");
		may_fail_also(x);
		printf("\tOK(may): x=%d\n", x);
	}
	CATCH_ALL {
		printf("\tCATCH_ALL(may): x=%d, msg=%s, exp=%d\n", x, EXCEPTION.message, EXCEPTION.code);
		RETHROW;
	}
	END_TRY;
	printf("\tFINISH(may): x=%d\n", x);
}

int main() {
	// SET_UNHANDLED_EXCEPTION_HANDLER(uncaught_exception_handler);
	// SET_LOCAL_UNHANDLED_EXCEPTION_HANDLER(uncaught_exception_handler_local);

	printf("START(main)\n");
	TRY {
		printf("TRY(main)\n");
		may_fail(1);
		may_fail(2);
		may_fail(3);
		may_fail(42);
		may_fail(4);
	}
	CATCH(EXCEPTION_CODE_FAIL_ON_42) {
		printf("CATCH(main): msg=%s, exp=%d\n", EXCEPTION.message, EXCEPTION.code);
	}
	FINALLY {
		printf("FINALLY(main)\n");
	}
	END_TRY;

	printf("FINISH(main)\n");
	return 0;
}