
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "net_helper.h"

#define MAX_CLIENTS 100
#define BUF 512

typedef struct {
    Connection *conn;
    char name[50];
} Client;

static Client *clients[MAX_CLIENTS];
static pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;

// broadcast message to all connected clients
void broadcast(const char *msg, Connection *exclude) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->conn != exclude) {
            conn_write(clients[i]->conn, msg, strlen(msg));
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

void *client_thread(void *arg) {
    Client *c = (Client *)arg;
    char buf[BUF];
    ssize_t n;

    sprintf(buf, "[Server] %s joined the chat\n", c->name);
    broadcast(buf, c->conn);

    while ((n = conn_read(c->conn, buf, BUF - 1)) > 0) {
        buf[n] = '\0';
        if (strncmp(buf, ":q", 2) == 0) {
            sprintf(buf, "[Server] %s left the chat\n", c->name);
            broadcast(buf, c->conn);
            break;
        }

        char msg[BUF + 64];
        snprintf(msg, sizeof(msg), "%s: %s\n", c->name, buf);
        broadcast(msg, c->conn);
    }

    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == c) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_lock);

    conn_close(c->conn);
    free(c);
    pthread_exit(NULL);
}

void chat_server_run(unsigned int port) {
    int listenfd = listener_create(NULL, port);
    if (listenfd < 0) {
        perror("listener_create");
        exit(1);
    }

    printf("Chat server started on port %u...\n", port);

    while (1) {
        Connection *conn = listener_accept(listenfd);
        if (!conn) continue;

        pthread_mutex_lock(&clients_lock);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i]) {
                Client *c = malloc(sizeof(Client));
                c->conn = conn;
                snprintf(c->name, sizeof(c->name), "User%d", i + 1);
                clients[i] = c;

                pthread_t tid;
                pthread_create(&tid, NULL, client_thread, c);
                pthread_detach(tid);
                break;
            }
        }
        pthread_mutex_unlock(&clients_lock);

        if (i == MAX_CLIENTS) {
            char *msg = "Server full. Try again later.\n";
            conn_write(conn, msg, strlen(msg));
            conn_close(conn);
        }
    }
}

