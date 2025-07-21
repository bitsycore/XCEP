#include "cross_thread.h"

#if defined(_WIN32)

#include <Windows.h>

typedef HANDLE my_thread_t;
typedef DWORD (WINAPI *my_thread_proc_t)(LPVOID);

int my_thread_create(my_thread_t *thread, my_thread_proc_t func, void *arg) {
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    if (*thread == NULL) return -1;
    return 0;
}

void my_thread_join(my_thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
}

void my_sleep(int milliseconds) {
    Sleep(milliseconds);
}

#else // POSIX Thread
int my_thread_create(my_thread_t* thread, my_thread_proc_t func, void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

void my_thread_join(my_thread_t thread) {
    pthread_join(thread, NULL);
}

void my_sleep(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif