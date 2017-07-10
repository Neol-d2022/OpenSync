#include <stdio.h>
#include <pthread.h>

#include "client.h"
#include "server.h"
#include "common.h"
#include "config.h"

void *ClientThreadEntry(void *param)
{
#ifdef DEBUG
    pthread_t selfThread = pthread_self();

    printf("[DEBUG] %u in ClientThreadEntry(%s, %i)\n", (unsigned int)GetThreadID(selfThread), __FILE__, __LINE__);
#endif
    if (ClientInit() != 0)
        fprintf(stderr, "[ERROR] Client function failed.\n");

    return param;
}

void *ServerThreadEntry(void *param)
{
#ifdef DEBUG
    pthread_t selfThread = pthread_self();

    printf("[DEBUG] %u in ServerThreadEntry(%s, %i)\n", (unsigned int)GetThreadID(selfThread), __FILE__, __LINE__);
#endif
    if (ServerInit() != 0)
        fprintf(stderr, "[ERROR] Server function failed.\n");

    return param;
}

int main(void)
{
    Configuration_t c;
    void *retParam;
    pthread_t clientThread, serverThread;
    int retCode;

    if (InitCommon() != 0)
    {
        fprintf(stderr, "[ERROR] Common function failed.\n");
        return 1;
    }

    if (LoadConfigFile("config.ini", &c) != 0)
    {
        fprintf(stderr, "[ERROR] Config load failed.\n");
        return 1;
    }

    retCode = pthread_create(&clientThread, NULL, ClientThreadEntry, NULL);
    if (retCode)
    {
        fprintf(stderr, "[ERROR] Creating client-side thread failed, exiting now.\n");
        return 1;
    }

    retCode = pthread_create(&serverThread, NULL, ServerThreadEntry, NULL);
    if (retCode)
    {
        fprintf(stderr, "[ERROR] Creating server-side thread failed, exiting now.\n");
        return 1;
    }

    retCode = pthread_join(clientThread, &retParam);
    if (retCode)
    {
        fprintf(stderr, "[ERROR] Joining client-side thread failed, exiting now.\n");
        return 1;
    }

    retCode = pthread_join(serverThread, &retParam);
    if (retCode)
    {
        fprintf(stderr, "[ERROR] Joining server-side thread failed, exiting now.\n");
        return 1;
    }

    return 0;
}
