#ifndef XCEPPROJECT_CROSS_THREAD_H
#define XCEPPROJECT_CROSS_THREAD_H

#if defined(_WIN32)

#include <Windows.h>
typedef HANDLE my_thread_t;
typedef DWORD (WINAPI *my_thread_proc_t)(LPVOID);
#else // POSIX Thread
#include <pthread.h>
#include <unistd.h>
typedef pthread_t my_thread_t;
typedef void* (*my_thread_proc_t)(void*);
#endif

int my_thread_create(my_thread_t *thread, my_thread_proc_t func, void *arg);
void my_thread_join(my_thread_t thread);
void my_sleep(int milliseconds);

#endif //XCEPPROJECT_CROSS_THREAD_H
