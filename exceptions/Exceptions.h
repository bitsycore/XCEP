#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <setjmp.h>
#include <stddef.h>

// =========================================================
// MARK: Configuration
// =========================================================

#define XCEP_CONF_ENABLE_THREAD_SAFE 1
#define XCEP_CONF_ENABLE_SHORT_COMMANDS 1

// =========================================================
// MARK: Utilities
// =========================================================

#if !XCEP_CONF_ENABLE_THREAD_SAFE
	#define XCEP_THREAD_LOCAL
#elif defined(_MSC_VER)
	#define XCEP_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
	#if __STDC_VERSION__ >= 201112L
		#define XCEP_THREAD_LOCAL _Thread_local
	#else
		#define XCEP_THREAD_LOCAL __thread
	#endif
#else
	#define XCEP_THREAD_LOCAL thread_local
#endif

// =========================================================
// MARK: Types
// =========================================================

typedef struct {
	int code;
	const char* message;
	int line;
	const char* file;
	const char* function;
} XCEP_t_Exception;

typedef struct XCEP_t_Frame {
	jmp_buf env;
	int rethrow_request;
	int handled;
	XCEP_t_Exception exception;
	struct XCEP_t_Frame* prev;
} XCEP_t_Frame;

typedef void (*XCEP_t_ExceptionHandler)(XCEP_t_Exception);

// =========================================================
// MARK: Stack
// =========================================================

extern XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack;

// =========================================================
// MARK: Unhandled Exception Handler
// =========================================================

extern XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler;
#define XCEP_SetUncaughtExceptionHandler(handler) XCEP_g_UncaughtExceptionHandler = (handler)

#if XCEP_CONF_ENABLE_THREAD_SAFE
extern XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler;
#define XCEP_SetThreadUncaughtExceptionHandler(handler) XCEP_g_ThreadUncaughtExceptionHandler = (handler)
#define XCEPT___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(_exception) \
	else if (XCEP_g_ThreadUncaughtExceptionHandler) { \
		XCEP_g_ThreadUncaughtExceptionHandler(_exception);\
	}
#else
#define XCEPT___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(_exception)
#endif

// =========================================================
// MARK: Exception Print
// =========================================================

#define XCEP___M_XCEP_PRINT_EXCEPTION_FULL(_text, _code, _message, _function, _file, _line) fprintf(stderr, _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n", _code, _message, _function, _file, _line )
#define XCEP___M_XCEP_PRINT_EXCEPTION(_text, _exception) XCEP___M_XCEP_PRINT_EXCEPTION_FULL(_text, _exception.code, _exception.message, _exception.function, _exception.file, _exception.line )
#define XCEP_PrintException(_exception) XCEP___M_XCEP_PRINT_EXCEPTION("Exception", _exception)

// =========================================================
// MARK: Syntax
// =========================================================

#define XCEP_Try do { \
	XCEP_t_Frame XCEP_v_CurrentFrame = {0}; \
	XCEP_v_CurrentFrame.prev = XCEP_g_Stack; \
	XCEP_g_Stack = &XCEP_v_CurrentFrame; \
	int XCEP_v_hasThrown = setjmp(XCEP_v_CurrentFrame.env); \
	if (XCEP_v_hasThrown == 0)

#define XCEP_Catch(_code) \
	else if (!XCEP_v_CurrentFrame.handled && XCEP_v_CurrentFrame.exception.code == (_code) && (XCEP_v_CurrentFrame.handled = 1)) \

#define XCEP_CatchAll \
    else if (!XCEP_v_CurrentFrame.handled && (XCEP_v_CurrentFrame.handled = 1)) \

#define XCEP_Exception XCEP_g_Stack->exception

#define XCEP_Finally if (1)

#define XCEP_EndTry \
    XCEP_g_Stack = XCEP_g_Stack->prev; \
    if ((XCEP_v_hasThrown && !XCEP_v_CurrentFrame.handled) || XCEP_v_CurrentFrame.rethrow_request) { \
        if (XCEP_g_Stack) { \
            XCEP_g_Stack->exception = XCEP_v_CurrentFrame.exception; \
            longjmp(XCEP_g_Stack->env, 1); \
        } else { \
			if(0){} \
			XCEPT___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP_v_CurrentFrame.exception) \
			else if (XCEP_g_UncaughtExceptionHandler) { \
				XCEP_g_UncaughtExceptionHandler(XCEP_v_CurrentFrame.exception);\
			} \
			else { \
				XCEP___M_XCEP_PRINT_EXCEPTION("Uncaught exception", XCEP_v_CurrentFrame.exception); \
				exit(XCEP_v_CurrentFrame.exception.code); \
			} \
		} \
    } \
} while (0)

#define XCEP_Throw(_code, msg) do { \
    if (XCEP_g_Stack) { \
        XCEP_g_Stack->exception.code = (_code); \
        XCEP_g_Stack->exception.message = (msg); \
		XCEP_g_Stack->exception.line = __LINE__; \
		XCEP_g_Stack->exception.file = __FILE__; \
		XCEP_g_Stack->exception.function = __func__; \
		longjmp(XCEP_g_Stack->env, 1); \
    } else { \
			if(0){} \
			XCEPT___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP_g_Stack->exception) \
			else if (XCEP_g_UncaughtExceptionHandler) { \
				XCEP_g_UncaughtExceptionHandler(XCEP_g_Stack->exception);\
			} \
			else { \
	            XCEP___M_XCEP_PRINT_EXCEPTION_FULL("Uncaught exception", _code, msg, __func__, __FILE__, __LINE__ ); \
	            exit(_code); \
			} \
    } \
} while (0)

#define XCEP_Rethrow do { \
	XCEP_v_CurrentFrame.rethrow_request = 1; \
} while (0)

// =========================================================
// MARK: Short Commands
// =========================================================

#if XCEP_CONF_ENABLE_SHORT_COMMANDS
	#define Try XCEP_Try
	#define Catch(_code) XCEP_Catch(_code)
	#define CatchAll XCEP_CatchAll
	#define Exception XCEP_Exception
	#define Finally XCEP_Finally
	#define EndTry XCEP_EndTry
	#define Throw(_code, msg) XCEP_Throw(_code, msg)
	#define Rethrow XCEP_Rethrow
	#define PrintException(_exception) XCEP_PrintException(_exception)
	#define SetUncaughtExceptionHandler(_handler) XCEP_SetUncaughtExceptionHandler(_handler)

#if XCEP_CONF_ENABLE_THREAD_SAFE
	#define SetThreadUncaughtExceptionHandler(_handler) XCEP_SetThreadUncaughtExceptionHandler(_handler)
#endif

#endif

#endif //EXCEPTIONS_H