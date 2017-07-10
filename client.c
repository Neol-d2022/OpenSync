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

static void *ClientPathEntry(void *param)
{
    char fullPathFilename[FILENAME_MAX];
    char timebuf[32];
    struct stat s;
    DIR *dir;
    struct dirent *dentry;
    size_t len;
    struct tm time_st;
    const char *syncPath = (const char *)param;

    pthread_rwlock_rdlock(&_proceedFlag);
    if (_clientSyncProceed == 0)
    {
        pthread_rwlock_unlock(&_proceedFlag);
        return 0;
    }
    pthread_rwlock_unlock(&_proceedFlag);

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
        return 0;
    }

    while ((dentry = readdir(dir)))
    {
        if (strcmp(dentry->d_name, ".") == 0)
            continue;
        if (strcmp(dentry->d_name, "..") == 0)
            continue;
        
        memcpy(fullPathFilename, syncPath, len + 1);
        ConcatPath(fullPathFilename, dentry->d_name);
        if (fullPathFilename[0] == '\0')
        {
            fprintf(stderr, "[ERROR] Filename too long\n");
            continue;
        }
        if (stat(fullPathFilename, &s) != 0)
        {
            memset(&s, 0, sizeof(s));
            fprintf(stderr, "[ERROR] Cannot get stat from file \"%s\"(%d)\n", dentry->d_name, errno);
        }
        time2tm(&(s.st_mtime), &time_st);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &time_st);
        printf("[DEBUG] M:%hx\tD:%i\tF:%i\t%s\t%s\n", s.st_mode, isDir(s.st_mode), isFile(s.st_mode), timebuf, fullPathFilename);
    }
    closedir(dir);

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
