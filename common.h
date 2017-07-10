#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdlib.h>
#include <pthread.h>

int InitCommon(void);

void *MemoryRequest(size_t size);
void MemoryRelease(void *ptr);
unsigned int _DebugGetReleaseCounter(void);

unsigned long GetThreadID(pthread_t thread);

#endif
