#ifndef XCEPPROJECT_CROSS_THREAD_H
#define XCEPPROJECT_CROSS_THREAD_H

#if defined(_WIN32)

#include <Windows.h>
typedef HANDLE XCEPTEST_t_Thread;
typedef DWORD (WINAPI *XCEPTEST_t_ThreadProc)(LPVOID);
#else // POSIX Thread
#include <pthread.h>
typedef pthread_t XCEPTEST_t_Thread;
typedef void* (*XCEPTEST_t_ThreadProc)(void*);
#endif

int XCEPTEST_ThreadCreate(XCEPTEST_t_Thread *thread, XCEPTEST_t_ThreadProc func, void *arg);
void XCEPTEST_ThreadJoin(XCEPTEST_t_Thread thread);
void XCEPTEST_sleep(int milliseconds);

#endif //XCEPPROJECT_CROSS_THREAD_H
