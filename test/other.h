#ifndef OTHER_H
#define OTHER_H

#include "XCEP.h"

#define EXCEPTION_TYPE(_code) _code, Exception_Codes_to_string(_code)

enum Exception_Codes {
	EXCEPTION_CODE_FAIL = -1,
	EXCEPTION_CODE_FAIL_ON_2 = -2,
	EXCEPTION_CODE_FAIL_ON_42 = -42,
	EXCEPTION_CODE_FAIL_ON_500 = -500
};

const char* Exception_Codes_to_string(enum Exception_Codes inEnumExceptionCodes);

void Handler_Local(const XCEP_t_Exception* inException);
void Handler(const XCEP_t_Exception* inException);
void may_fail_also(int inX);
void may_fail(int inX);

#endif //OTHER_H
