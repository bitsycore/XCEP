#include "other.h"

#include <stdio.h>
#include <stdlib.h>

#include "XCEP.h"

const char* Exception_Codes_to_string(const enum Exception_Codes e) {
	switch (e) {
	case EXCEPTION_CODE_FAIL: return "EXCEPTION_CODE_FAIL";
	case EXCEPTION_CODE_FAIL_ON_2: return "EXCEPTION_CODE_FAIL_ON_2";
	case EXCEPTION_CODE_FAIL_ON_42: return "EXCEPTION_CODE_FAIL_ON_42";
	case EXCEPTION_CODE_FAIL_ON_500: return "EXCEPTION_CODE_FAIL_ON_500";
	default: return "unknown";
	}
}

void Handler_Local(const XCEP_t_Exception *exception) {
	XCEP_PrintException("Handler Local, Fatal Exception", exception);
	exit(exception->code);
}

void Handler(const XCEP_t_Exception *exception) {
	XCEP_PrintException("Handler, Fatal Exception", exception);
	exit(exception->code);
}

void may_fail_also(const int x) {
	printf("\t\tSTART(also): x=%d\n", x);
	if (x == 2) Throw(EXCEPTION_CODE_FAIL_ON_2, Exception_Codes_to_string(EXCEPTION_CODE_FAIL_ON_2));
	printf("\t\tFINISH(also): x=%d\n", x);
}

void may_fail(const int x) {
	printf("\tSTART(may)\n");
	Try {
		printf("\tTRY(may): x=%d\n", x);
		if (x == 42) Throw(EXCEPTION_CODE_FAIL_ON_42, Exception_Codes_to_string(EXCEPTION_CODE_FAIL_ON_42));
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