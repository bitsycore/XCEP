#ifndef OTHER_H
#define OTHER_H

#include "XCEP.h"

enum Exception_Codes {
	EXCEPTION_CODE_FAIL = -1,
	EXCEPTION_CODE_FAIL_ON_2 = -2,
	EXCEPTION_CODE_FAIL_ON_42 = -42,
	EXCEPTION_CODE_FAIL_ON_500 = -500
};

void Handler_Local(XCEP_t_Exception exception);
void Handler(XCEP_t_Exception exception);
void may_fail_also(int x);
void may_fail(int x);

#endif //OTHER_H
