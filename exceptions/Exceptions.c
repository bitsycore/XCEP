#include "Exceptions.h"

XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack = NULL;
#if XCEP_CONF_ENABLE_THREAD_SAFE
XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;