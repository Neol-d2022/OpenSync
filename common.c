#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

static void _MemoryRequestFailed(size_t size, const char *sourceFile, unsigned int lineNumber)
{
    fprintf(stderr, "[CRITICAL] Memory Request Failed.\n");
#ifdef DEBUG
    fprintf(stderr, "[CRITICAL] in %s at %u, size = %u.\n", sourceFile, lineNumber, (unsigned int)size);
#endif
    exit(1);
}

static unsigned int _releaseCounter = 0;

void *MemoryRequest(size_t size, const char *sourceFile, unsigned int lineNumber)
{
    void *ptr;

    _releaseCounter += 1;
    ptr = malloc(size);
    if (!ptr)
        _MemoryRequestFailed(size, sourceFile, lineNumber);
    return ptr;
}

void MemoryRelease(void *ptr)
{
    _releaseCounter -= 1;
    free(ptr);
}

unsigned long GetThreadID(pthread_t thread)
{
    return (unsigned long)thread;
}
