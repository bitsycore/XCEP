#include "test_suite.h"
#include "XCEP.h"

int main() {
#if XCEP_CONF_ENABLE_THREAD_SAFE
    SetThreadUncaughtExceptionHandler(XCEP_g_UncaughtExceptionHandler);
#endif
    SetThreadUncaughtExceptionHandler(NULL);
    XCEP_t_Exception exception = { .code = 100, .message = "Test Exception", .line = 100, .file = "main.c", .function = "main"};
    PrintException("Test Exception", &exception);
    return run_test_suit();
}