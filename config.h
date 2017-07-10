#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define CLIENT_FUNCTION_FLAG 0x1
#define SERVER_FUNCTION_FLAG 0x2

typedef struct
{
    unsigned char functionFlag;
} Configuration_t;

int LoadConfigFile(const char *filename, Configuration_t *c);

int IsClientEnabled(const Configuration_t *c);
int IsServerEnabled(const Configuration_t *c);

#endif
