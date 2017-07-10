#include "client.h"

#include "config.h"

int ClientInit(void *config)
{
    const Configuration_t *c = (const Configuration_t *)config;
    if (!IsClientEnabled(c))
        return 0;

    return 0;
}
