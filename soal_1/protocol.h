#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string.h>

#define PORT 8080
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define ADMIN_NAME "The Knights"
#define ADMIN_PASS "protocol7"

static inline void trim_newline(char *str)
{
    int len = strlen(str);

    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

#endif
