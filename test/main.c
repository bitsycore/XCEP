#include <stdio.h>

#include "test_suite.h"
#include "XCEP.h"

void handlerIgnore(const XCEP_t_Exception* inException) {
    printf("Ignoring exception: %d\n", inException->code);
}

int main() {

#if XCEP_CONF_ENABLE_THREAD_SAFE
    SetThreadUncaughtExceptionHandler(handlerIgnore);
    SetThreadUncaughtExceptionHandler(NULL);
#endif

    const t_Exception exception = NewException(100, "Error 100 have been triggered");
    PrintException("Test Exception", &exception);

    return run_test_suit();
}