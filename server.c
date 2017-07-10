#include "server.h"

#include "config.h"

int ServerInit(void *config)
{
    const Configuration_t *c = (const Configuration_t *)config;
    if (!IsServerEnabled(c))
        return 0;

    return 0;
}
