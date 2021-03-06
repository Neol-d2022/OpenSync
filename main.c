#include <stdio.h>
#include <pthread.h>

void *ClientThreadEntry(void *param)
{
#ifdef DEBUG
    pthread_t selfThread = pthread_self();
    
    printf("[DEBUG] %u in ClientThreadEntry(%s, %i)\n", (unsigned)selfThread.x, __FILE__, __LINE__);
#endif
    return param;
}

void *ServerThreadEntry(void *param)
{
#ifdef DEBUG
    pthread_t selfThread = pthread_self();

    printf("[DEBUG] %u in ServerThreadEntry(%s, %i)\n", (unsigned)selfThread.x, __FILE__, __LINE__);
#endif
    return param;
}

int main(void)
{
    void *retParam;
    pthread_t clientThread, serverThread;
    int retCode;

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