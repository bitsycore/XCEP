#ifndef XCEP_CDAD39BB4CBB62BD_H
#define XCEP_CDAD39BB4CBB62BD_H

#include <setjmp.h>

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
#elif defined(__clang__) || defined(__GNUC__)
	#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && __STDC_VERSION__ < 202311L
		#define XCEP_THREAD_LOCAL _Thread_local
	#else
		#define XCEP_THREAD_LOCAL __thread
	#endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
	#define XCEP_THREAD_LOCAL thread_local
#else
	#error "Cannot determine thread-local storage specifier"
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

typedef void (*XCEP_t_ExceptionHandler)(const XCEP_t_Exception*);

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
	#define XCEP___IF_THREAD(_bool) (_ptr)
#else
	#define XCEP___IF_THREAD(_bool) (NULL)
#endif


// =========================================================
// MARK: Functions Def
// =========================================================

void XCEP___UpdateException(XCEP_t_Frame* frame, int code, const char* message, const char* function, const char* file, int line);
void XCEP___PrintException(const char* format, const XCEP_t_Exception* exception);
void XCEP___Thrown(const XCEP_t_Exception *exception);
void XCEP___EndTry(int XCEP_v_hasThrown, const XCEP_t_Frame* XCEP_v_CurrentFrame);
void XCEP___Rethrow(XCEP_t_Frame* XCEP_v_CurrentFrame);

// =========================================================
// MARK: Exception Print
// =========================================================

#define XCEP_FormatException(_text) _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n"
#define XCEP_PrintException(_text, _exception) XCEP___PrintException(XCEP_FormatException(_text), _exception)

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
	XCEP___EndTry(XCEP_v_hasThrown, &XCEP_v_CurrentFrame);\
} while (0)

#define XCEP_Throw(_code, msg) XCEP___Thrown(&(XCEP_t_Exception){ _code, msg, __LINE__, __FILE__, __func__ })

#define XCEP_Rethrow XCEP___Rethrow(&XCEP_v_CurrentFrame)

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

// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile

//#define XCEP_IMPLEMENTATION
#ifdef XCEP_IMPLEMENTATION

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack = NULL;
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;

#if XCEP_CONF_ENABLE_THREAD_SAFE
	XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif

static int XCEP___DefaultUncaughtExceptionHandler(const XCEP_t_Exception* exception) {
	XCEP___PrintException(XCEP_FormatException("Uncaught exception"), exception);
	exit(exception->code);
}

static int XCEP___RunIfPossible(const XCEP_t_ExceptionHandler func, const XCEP_t_Exception* exception) {
	if (func) {
		func(exception);
		return 1;
	}
	return 0;
}

void XCEP___UpdateException(XCEP_t_Frame* frame, const int code, const char* message, const char* function, const char* file, const int line) {
	frame->exception.code = code;
	frame->exception.message = message;
	frame->exception.line = line;
	frame->exception.file = file;
	frame->exception.function = function;
}

void XCEP___PrintException(const char* format, const XCEP_t_Exception* exception) {
	fprintf(stderr, format, exception->code, exception->message, exception->function, exception->file, exception->line );
}

void XCEP___Thrown(const XCEP_t_Exception *exception) {
	XCEP_t_Frame* current_frame = XCEP_g_Stack;
	if (current_frame) {
		XCEP___UpdateException(current_frame, exception->code, exception->message, exception->function, exception->file, exception->line);
		longjmp(current_frame->env, 1);
	}
	if (
		// ReSharper disable once CppDFAConstantConditions
		XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, exception))
		||
		XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, exception)
		||
		XCEP___DefaultUncaughtExceptionHandler(exception)
	) {}
}

void XCEP___EndTry(const int XCEP_v_hasThrown, const XCEP_t_Frame* XCEP_v_CurrentFrame) {
	XCEP_g_Stack = XCEP_g_Stack->prev;
	if ((XCEP_v_hasThrown && !XCEP_v_CurrentFrame->handled) || XCEP_v_CurrentFrame->rethrow_request) {
		if (XCEP_g_Stack) {
			XCEP_g_Stack->exception = XCEP_v_CurrentFrame->exception;
			longjmp(XCEP_g_Stack->env, 1);
		}
		if (
			// ReSharper disable once CppDFAConstantConditions
			XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, &XCEP_v_CurrentFrame->exception))
			||
			XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, &XCEP_v_CurrentFrame->exception)
			||
			XCEP___DefaultUncaughtExceptionHandler(&XCEP_v_CurrentFrame->exception)
		) {}
	}
}

void XCEP___Rethrow(XCEP_t_Frame* XCEP_v_CurrentFrame) {
	XCEP_v_CurrentFrame->rethrow_request = 1;
}

#undef XCEP_IMPLEMENTATION
#endif