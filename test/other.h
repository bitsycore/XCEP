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

const char* Exception_Codes_to_string(enum Exception_Codes e);

void Handler_Local(const XCEP_t_Exception* exception);
void Handler(const XCEP_t_Exception* exception);
void may_fail_also(int x);
void may_fail(int x);

#endif //OTHER_H
