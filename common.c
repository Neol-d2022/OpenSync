#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

static void _MemoryRequestFailed(size_t size, const char *sourceFile, unsigned int lineNumber)
{
    fprintf(stderr, "[CRITICAL] Memory Request Failed.\n");
#ifdef DEBUG
    fprintf(stderr, "[CRITICAL] in %s at %u, size = %u.\n", sourceFile, lineNumber, (unsigned int)size);
#endif
    exit(1);
}

static pthread_rwlock_t _rwl_releaseCounter;
static pthread_mutex_t _mutex_time2tm;
static unsigned int _releaseCounter = 0;

int InitCommon(void)
{
    return pthread_rwlock_init(&_rwl_releaseCounter, 0) + pthread_mutex_init(&_mutex_time2tm, 0);
}

void *MemoryRequest(size_t size, const char *sourceFile, unsigned int lineNumber)
{
    void *ptr;

    ptr = malloc(size);
    if (!ptr)
        _MemoryRequestFailed(size, sourceFile, lineNumber);

    if (pthread_rwlock_wrlock(&_rwl_releaseCounter) == 0)
    {
        _releaseCounter += 1;
        pthread_rwlock_unlock(&_rwl_releaseCounter);
    }
    else
    {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] RW Lock failed.\n");
#endif
    }

    return ptr;
}

void MemoryRelease(void *ptr)
{
    free(ptr);

    if (pthread_rwlock_wrlock(&_rwl_releaseCounter) == 0)
    {
        _releaseCounter -= 1;
        pthread_rwlock_unlock(&_rwl_releaseCounter);
    }
    else
    {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] RW Lock failed.\n");
#endif
    }
}

unsigned int _DebugGetReleaseCounter(void)
{
    unsigned int r = 0;

    if (pthread_rwlock_rdlock(&_rwl_releaseCounter) == 0)
    {
        r = _releaseCounter;
        pthread_rwlock_unlock(&_rwl_releaseCounter);
    }
    else
    {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] RW Lock failed.\n");
#endif
    }

    return r;
}

unsigned long GetThreadID(pthread_t thread)
{
    unsigned long ret = 0;
    memcpy(&ret, &thread, sizeof(thread));
    return ret;
}

void time2tm(const time_t *timer, struct tm *t)
{
    struct tm *_t;
    pthread_mutex_lock(&_mutex_time2tm);
    _t = localtime(timer);
    memcpy(t, _t, sizeof(*_t));
    pthread_mutex_unlock(&_mutex_time2tm);
}
