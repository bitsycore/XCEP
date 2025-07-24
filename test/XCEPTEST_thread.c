#include "XCEPTEST_thread.h"

#if defined(_WIN32)

#include <Windows.h>

typedef HANDLE XCEPTEST_t_Thread;
typedef DWORD (WINAPI *XCEPTEST_t_ThreadProc)(LPVOID);

int XCEPTEST_ThreadCreate(XCEPTEST_t_Thread *thread,  XCEPTEST_t_ThreadProc func, void *arg) {
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    if (*thread == NULL) return -1;
    return 0;
}

void XCEPTEST_ThreadJoin(const XCEPTEST_t_Thread thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
}

void XCEPTEST_sleep(const int milliseconds) {
    Sleep(milliseconds);
}

#else // POSIX Thread

#include <unistd.h>

int XCEPTEST_ThreadCreate(XCEPTEST_t_Thread* thread, XCEPTEST_t_ThreadProc func, void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

void XCEPTEST_ThreadJoin(XCEPTEST_t_Thread thread) {
    pthread_join(thread, NULL);
}

void XCEPTEST_sleep(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif