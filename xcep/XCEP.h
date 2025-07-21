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

enum XCEP_t_FrameState {
    XCEP_FRAME_STATE_RETHROW_REQUESTED = 1 << 0,
    XCEP_FRAME_STATE_THROWN_IN_CATCH = 1 << 1,
};

typedef struct {
	int code;
	const char* message;
	int line;
	const char* file;
	const char* function;
} XCEP_t_Exception;

typedef struct XCEP_t_Frame {
	jmp_buf env;
    int state_flags;
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
// MARK: Unhandled CaughtException Handler
// =========================================================

extern XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler;
#define XCEP_SetUncaughtExceptionHandler(_handler) XCEP_g_UncaughtExceptionHandler = (_handler)

#if XCEP_CONF_ENABLE_THREAD_SAFE
	extern XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler;
	#define XCEP_SetThreadUncaughtExceptionHandler(_handler) XCEP_g_ThreadUncaughtExceptionHandler = (_handler)
	#define XCEP___IF_THREAD(_instruction) (_instruction)
#else
	#define XCEP___IF_THREAD(_instruction) (NULL)
#endif

// =========================================================
// MARK: Functions Def
// =========================================================

void XCEP___UpdateException(XCEP_t_Frame* inFrame, int inCode, const char* inMessage, const char* inFunctionName, const char* inFile, int inLine);
void XCEP___PrintException(const char* inFormat, const XCEP_t_Exception* inException);
void XCEP___Thrown(const XCEP_t_Exception *inException);
void XCEP___EndTry(int inHasThrown, const volatile XCEP_t_Frame* inCurrentFrame);
void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame);

// =========================================================
// MARK: CaughtException Print
// =========================================================

#define XCEP_FormatException(_text) _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n"
#define XCEP_PrintException(_text, _exception) XCEP___PrintException(XCEP_FormatException(_text), _exception)

// =========================================================
// MARK: Syntax
// =========================================================

#define XCEP__DECLARE_STATE_STRUCT \
    struct { \
        volatile XCEP_t_Frame frame; \
        volatile int thrown; \
        int run_once; \
    } XCEP_v_state

#define XCEP_Try \
    for ( \
        XCEP__DECLARE_STATE_STRUCT = { .run_once = 1 }; /*Init*/ \
        XCEP_v_state.run_once; /*Cond*/ \
        XCEP_v_state.run_once = 0, XCEP___EndTry(XCEP_v_state.thrown, (XCEP_t_Frame*)&XCEP_v_state.frame) /*Cleanup*/ \
    ) \
    do { \
        if ( (XCEP_v_state.frame.prev = XCEP_g_Stack, \
              XCEP_g_Stack = (XCEP_t_Frame*)&XCEP_v_state.frame, \
              XCEP_v_state.thrown = setjmp(XCEP_v_state.frame.env)) == 0 )

#define XCEP_Catch(_code) \
	else if (!XCEP_v_state.frame.handled && XCEP_v_state.frame.exception.code == (_code) && (XCEP_v_state.frame.handled = 1)) \

#define XCEP_CatchAll \
    else if (!XCEP_v_state.frame.handled && (XCEP_v_state.frame.handled = 1)) \

#define XCEP_CaughtException XCEP_g_Stack->exception

#define XCEP_Finally if (1)

#define XCEP_EndTry \
    } while (0)

#define XCEP_Throw(_code, _msg) XCEP___Thrown(&(XCEP_t_Exception){ .code = _code, .message = _msg, .line = __LINE__, .file = __FILE__, .function = __func__ })

#define XCEP_Rethrow XCEP___Rethrow((XCEP_t_Frame*)&XCEP_v_state.frame)

// =========================================================
// MARK: Short Commands
// =========================================================

#if XCEP_CONF_ENABLE_SHORT_COMMANDS

	#define Try XCEP_Try
	#define Catch(_code) XCEP_Catch(_code)
	#define CatchAll XCEP_CatchAll
	#define CaughtException XCEP_CaughtException
	#define Finally XCEP_Finally
	#define EndTry XCEP_EndTry
	#define Throw(_code, _msg) XCEP_Throw(_code, _msg)
	#define Rethrow XCEP_Rethrow
    #define PrintException(_text, _exception) XCEP_PrintException(_text, _exception)
	#define SetUncaughtExceptionHandler(_handler) XCEP_SetUncaughtExceptionHandler(_handler)

	#if XCEP_CONF_ENABLE_THREAD_SAFE
		#define SetThreadUncaughtExceptionHandler(_handler) XCEP_SetThreadUncaughtExceptionHandler(_handler)
	#endif

#endif

#endif // XCEP_CDAD39BB4CBB62BD_H

#ifdef XCEP_IMPLEMENTATION

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack = NULL;
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;

#if XCEP_CONF_ENABLE_THREAD_SAFE
	XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif

static int XCEP___DefaultUncaughtExceptionHandler(const XCEP_t_Exception* inException) {
	XCEP___PrintException(XCEP_FormatException("Uncaught inException"), inException);
	exit(inException->code);
}

static int XCEP___RunIfPossible(const XCEP_t_ExceptionHandler inFunction, const XCEP_t_Exception* inException) {
	if (inFunction) {
		inFunction(inException);
		return 1;
	}
	return 0;
}

void XCEP___UpdateException(XCEP_t_Frame* inFrame, const int inCode, const char* inMessage, const char* inFunctionName, const char* inFile, const int inLine) {
    inFrame->exception.code = inCode;
    inFrame->exception.message = inMessage;
    inFrame->exception.line = inLine;
    inFrame->exception.file = inFile;
    inFrame->exception.function = inFunctionName;
}

void XCEP___PrintException(const char* inFormat, const XCEP_t_Exception* inException) {
	fprintf(stderr, inFormat, inException->code, inException->message, inException->function, inException->file, inException->line );
}

void XCEP___Thrown(const XCEP_t_Exception *inException) {
    XCEP_t_Frame* vCurrentFrame = XCEP_g_Stack;

    // Propagate inException when thrown in catch
    if (vCurrentFrame != NULL && vCurrentFrame->handled) {
        vCurrentFrame->state_flags |= XCEP_FRAME_STATE_THROWN_IN_CATCH;
    }

	if (vCurrentFrame) {
		XCEP___UpdateException(vCurrentFrame, inException->code, inException->message, inException->function, inException->file, inException->line);
		longjmp(vCurrentFrame->env, 1);
	}

	XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, inException))
	||
	XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, inException)
	||
	XCEP___DefaultUncaughtExceptionHandler(inException);
}

void XCEP___EndTry(const int inHasThrown, const volatile XCEP_t_Frame* inCurrentFrame) {
	XCEP_g_Stack = XCEP_g_Stack->prev;

    int vShouldPropagate = (inHasThrown && !inCurrentFrame->handled) ||
                           (inCurrentFrame->state_flags & (XCEP_FRAME_STATE_RETHROW_REQUESTED | XCEP_FRAME_STATE_THROWN_IN_CATCH));

    if (vShouldPropagate) {
        if (XCEP_g_Stack) {
            XCEP_g_Stack->exception = inCurrentFrame->exception;
            longjmp(XCEP_g_Stack->env, 1);
        }

        XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, (XCEP_t_Exception*)&inCurrentFrame->exception))
        ||
        XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, (XCEP_t_Exception*)&inCurrentFrame->exception)
        ||
        XCEP___DefaultUncaughtExceptionHandler((XCEP_t_Exception*)&inCurrentFrame->exception);
    }
}

void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame) {
    assert(inCurrentFrame->handled && "Rethrow can only be used inside a Catch or CatchAll block.");
    inCurrentFrame->state_flags |= XCEP_FRAME_STATE_RETHROW_REQUESTED;
}

#undef XCEP_IMPLEMENTATION
#endif