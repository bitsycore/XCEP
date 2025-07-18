#ifndef XCEP_CDAD39BB4CBB62BD_H
#define XCEP_CDAD39BB4CBB62BD_H

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
#define XCEP___RUN_IF_SET_UNCAUGHT_EXCEPTIONS_HANDLER(_exception) \
		else if (XCEP_g_UncaughtExceptionHandler) { \
			XCEP_g_UncaughtExceptionHandler(_exception);\
		}

#if XCEP_CONF_ENABLE_THREAD_SAFE
	#define XCEP_SetThreadUncaughtExceptionHandler(handler) XCEP_g_ThreadUncaughtExceptionHandler = (handler)
	extern XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler;
	#define XCEP___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(_exception) \
		else if (XCEP_g_ThreadUncaughtExceptionHandler) { \
			XCEP_g_ThreadUncaughtExceptionHandler(_exception);\
		}
#else
	#define XCEP___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(_exception)
#endif

// =========================================================
// MARK: Functions Def
// =========================================================

void XCEP___UpdateException(XCEP_t_Frame* frame, int code, const char* message, const char* function, const char* file, int line);
void XCEP___PrintException(const char* format, XCEP_t_Exception exception);

// =========================================================
// MARK: Exception Print
// =========================================================

#define XCEP___FormatException(_text) _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n"
#define XCEP___UncaughtPrintException(_exception) XCEP___PrintException(XCEP___FormatException("Uncaught exception"), _exception)
#define XCEP_PrintException(_exception) XCEP___PrintException(XCEP___FormatException("Exception"), _exception)

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
			XCEP___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP_v_CurrentFrame.exception) \
			XCEP___RUN_IF_SET_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP_v_CurrentFrame.exception) \
			else { \
				XCEP___UncaughtPrintException(XCEP_v_CurrentFrame.exception); \
				exit(XCEP_v_CurrentFrame.exception.code); \
			} \
		} \
    } \
} while (0)

#define XCEP_Throw(_code, msg) do { \
    if (XCEP_g_Stack) { \
		XCEP___UpdateException(XCEP_g_Stack, _code, msg, __func__, __FILE__, __LINE__); \
		longjmp(XCEP_g_Stack->env, 1); \
    } else { \
			XCEP_t_Exception XCEP___v_ExceptionToPrint = { _code, msg, __LINE__, __FILE__, __func__ }; \
			if(0){} \
			XCEP___RUN_IF_SET_THREAD_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP___v_ExceptionToPrint) \
			XCEP___RUN_IF_SET_UNCAUGHT_EXCEPTIONS_HANDLER(XCEP___v_ExceptionToPrint) \
			else { \
				XCEP___UncaughtPrintException(XCEP___v_ExceptionToPrint); \
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

#endif // XCEP_CDAD39BB4CBB62BD_H

#ifdef XCEP_IMPLEMENTATION

XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack = NULL;
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;

#if XCEP_CONF_ENABLE_THREAD_SAFE
	XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif

inline void XCEP___UpdateException(XCEP_t_Frame* frame, const int code, const char* message, const char* function, const char* file, const int line) {
	frame->exception.code = code;
	frame->exception.message = message;
	frame->exception.line = line;
	frame->exception.file = file;
	frame->exception.function = function;
}

inline void XCEP___PrintException(const char* format, const XCEP_t_Exception exception) {
	fprintf(stderr, format, exception.code, exception.message, exception.function, exception.file, exception.line );
}

#undef XCEP_IMPLEMENTATION
#endif