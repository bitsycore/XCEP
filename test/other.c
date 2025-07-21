#include "other.h"

#include <stdio.h>
#include <stdlib.h>

#include "XCEP.h"

const char* Exception_Codes_to_string(const enum Exception_Codes inEnumExceptionCodes) {
	switch (inEnumExceptionCodes) {
        case EXCEPTION_CODE_FAIL: return "EXCEPTION_CODE_FAIL";
        case EXCEPTION_CODE_FAIL_ON_2: return "EXCEPTION_CODE_FAIL_ON_2";
        case EXCEPTION_CODE_FAIL_ON_42: return "EXCEPTION_CODE_FAIL_ON_42";
        case EXCEPTION_CODE_FAIL_ON_500: return "EXCEPTION_CODE_FAIL_ON_500";
	    default: return "unknown";
	}
}

void Handler_Local(const XCEP_t_Exception *inException) {
	PrintException("Handler Local, Fatal Exception", inException);
	exit(inException->code);
}

void Handler(const XCEP_t_Exception *inException) {
	PrintException("Handler, Fatal Exception", inException);
	exit(inException->code);
}

void may_fail_also(const int inX) {
	printf("\t\tSTART(also): inX=%d\n", inX);
	if (inX == 2) Throw(EXCEPTION_CODE_FAIL_ON_2, Exception_Codes_to_string(EXCEPTION_CODE_FAIL_ON_2));
	printf("\t\tFINISH(also): inX=%d\n", inX);
}

void may_fail(const int inX) {
	printf("\tSTART(may)\n");
	Try {
		printf("\tTRY(may): inX=%d\n", inX);
		if (inX == 42) Throw(EXCEPTION_CODE_FAIL_ON_42, Exception_Codes_to_string(EXCEPTION_CODE_FAIL_ON_42));
		may_fail_also(inX);
		Try {
			may_fail_also(2);
		} CatchAll {
			printf("\t\tCATCH_ALL(may): inX=%d, msg=%s, exp=%d\n", inX, Exception.message, Exception.code);
		}
		EndTry;
		printf("\tOK(may): inX=%d\n", inX);
	}
	CatchAll {
		printf("\tCATCH_ALL(may): inX=%d, msg=%s, exp=%d\n", inX, Exception.message, Exception.code);
		Rethrow;
	}
	EndTry;
	printf("\tFINISH(may): inX=%d\n", inX);
}