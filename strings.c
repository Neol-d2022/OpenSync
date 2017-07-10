#include <string.h>

#include "common.h"

char *StringsRemoveNewline(char *str)
{
    char *p;

    p = strchr(str, '\n');
    if (p)
        *p = '\0';
    p = strchr(str, '\r');
    if (p)
        *p = '\0';

    return str;
}

char **StringsSplit(const char *str, int ch, unsigned int *count)
{
    char **ret, *n;
    const char *p, *q;
    size_t l, d;
    unsigned int c = 0, i = 0, j;

    l = strlen(str);
    p = str;
    while ((p = strchr(p, ch)))
    {
        p++;
        c++;
    }

    ret = (char **)MemoryRequest(sizeof(*ret) * (c + 2), __FILE__, __LINE__);
    if (!ret)
        return 0;

    q = p = str;
    while ((p = strchr(p, ch)))
    {
        d = ((size_t)p - (size_t)q) + 1;
        n = (char *)MemoryRequest(d, __FILE__, __LINE__);
        if (!n)
            goto StringSplit_failure;
        memcpy(n, q, d - 1);
        n[d - 1] = 0;
        ret[i] = n;
        p++;
        i++;
        q = p;
    }

    d = (((size_t)str + l) - (size_t)q) + 1;
    n = (char *)MemoryRequest(d, __FILE__, __LINE__);
    if (!n)
        goto StringSplit_failure;
    memcpy(n, q, d - 1);
    n[d - 1] = 0;
    ret[i] = n;

    if (count)
        *count = c + 1;
    ret[c + 1] = 0;
    return ret;

StringSplit_failure:
    for (j = 0; j < i; j += 1)
        MemoryRelease(ret[j]);
    MemoryRelease(ret);

    return 0;
}
