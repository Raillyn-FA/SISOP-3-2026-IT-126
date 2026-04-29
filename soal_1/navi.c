#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "protocol.h"

int sock;

void *receive_msg(void *arg)
{
    (void)arg;

    char msg[BUFFER_SIZE];

    while(1)
    {
        int n = recv(sock, msg, sizeof(msg)-1, 0);

        if(n <= 0)
            break;

        msg[n] = '\0';
        printf("%s", msg);
    }

    return NULL;
}

int main()
{
    struct sockaddr_in server_addr;
    char name[100];
    char msg[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    send(sock, name, strlen(name), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, receive_msg, NULL);

    while(1)
    {
        fgets(msg, sizeof(msg), stdin);
        trim_newline(msg);

        send(sock, msg, strlen(msg), 0);

        if(strcmp(msg, "/exit")==0)
            break;
    }

    close(sock);
}
