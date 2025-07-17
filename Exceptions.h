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

typedef struct __EXCEPTIONS_t_Frame {
	jmp_buf env;
	int rethrow_request;
	int handled;
	const char* message;
	int code;
	struct __EXCEPTIONS_t_Frame* prev;
} __EXCEPTIONS_t_Frame;

typedef struct {
	int code;
	const char* message;
} __EXCEPTIONS_t_Exception;

typedef void (*__EXCEPTIONS_t_ExceptionHandler)(__EXCEPTIONS_t_Exception);

extern __EXCEPTIONS_M_THREAD_LOCAL __EXCEPTIONS_t_Frame* __EXCEPTIONS_g_STACK;
extern __EXCEPTIONS_M_THREAD_LOCAL __EXCEPTIONS_t_ExceptionHandler __EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER;
extern __EXCEPTIONS_t_ExceptionHandler __EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER;

#define SET_UNHANDLED_EXCEPTION_HANDLER(handler) __EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER = (handler)
#define SET_LOCAL_UNHANDLED_EXCEPTION_HANDLER(handler) __EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER = (handler)

#define TRY do { \
	__EXCEPTIONS_t_Frame __EXCEPTIONS_v_CurrentFrame; \
	__EXCEPTIONS_v_CurrentFrame.prev = __EXCEPTIONS_g_STACK; \
	__EXCEPTIONS_v_CurrentFrame.handled = 0; \
	__EXCEPTIONS_v_CurrentFrame.rethrow_request = 0;\
	__EXCEPTIONS_v_CurrentFrame.message = NULL; \
	__EXCEPTIONS_v_CurrentFrame.code = 0; \
	__EXCEPTIONS_g_STACK = &__EXCEPTIONS_v_CurrentFrame; \
	int __v_EXCEPTION__THROWN = setjmp(__EXCEPTIONS_v_CurrentFrame.env); \
	if (__v_EXCEPTION__THROWN == 0)

#define CATCH(_code) \
else if (!__EXCEPTIONS_v_CurrentFrame.handled && __EXCEPTIONS_v_CurrentFrame.code == (_code) && (__EXCEPTIONS_v_CurrentFrame.handled = 1)) \
        for (int __EXCEPTIONS_v_RunOnce = 1; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0) \
            for (const __EXCEPTIONS_t_Exception EXCEPTION = {__EXCEPTIONS_g_STACK->code, __EXCEPTIONS_g_STACK->message}; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0)

#define CATCH_ALL \
    else if (!__EXCEPTIONS_v_CurrentFrame.handled && (__EXCEPTIONS_v_CurrentFrame.handled = 1)) \
        for (int __EXCEPTIONS_v_RunOnce = 1; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0) \
            for (const __EXCEPTIONS_t_Exception EXCEPTION = {__EXCEPTIONS_g_STACK->code, __EXCEPTIONS_g_STACK->message}; __EXCEPTIONS_v_RunOnce; __EXCEPTIONS_v_RunOnce = 0)

#define FINALLY if (1)

#define END_TRY \
    __EXCEPTIONS_g_STACK = __EXCEPTIONS_g_STACK->prev; \
    if ((__v_EXCEPTION__THROWN && !__EXCEPTIONS_v_CurrentFrame.handled) || __EXCEPTIONS_v_CurrentFrame.rethrow_request) { \
        if (__EXCEPTIONS_g_STACK) { \
            __EXCEPTIONS_g_STACK->code = __EXCEPTIONS_v_CurrentFrame.code; \
            __EXCEPTIONS_g_STACK->message = __EXCEPTIONS_v_CurrentFrame.message; \
            longjmp(__EXCEPTIONS_g_STACK->env, 1); \
        } else { \
			if (__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER((__EXCEPTIONS_t_Exception){__EXCEPTIONS_v_CurrentFrame.code, __EXCEPTIONS_v_CurrentFrame.message});\
			} \
			else if (__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER((__EXCEPTIONS_t_Exception){__EXCEPTIONS_v_CurrentFrame.code, __EXCEPTIONS_v_CurrentFrame.message});\
			} \
			else { \
	            fprintf(stderr, "Uncaught exception %d: %s\n", __EXCEPTIONS_v_CurrentFrame.code, __EXCEPTIONS_v_CurrentFrame.message); \
	            exit(__EXCEPTIONS_v_CurrentFrame.code); \
			} \
		} \
    } \
} while (0)

#define THROW(_code, msg) do { \
    if (__EXCEPTIONS_g_STACK) { \
        __EXCEPTIONS_g_STACK->code = (_code); \
        __EXCEPTIONS_g_STACK->message = (msg); \
        longjmp(__EXCEPTIONS_g_STACK->env, 1); \
    } else { \
			if (__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_THREAD_LOCAL_UNHANDLED_EXCEPTION_HANDLER((__EXCEPTIONS_t_Exception){_code, msg});\
			} \
			else if (__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER) { \
				__EXCEPTIONS_g_UNHANDLED_EXCEPTION_HANDLER((__EXCEPTIONS_t_Exception){_code, msg});\
			} \
			else { \
	            fprintf(stderr, "Uncaught exception %d: %s\n", _code, msg); \
	            exit(_code); \
			} \
    } \
} while (0)

#define RETHROW do { \
	__EXCEPTIONS_v_CurrentFrame.rethrow_request = 1; \
} while (0)

#endif //EXCEPTIONS_H