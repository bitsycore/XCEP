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
#include <assert.h>

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

#define XCEP_CONF_CUSTOM_TYPE_INT uint16_t
#define XCEP_CONF_CUSTOM_TYPE_UINT uint8_t
#define XCEP_CONF_CUSTOM_TYPE_BOOL bool
#define XCEP_CONF_CUSTOM_TRUE true
#define XCEP_CONF_CUSTOM_FALSE false

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
	#define XCEP_TRUE XCEP_CONF_CUSTOM_TRUE
	#define XCEP_FALSE XCEP_CONF_CUSTOM_FALSE
#else
	typedef unsigned int XCEP_BOOL;
	typedef unsigned int XCEP_UINT;
	typedef int XCEP_INT;
	#define XCEP_TRUE ((XCEP_BOOL)1u)
	#define XCEP_FALSE ((XCEP_BOOL)0u)
#endif

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
	struct {
		XCEP_BOOL run_once : 1;
		XCEP_BOOL thrown : 1;
		XCEP_BOOL rethrow_requested : 1;
		XCEP_BOOL thrown_in_catch: 1;
		XCEP_BOOL have_been_handled: 1;
	} state_flags;
	struct XCEP_t_Frame* prev;
} XCEP_t_Frame;

typedef void (*XCEP_t_ExceptionHandler)(const XCEP_t_Exception*);

// =========================================================
// MARK: Stack
// =========================================================

extern XCEP_THREAD_LOCAL XCEP_t_Frame* XCEP_g_Stack;
extern XCEP_THREAD_LOCAL XCEP_t_Exception XCEP_g_LastException;

// =========================================================
// MARK: UncaughtExceptionHandler
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
	XCEP_INT inCode,
	const char *inMessage
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	,const char *inFunctionName
	,const char *inFile
	,XCEP_INT inLine
#endif
);

void XCEP___PrintException(const char* inFormat, const XCEP_t_Exception* inException);
void XCEP___Thrown(const XCEP_t_Exception *inException);
void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame);
void XCEP___EndTry(const XCEP_t_Frame* inCurrentFrame);

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
	} XCEP_v_state

#define XCEP_Try \
	for ( \
		XCEP__DECLARE_STATE_STRUCT = { 0 }; /*Init*/ \
		XCEP_v_state.frame.state_flags.run_once == XCEP_FALSE; /*Cond*/ \
		XCEP_v_state.frame.state_flags.run_once = XCEP_TRUE, XCEP___EndTry((XCEP_t_Frame*)&XCEP_v_state.frame) /*Cleanup*/ \
	) \
	do { \
		if ( (XCEP_v_state.frame.prev = XCEP_g_Stack, \
			  XCEP_g_Stack = (XCEP_t_Frame*)&XCEP_v_state.frame, \
			  XCEP_v_state.frame.state_flags.thrown = setjmp(XCEP_v_state.frame.env) ) == XCEP_FALSE )

#define XCEP_Catch(_code) \
	else if (XCEP_v_state.frame.state_flags.have_been_handled == XCEP_FALSE && XCEP_g_LastException.code == (_code) && (XCEP_v_state.frame.state_flags.have_been_handled = XCEP_TRUE)) \

#define XCEP_CatchAll \
	else if (XCEP_v_state.frame.state_flags.have_been_handled == XCEP_FALSE && (XCEP_v_state.frame.state_flags.have_been_handled = XCEP_TRUE)) \

#define XCEP_CaughtException XCEP_g_LastException

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
XCEP_THREAD_LOCAL XCEP_t_Exception XCEP_g_LastException = {0};
XCEP_t_ExceptionHandler XCEP_g_UncaughtExceptionHandler = NULL;

#if XCEP_CONF_ENABLE_THREAD_SAFE
	XCEP_THREAD_LOCAL XCEP_t_ExceptionHandler XCEP_g_ThreadUncaughtExceptionHandler = NULL;
#endif

static inline void XCEP___DefaultUncaughtExceptionHandler(const XCEP_t_Exception* inException) {
	XCEP___PrintException(XCEP_FormatException("Uncaught inException"), inException);
	exit(inException->code);
}

static inline XCEP_BOOL XCEP___RunIfPossible(const XCEP_t_ExceptionHandler inFunction, const XCEP_t_Exception* inException) {
	if (inFunction) {
		inFunction(inException);
		return 1;
	}
	return 0;
}

void XCEP___PrintException(const char *inFormat, const XCEP_t_Exception *inException) {
	fprintf(stderr, inFormat,
			inException->code,
			inException->message
		#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
			,inException->function
			,inException->file
			,inException->line
		#endif
	);
}

void XCEP___Thrown(const XCEP_t_Exception *inException) {
	XCEP_t_Frame* vCurrentFrame = XCEP_g_Stack;

	// Propagate inException when thrown in catch
	if (vCurrentFrame != NULL && vCurrentFrame->state_flags.have_been_handled) {
		vCurrentFrame->state_flags.thrown_in_catch = 1;
	}

	if (vCurrentFrame) {
		XCEP___UpdateException(
			inException->code,
			inException->message
		#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
			,inException->function
			,inException->file
			,inException->line
		#endif
		);
		longjmp(vCurrentFrame->env, XCEP_TRUE);
	}

	if (!XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, inException)) &&
		!XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, inException)) {
		XCEP___DefaultUncaughtExceptionHandler(inException);
	}
}

void XCEP___UpdateException(
	const XCEP_INT inCode,
	const char *inMessage
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	,const char *inFunctionName
	,const char *inFile
	,const XCEP_INT inLine
#endif
) {
	XCEP_g_LastException.code = inCode;
	XCEP_g_LastException.message = inMessage;
#if XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO
	XCEP_g_LastException.line = inLine;
	XCEP_g_LastException.file = inFile;
	XCEP_g_LastException.function = inFunctionName;
#endif
}

void XCEP___EndTry(const XCEP_t_Frame *inCurrentFrame) {
	XCEP_g_Stack = XCEP_g_Stack->prev;

	const XCEP_BOOL vShouldPropagate =
			inCurrentFrame->state_flags.thrown == XCEP_TRUE && inCurrentFrame->state_flags.have_been_handled == XCEP_FALSE
			|| (inCurrentFrame->state_flags.rethrow_requested || inCurrentFrame->state_flags.thrown_in_catch);

	if (vShouldPropagate) {
		if (XCEP_g_Stack) {
			longjmp(XCEP_g_Stack->env, XCEP_TRUE);
		}

		if (!XCEP___IF_THREAD(XCEP___RunIfPossible(XCEP_g_ThreadUncaughtExceptionHandler, &XCEP_g_LastException)) &&
			!XCEP___RunIfPossible(XCEP_g_UncaughtExceptionHandler, &XCEP_g_LastException)) {
			XCEP___DefaultUncaughtExceptionHandler(&XCEP_g_LastException);
		}
	}
}

void XCEP___Rethrow(XCEP_t_Frame* inCurrentFrame) {
	assert(inCurrentFrame->state_flags.have_been_handled == XCEP_TRUE && "Rethrow can only be used inside a Catch or CatchAll block.");
	inCurrentFrame->state_flags.rethrow_requested = XCEP_TRUE;
}

#endif