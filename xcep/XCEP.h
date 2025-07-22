/*
Copyright (c) 2025 bitsycore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile

#ifndef XCEP_CDAD39BB4CBB62BD_H
#define XCEP_CDAD39BB4CBB62BD_H

#include <setjmp.h>

// =========================================================
// MARK: Configuration
// =========================================================

#define XCEP_CONF_ENABLE_THREAD_SAFE 1
#define XCEP_CONF_ENABLE_SHORT_COMMANDS 1
#define XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO 1
#define XCEP_CONF_ENABLE_CUSTOM_TYPES 0

#if XCEP_CONF_ENABLE_CUSTOM_TYPES

#include <stdbool.h>
#include <stdint.h>
#define XCEP_CONF_CUSTOM_TYPE_INT int32_t
#define XCEP_CONF_CUSTOM_TYPE_UINT uint32_t
#define XCEP_CONF_CUSTOM_TYPE_BOOL bool

#endif

// =========================================================
// MARK: Attributes
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

#if XCEP_CONF_ENABLE_CUSTOM_TYPES
	typedef XCEP_CONF_CUSTOM_TYPE_BOOL XCEP_BOOL;
	typedef XCEP_CONF_CUSTOM_TYPE_UINT XCEP_UINT;
	typedef XCEP_CONF_CUSTOM_TYPE_INT XCEP_INT;
#else
	typedef int XCEP_BOOL;
	typedef unsigned int XCEP_UINT;
	typedef int XCEP_INT;
#endif

enum XCEP_t_FrameState {
    XCEP_FRAME_STATE_RETHROW_REQUESTED = 1 << 0,
    XCEP_FRAME_STATE_THROWN_IN_CATCH = 1 << 1,
	XCEP_FRAME_STATE_HANDLED = 1 << 2,
};

typedef struct {
	XCEP_INT code;
	const char* message;
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	XCEP_INT line;
	const char* file;
	const char* function;
#endif
} XCEP_t_Exception;

typedef struct XCEP_t_Frame {
	jmp_buf env;
    XCEP_UINT state_flags;
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
	#define XCEP___IF_THREAD(_instruction) (0)
#endif

// =========================================================
// MARK: Functions Def
// =========================================================

void XCEP___UpdateException(
	XCEP_t_Frame *inFrame,
	XCEP_INT inCode,
	const char *inMessage
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
    ,
    const char *inFunctionName,
    const char *inFile,
    XCEP_INT inLine
#endif
);

void XCEP___PrintException(const char* inFormat, const XCEP_t_Exception* inException);
void XCEP___Thrown(const XCEP_t_Exception *inException);
void XCEP___EndTry(XCEP_BOOL inHasThrown, const XCEP_t_Frame* inCurrentFrame);
void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame);

// =========================================================
// MARK: Exception Print
// =========================================================

#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
#define XCEP_FormatException(_text) _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n"
#else
#define XCEP_FormatException(_text) _text " (%d) caused by: \"%s\"\n"
#endif

#define XCEP_PrintException(_text, _exception) XCEP___PrintException(XCEP_FormatException(_text), _exception)

// =========================================================
// MARK: Syntax
// =========================================================

#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
#define XCEP_NewException(_code, _msg) (XCEP_t_Exception){ .code = _code, .message = _msg, .line = __LINE__, .file = __FILE__, .function = __func__ }
#else
#define XCEP_NewException(_code, _msg) (XCEP_t_Exception){ .code = _code, .message = _msg }
#endif

#define XCEP__DECLARE_STATE_STRUCT \
    struct { \
        XCEP_t_Frame frame; \
        XCEP_BOOL thrown : 1; \
        XCEP_BOOL run_once : 1; \
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
              XCEP_v_state.thrown = setjmp(XCEP_v_state.frame.env) ) == 0 )

#define XCEP_Catch(_code) \
	else if (!(XCEP_v_state.frame.state_flags & XCEP_FRAME_STATE_HANDLED) && XCEP_v_state.frame.exception.code == (_code) && (XCEP_v_state.frame.state_flags |= XCEP_FRAME_STATE_HANDLED)) \

#define XCEP_CatchAll \
    else if (!(XCEP_v_state.frame.state_flags & XCEP_FRAME_STATE_HANDLED) && (XCEP_v_state.frame.state_flags |= XCEP_FRAME_STATE_HANDLED)) \

#define XCEP_CaughtException XCEP_g_Stack->exception

#define XCEP_Finally if (1)

#define XCEP_EndTry \
    } while (0)

#define XCEP_Throw(_code, _msg) XCEP___Thrown(&XCEP_NewException(_code, _msg))

#define XCEP_Rethrow XCEP___Rethrow((XCEP_t_Frame*)&XCEP_v_state.frame)

// =========================================================
// MARK: Short Commands
// =========================================================

#if XCEP_CONF_ENABLE_SHORT_COMMANDS
	typedef XCEP_t_Exception t_Exception;
	typedef XCEP_t_ExceptionHandler t_ExceptionHandler;
	#define NewException(_code, _msg) XCEP_NewException(_code, _msg)
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
#undef XCEP_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack = NULL;
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;

#if XCEP_CONF_ENABLE_THREAD_SAFE
	XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif

static XCEP_BOOL XCEP___DefaultUncaughtExceptionHandler(const XCEP_t_Exception* inException) {
	XCEP___PrintException(XCEP_FormatException("Uncaught inException"), inException);
	exit(inException->code);
}

static XCEP_BOOL XCEP___RunIfPossible(const XCEP_t_ExceptionHandler inFunction, const XCEP_t_Exception* inException) {
	if (inFunction) {
		inFunction(inException);
		return 1;
	}
	return 0;
}

void XCEP___UpdateException(
	XCEP_t_Frame *inFrame,
    const XCEP_INT inCode,
    const char *inMessage
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
    ,
    const char *inFunctionName,
    const char *inFile,
    const XCEP_INT inLine
#endif
) {
	inFrame->exception.code = inCode;
	inFrame->exception.message = inMessage;
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	inFrame->exception.line = inLine;
	inFrame->exception.file = inFile;
	inFrame->exception.function = inFunctionName;
#endif
}

void XCEP___PrintException(const char *inFormat, const XCEP_t_Exception *inException) {
	fprintf(stderr, inFormat,
	        inException->code,
	        inException->message
		#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	        ,
	        inException->function,
	        inException->file,
	        inException->line
		#endif
	);
}

void XCEP___Thrown(const XCEP_t_Exception *inException) {
	XCEP_t_Frame* vCurrentFrame = XCEP_g_Stack;

	// Propagate inException when thrown in catch
	if (vCurrentFrame != NULL && vCurrentFrame->state_flags & XCEP_FRAME_STATE_HANDLED) {
		vCurrentFrame->state_flags |= XCEP_FRAME_STATE_THROWN_IN_CATCH;
	}

	if (vCurrentFrame) {
		XCEP___UpdateException(vCurrentFrame,
			inException->code,
			inException->message
		#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
			,
			inException->function,
			inException->file,
			inException->line
		#endif
			);
		longjmp(vCurrentFrame->env, 1);
	}

	if (!XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, inException)) &&
	    !XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, inException)) {
		XCEP___DefaultUncaughtExceptionHandler(inException);
	}
}

void XCEP___EndTry(const XCEP_BOOL inHasThrown, const XCEP_t_Frame *inCurrentFrame) {
	XCEP_g_Stack = XCEP_g_Stack->prev;

	const XCEP_BOOL vShouldPropagate = inHasThrown && !(inCurrentFrame->state_flags & XCEP_FRAME_STATE_HANDLED) ||
	                       inCurrentFrame->state_flags & (XCEP_FRAME_STATE_RETHROW_REQUESTED | XCEP_FRAME_STATE_THROWN_IN_CATCH);

	if (vShouldPropagate) {
		if (XCEP_g_Stack) {
			XCEP_g_Stack->exception = inCurrentFrame->exception;
			longjmp(XCEP_g_Stack->env, 1);
		}

		if (!XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, &inCurrentFrame->exception)) &&
		    !XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, &inCurrentFrame->exception)) {
			XCEP___DefaultUncaughtExceptionHandler(&inCurrentFrame->exception);
		}
	}
}

void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame) {
    assert(inCurrentFrame->state_flags & XCEP_FRAME_STATE_HANDLED && "Rethrow can only be used inside a Catch or CatchAll block.");
    inCurrentFrame->state_flags |= XCEP_FRAME_STATE_RETHROW_REQUESTED;
}

#endif