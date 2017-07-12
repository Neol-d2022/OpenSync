#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "client.h"
#include "config.h"
#include "common.h"

static pthread_rwlock_t _proceedFlag;
static int _clientSyncProceed;

typedef struct _linkedCachedFileIndex_struct_t
{
    CachedFileIndex_t *index;
    struct _linkedCachedFileIndex_struct_t *next;
} _linkedCachedFileIndex_t;

static CachedFileIndex_t **_processDirs(const char *syncPath, unsigned int *count)
{
    char fullPathFilename[FILENAME_MAX];
    struct stat s;
    struct dirent *dentry;
    DIR *dir;
    _linkedCachedFileIndex_t *head, *n, **cur, *_cur;
    CachedFileIndex_t **ret;
    size_t len, size;
    unsigned int c = 0, i;

    dir = opendir(syncPath);
    if (dir == NULL)
    {
        fprintf(stderr, "[ERROR] Cannot read file list under \"%s\"\n", syncPath);
        return 0;
    }

    len = strlen(syncPath);
    if (len >= FILENAME_MAX)
    {
        fprintf(stderr, "[ERROR] Filename too long\n");
        closedir(dir);
        return 0;
    }

    head = 0;
    cur = &head;
    while ((dentry = readdir(dir)))
    {
        if (strcmp(dentry->d_name, ".") == 0)
            continue;
        if (strcmp(dentry->d_name, "..") == 0)
            continue;
        if (strcmp(dentry->d_name, INDEX_CACHE_FILENAME) == 0)
            continue;

        memcpy(fullPathFilename, syncPath, len + 1);
        ConcatPath(fullPathFilename, dentry->d_name);
        if (fullPathFilename[0] == '\0')
        {
            fprintf(stderr, "[ERROR] Filename too long\n");
            closedir(dir);
            continue;
        }
        if (stat(fullPathFilename, &s) != 0)
        {
            memset(&s, 0, sizeof(s));
            fprintf(stderr, "[ERROR] Cannot get stat from file \"%s\"(%d)\n", dentry->d_name, errno);
            continue;
        }

        if (isDir(s.st_mode))
        {

        }
        else if (isFile(s.st_mode))
        {
            n = (_linkedCachedFileIndex_t *)MemoryRequest(sizeof(*n), __FILE__, __LINE__);
            n->index = (CachedFileIndex_t *)MemoryRequest(sizeof(*(n->index)), __FILE__, __LINE__);
            n->index->fileSize = s.st_size;
            n->index->fileMTime = s.st_mtime;
            n->index->fileCRC32 = 0;
            size = strlen(dentry->d_name);
            n->index->filename = (char *)MemoryRequest(size + 1, __FILE__, __LINE__);
            memcpy(n->index->filename, dentry->d_name, size);
            (n->index->filename)[size] = '\0';
            c += 1;
            *cur = n;
            cur = &(n->next);
        }
        *cur = 0;
    }

    closedir(dir);

    if (count)
        *count = c;

    ret = (CachedFileIndex_t **)MemoryRequest(sizeof(*ret) * (c + 1), __FILE__, __LINE__);
    _cur = head;
    for (i = 0; i < c; i += 1)
    {
        ret[i] = _cur->index;
        n = _cur->next;
        free(_cur);
        _cur = n;
    }
    ret[c] = 0;

    return ret;
}

static int _refreshCacheIndex(const char *syncPath)
{
    FILE *f;
    CachedFileIndex_t **index;
    unsigned int count, i;

    f = fopen(INDEX_CACHE_FILENAME, "r");
    if (f == NULL)
    {
        f = fopen(INDEX_CACHE_FILENAME, "w");
        if (f == NULL)
        {
            fprintf(stderr, "[ERROR] Cannot create file \"%s\"\n", INDEX_CACHE_FILENAME);
            return 0;
        }
    }

    index = _processDirs(syncPath, &count);

    if (index == 0)
    {
        fprintf(stderr, "[ERROR] Cannot make cache\n");
        fclose(f);
        return 0;
    }

#ifdef DEBUG
    for (i = 0; i < count; i += 1)
    {
        printf("[DEBUG] %s\n\tS:%lu\n\tT:%lu\n\tC:%u\n\n", index[i]->filename, (unsigned long)index[i]->fileSize, (unsigned long)index[i]->fileMTime, index[i]->fileCRC32);
    }
#endif
    for (i = 0; i < count; i += 1)
    {
        MemoryRelease(index[i]->filename);
        MemoryRelease(index[i]);
    }
    MemoryRelease(index);

    fclose(f);
    return 0;
}

static void *ClientPathEntry(void *param)
{
    const char *syncPath = (const char *)param;

    pthread_rwlock_rdlock(&_proceedFlag);
    if (_clientSyncProceed == 0)
    {
        pthread_rwlock_unlock(&_proceedFlag);
        return 0;
    }
    pthread_rwlock_unlock(&_proceedFlag);

    _refreshCacheIndex(syncPath);

    return 0;
}

int ClientInit(void *config)
{
    pthread_t *threads;
    const Configuration_t *c = (const Configuration_t *)config;
    const char *syncFilename = c->clientSyncPathFilename;
    char **syncPath;
    void *ret;
    unsigned int syncPathCount, i, j;
    int _ret = 0;

    if (!IsClientEnabled(c))
        return 0;

    if (syncFilename == 0)
    {
        fprintf(stderr, "[ERROR] Sync path is not set.\n");
        return 1;
    }

    syncPath = LoadSyncPathIntoArray(syncFilename, &syncPathCount);
    if (syncPath == 0)
        return 1;

    threads = (pthread_t *)MemoryRequest(sizeof(*threads) * syncPathCount, __FILE__, __LINE__);
    _clientSyncProceed = 1;
    pthread_rwlock_init(&_proceedFlag, NULL);
    pthread_rwlock_wrlock(&_proceedFlag);
    for (i = 0; i < syncPathCount; i += 1)
    {
        if (pthread_create(threads + i, NULL, ClientPathEntry, syncPath[i]) != 0)
        {
            _ret = 1;
            fprintf(stderr, "[ERROR] pthread_create failed.\n");
            _clientSyncProceed = 0;
            pthread_rwlock_unlock(&_proceedFlag);
            for (j = 0; j < i; j += 1)
            {
                pthread_join(threads[j], &ret);
            }
            goto ClientInit_exit;
        }
    }
    pthread_rwlock_unlock(&_proceedFlag);

    for (i = 0; i < syncPathCount; i += 1)
        pthread_join(threads[i], &ret);

ClientInit_exit:
    MemoryRelease(threads);
    for (i = 0; i < syncPathCount; i += 1)
        MemoryRelease(syncPath[i]);
    MemoryRelease(syncPath);
    pthread_rwlock_destroy(&_proceedFlag);

    return _ret;
}
