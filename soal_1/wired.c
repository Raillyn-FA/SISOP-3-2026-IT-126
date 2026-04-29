#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "protocol.h"

typedef struct {
    int sock;
    char name[100];
    int is_admin;
} Client;

Client clients[MAX_CLIENTS];
int total_clients = 0;
int server_running = 1;

pthread_mutex_t lock;
time_t start_time;

/* ================= LOGGING ================= */

void write_log(const char *role, const char *msg)
{
    FILE *fp = fopen("history.log", "a");
    if (fp == NULL)
        return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp,
            "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s]\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            role,
            msg);

    fclose(fp);
}

/* ================= CLIENT MANAGEMENT ================= */

int username_exist(const char *name)
{
    for (int i = 0; i < total_clients; i++)
    {
        if (strcmp(clients[i].name, name) == 0)
            return 1;
    }
    return 0;
}

void remove_client(int sock)
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < total_clients; i++)
    {
        if (clients[i].sock == sock)
        {
            for (int j = i; j < total_clients - 1; j++)
            {
                clients[j] = clients[j + 1];
            }
            total_clients--;
            break;
        }
    }

    pthread_mutex_unlock(&lock);
}

/* ================= BROADCAST ================= */

void broadcast(const char *msg, int sender_sock)
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < total_clients; i++)
    {
        if (clients[i].sock != sender_sock)
        {
            send(clients[i].sock, msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

/* ================= ADMIN ================= */

void send_admin_menu(int sock)
{
    const char *menu =
        "\n--- THE KNIGHTS CONSOLE ---\n"
        "1. Check Active Users\n"
        "2. Check Server Uptime\n"
        "3. Shutdown Server\n"
        "4. Exit\n"
        "Command >> ";

    send(sock, menu, strlen(menu), 0);
}

void handle_admin(int sock)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        send_admin_menu(sock);

        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0)
            break;

        buffer[n] = '\0';
        trim_newline(buffer);

        if (strcmp(buffer, "1") == 0)
        {
            char msg[128];
            snprintf(msg, sizeof(msg),
                     "Active users: %d\n",
                     total_clients - 1);

            send(sock, msg, strlen(msg), 0);
            write_log("Admin", "RPC_GET_USERS");
        }
        else if (strcmp(buffer, "2") == 0)
        {
            time_t now = time(NULL);
            int uptime = (int)difftime(now, start_time);

            char msg[128];
            snprintf(msg, sizeof(msg),
                     "Uptime: %d seconds\n",
                     uptime);

            send(sock, msg, strlen(msg), 0);
            write_log("Admin", "RPC_GET_UPTIME");
        }
        else if (strcmp(buffer, "3") == 0)
        {
            write_log("Admin", "RPC_SHUTDOWN");
            write_log("System", "EMERGENCY SHUTDOWN INITIATED");

            server_running = 0;
            exit(0);
        }
        else if (strcmp(buffer, "4") == 0)
        {
            break;
        }
        else
        {
            send(sock, "Invalid command\n", 16, 0);
        }
    }
}

/* ================= THREAD CLIENT ================= */

void *handle_client(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char name[100];
    char buffer[BUFFER_SIZE];
    char msg[2048];
    char logmsg[2048];

    int n = recv(sock, name, sizeof(name) - 1, 0);
    if (n <= 0)
    {
        close(sock);
        return NULL;
    }

    name[n] = '\0';
    trim_newline(name);

    if (username_exist(name))
    {
        snprintf(msg, sizeof(msg),
                 "[System] The identity '%s' is already synchronized.\n",
                 name);

        send(sock, msg, strlen(msg), 0);
        close(sock);
        return NULL;
    }

    int is_admin = 0;

    /* ========== ADMIN LOGIN ========== */
    if (strcmp(name, ADMIN_NAME) == 0)
    {
        send(sock, "Enter Password: ", 16, 0);

        char pass[100];

        n = recv(sock, pass, sizeof(pass) - 1, 0);
        if (n <= 0)
        {
            close(sock);
            return NULL;
        }

        pass[n] = '\0';
        trim_newline(pass);

        if (strcmp(pass, ADMIN_PASS) == 0)
        {
            send(sock,
                 "[System] Authentication Successful.\n",
                 36, 0);

            is_admin = 1;
            write_log("System", "Admin connected");
            handle_admin(sock);
        }
        else
        {
            send(sock, "Wrong password\n", 15, 0);
            close(sock);
            return NULL;
        }
    }

    /* ========== ADD CLIENT ========== */

    pthread_mutex_lock(&lock);

    clients[total_clients].sock = sock;
    strncpy(clients[total_clients].name, name,
            sizeof(clients[total_clients].name) - 1);

    clients[total_clients].name[
        sizeof(clients[total_clients].name) - 1] = '\0';

    clients[total_clients].is_admin = is_admin;

    total_clients++;

    pthread_mutex_unlock(&lock);

    snprintf(msg, sizeof(msg),
             "--- Welcome to The Wired, %s ---\n",
             name);

    send(sock, msg, strlen(msg), 0);

    snprintf(logmsg, sizeof(logmsg),
             "User '%s' connected",
             name);

    write_log("System", logmsg);

    /* ========== CHAT LOOP ========== */

    while (1)
    {
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0)
            break;

        buffer[n] = '\0';
        trim_newline(buffer);

        if (strcmp(buffer, "/exit") == 0)
            break;

        snprintf(logmsg, sizeof(logmsg),
                 "[%s]: %s",
                 name, buffer);

        write_log("User", logmsg);

        snprintf(msg, sizeof(msg),
                 "[%s]: %s\n",
                 name, buffer);

        broadcast(msg, sock);
    }

    snprintf(logmsg, sizeof(logmsg),
             "User '%s' disconnected",
             name);

    write_log("System", logmsg);

    remove_client(sock);
    close(sock);

    return NULL;
}

/* ================= MAIN ================= */

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addrlen = sizeof(client_addr);

    pthread_mutex_init(&lock, NULL);
    start_time = time(NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("Server running on port %d...\n", PORT);
    write_log("System", "SERVER ONLINE");

    while (server_running)
    {
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addrlen);

        if (client_fd < 0)
            continue;

        int *pclient = malloc(sizeof(int));
        if (pclient == NULL)
            continue;

        *pclient = client_fd;

        pthread_t tid;

        pthread_create(&tid,
                       NULL,
                       handle_client,
                       pclient);

        pthread_detach(tid);
    }

    close(server_fd);
    pthread_mutex_destroy(&lock);

    return 0;
}
