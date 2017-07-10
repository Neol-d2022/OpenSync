#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "common.h"
#include "strings.h"

const char kPathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

int LoadConfigFile(const char *filename, Configuration_t *c)
{
    char buf[256];
    Configuration_t _c;
    FILE *f;
    char **tokens;
    size_t len;
    unsigned int ntokens, i, lincCount, in;

    f = fopen(filename, "r");
    if (!f)
    {
        fprintf(stderr, "[ERROR] Cannot open file \"%s\"\n", filename);
        return errno;
    }

    lincCount = 0;
    memset(&_c, 0, sizeof(_c));
    while (fgets(buf, sizeof(buf), f))
    {
        StringsRemoveNewline(buf);
        lincCount += 1;
        tokens = StringsSplit(buf, '=', &ntokens);

        if (ntokens == 2)
        {
            if (strcmp(tokens[0], "Client"))
            {
                if (sscanf(tokens[1], "%u", &in) == 1)
                {
                    if (_c.functionFlag & CLIENT_FUNCTION_FLAG)
                    {
                        fprintf(stderr, "[ERROR] line %u, duplicated settings.\n", lincCount);
                    }
                    else
                    {
                        if (in == 1)
                            c->functionFlag |= CLIENT_FUNCTION_FLAG;
                        _c.functionFlag |= CLIENT_FUNCTION_FLAG;
                    }
                }
                else
                {
                    fprintf(stderr, "[ERROR] line %u, format error.\n", lincCount);
                }
            }
            else if (strcmp(tokens[0], "Server"))
            {
                if (sscanf(tokens[1], "%u", &in) == 1)
                {
                    if (_c.functionFlag & SERVER_FUNCTION_FLAG)
                    {
                        fprintf(stderr, "[ERROR] line %u, duplicated settings.\n", lincCount);
                    }
                    else
                    {
                        if (in == 1)
                            c->functionFlag |= SERVER_FUNCTION_FLAG;
                        _c.functionFlag |= SERVER_FUNCTION_FLAG;
                    }
                }
                else
                {
                    fprintf(stderr, "[ERROR] line %u, format error.\n", lincCount);
                }
            }
            else if (strcmp(tokens[0], "ClientSyncPathFilename"))
            {
                if (_c.clientSyncPathFilename != 0)
                {
                    fprintf(stderr, "[ERROR] line %u, duplicated settings.\n", lincCount);
                }
                else
                {
                    c->clientSyncPathFilename = MemoryRequest((len = strlen(tokens[1])) + 1, __FILE__, __LINE__);
                    memcpy(c->clientSyncPathFilename, tokens[1], len);
                    c->clientSyncPathFilename[len] = '\0';
                    _c.clientSyncPathFilename = (char *)1;
                }
            }
            else if (strcmp(tokens[0], "ServerSyncPathFilename"))
            {
                if (_c.serverSyncPathFilename != 0)
                {
                    fprintf(stderr, "[ERROR] line %u, duplicated settings.\n", lincCount);
                }
                else
                {
                    c->serverSyncPathFilename = MemoryRequest((len = strlen(tokens[1])) + 1, __FILE__, __LINE__);
                    memcpy(c->serverSyncPathFilename, tokens[1], len);
                    c->serverSyncPathFilename[len] = '\0';
                    _c.serverSyncPathFilename = (char *)1;
                }
            }
            else
            {
                fprintf(stderr, "[ERROR] line %u, unknown key.\n", lincCount);
            }
        }
        else
        {
            fprintf(stderr, "[ERROR] line %u, format error.\n", lincCount);
        }

        for (i = 0; i < ntokens; i += 1)
            MemoryRelease(tokens[i]);
        MemoryRelease(tokens);
    }

    fclose(f);
    return 0;
}

int IsClientEnabled(const Configuration_t *c)
{
    return (c->functionFlag & CLIENT_FUNCTION_FLAG) ? 1 : 0;
}

int IsServerEnabled(const Configuration_t *c)
{
    return (c->functionFlag & SERVER_FUNCTION_FLAG) ? 1 : 0;
}
