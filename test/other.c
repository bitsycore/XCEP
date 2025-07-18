#include "other.h"

#include <stdio.h>
#include <stdlib.h>

#include "XCEP.h"

void Handler_Local(const XCEP_t_Exception exception) {
	XCEP_PrintException("Handler Local, Fatal Exception", exception);
	exit(exception.code);
}

void Handler(const XCEP_t_Exception exception) {
	XCEP_PrintException("Handler, Fatal Exception", exception);
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