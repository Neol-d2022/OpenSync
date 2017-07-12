#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define INDEX_CACHE_FILENAME ".index.cache"

typedef struct
{
    _off_t fileSize;
    time_t fileMTime;
    unsigned int fileCRC32;
    char *filename;
} CachedFileIndex_t;

int InitCommon(void);

void *MemoryRequest(size_t size, const char *sourceFile, unsigned int lineNumber);
void MemoryRelease(void *ptr);
unsigned int _DebugGetReleaseCounter(void);

unsigned long GetThreadID(pthread_t thread);

void time2tm(const time_t *timer, struct tm *t);

int isDir(unsigned short mask);
int isFile(unsigned short mask);

void ConcatPath(char *buf, const char *name);

#endif
