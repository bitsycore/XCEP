#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <setjmp.h>
#include <stddef.h>

#if defined(_MSC_VER)
  #define __EXCEPTIONS_M_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
  #if __STDC_VERSION__ >= 201112L
	#define __EXCEPTIONS_M_THREAD_LOCAL _Thread_local
  #else
	#define __EXCEPTIONS_M_THREAD_LOCAL __thread
  #endif
#else
  #define __EXCEPTIONS_M_THREAD_LOCAL thread_local
#endif

typedef struct {
	int code;
	const char* message;
	int line;
	const char* file;
	const char* function;
} __EXCEPTIONS_t_Exception;

typedef struct __EXCEPTIONS_t_Frame {
	jmp_buf env;
	int rethrow_request;
	int handled;
	__EXCEPTIONS_t_Exception exception;
	struct __EXCEPTIONS_t_Frame* prev;
} __EXCEPTIONS_t_Frame;

typedef void (*__EXCEPTIONS_t_ExceptionHandler)(__EXCEPTIONS_t_Exception);

extern __EXCEPTIONS_M_THREAD_LOCAL __EXCEPTIONS_t_Frame* __EXCEPTIONS_g_STACK;
extern __EXCEPTIONS_M_THREAD_LOCAL __EXCEPTIONS_t_ExceptionHandler __EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER;
extern __EXCEPTIONS_t_ExceptionHandler __EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER;

#define SET_UNHANDLED_EXCEPTION_HANDLER(handler) __EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER = (handler)
#define SET_LOCAL_UNHANDLED_EXCEPTION_HANDLER(handler) __EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER = (handler)

#define __EXCEPTIONS_M_PRINT_EXCEPTION_FULL(_text, _code, _message, _function, _file, _line) fprintf(stderr, _text " (%d) caused by: \"%s\"\n\tat %s(%s:%d)\n", _code, _message, _function, _file, _line )
#define __EXCEPTIONS_M_PRINT_EXCEPTION(_text, _exception) __EXCEPTIONS_M_PRINT_EXCEPTION_FULL(_text, _exception.code, _exception.message, _exception.function, _exception.file, _exception.line )
#define PRINT_EXCEPTION(_exception) __EXCEPTIONS_M_PRINT_EXCEPTION("Exception", _exception)

#define TRY do { \
	__EXCEPTIONS_t_Frame __EXCEPTIONS_v_CurrentFrame = {0}; \
	__EXCEPTIONS_v_CurrentFrame.prev = __EXCEPTIONS_g_STACK; \
	__EXCEPTIONS_g_STACK = &__EXCEPTIONS_v_CurrentFrame; \
	int __v_EXCEPTION__THROWN = setjmp(__EXCEPTIONS_v_CurrentFrame.env); \
	if (__v_EXCEPTION__THROWN == 0)

#define CATCH(_code) \
else if (!__EXCEPTIONS_v_CurrentFrame.handled && __EXCEPTIONS_v_CurrentFrame.exception.code == (_code) && (__EXCEPTIONS_v_CurrentFrame.handled = 1)) \
        for (int __EXCEPTIONS_v_RunOnce = 1; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0) \
            for (const __EXCEPTIONS_t_Exception EXCEPTION = __EXCEPTIONS_g_STACK->exception; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0)

#define CATCH_ALL \
    else if (!__EXCEPTIONS_v_CurrentFrame.handled && (__EXCEPTIONS_v_CurrentFrame.handled = 1)) \
        for (int __EXCEPTIONS_v_RunOnce = 1; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0) \
            for (const __EXCEPTIONS_t_Exception EXCEPTION = __EXCEPTIONS_g_STACK->exception; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0)

#define FINALLY if (1)

#define END_TRY \
    __EXCEPTIONS_g_STACK = __EXCEPTIONS_g_STACK->prev; \
    if ((__v_EXCEPTION__THROWN && !__EXCEPTIONS_v_CurrentFrame.handled) || __EXCEPTIONS_v_CurrentFrame.rethrow_request) { \
        if (__EXCEPTIONS_g_STACK) { \
            __EXCEPTIONS_g_STACK->exception = __EXCEPTIONS_v_CurrentFrame.exception; \
            longjmp(__EXCEPTIONS_g_STACK->env, 1); \
        } else { \
			if (__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER(__EXCEPTIONS_v_CurrentFrame.exception);\
			} \
			else if (__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER(__EXCEPTIONS_v_CurrentFrame.exception);\
			} \
			else { \
				__EXCEPTIONS_M_PRINT_EXCEPTION("Uncaught exception", __EXCEPTIONS_v_CurrentFrame.exception); \
				exit(__EXCEPTIONS_v_CurrentFrame.exception.code); \
			} \
		} \
    } \
} while (0)

#define THROW(_code, msg) do { \
    if (__EXCEPTIONS_g_STACK) { \
        __EXCEPTIONS_g_STACK->exception.code = (_code); \
        __EXCEPTIONS_g_STACK->exception.message = (msg); \
		__EXCEPTIONS_g_STACK->exception.line = __LINE__; \
		__EXCEPTIONS_g_STACK->exception.file = __FILE__; \
		__EXCEPTIONS_g_STACK->exception.function = __func__; \
        longjmp(__EXCEPTIONS_g_STACK->env, 1); \
    } else { \
			if (__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER(__EXCEPTIONS_g_STACK->exception);\
			} \
			else if (__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER(__EXCEPTIONS_g_STACK->exception);\
			} \
			else { \
	            __EXCEPTIONS_M_PRINT_EXCEPTION_FULL("Uncaught exception", _code, msg, __func__, __FILE__, __LINE__ ); \
	            exit(_code); \
			} \
    } \
} while (0)

#define RETHROW do { \
	__EXCEPTIONS_v_CurrentFrame.rethrow_request = 1; \
} while (0)

#endif //EXCEPTIONS_H