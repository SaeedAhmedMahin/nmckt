#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "net_helper.h" 

typedef struct {
    Connection *conn;
    char name[50];
} Client;

void broadcast(const char *msg, Connection *exclude);
void *client_thread(void *arg);
void chat_server_run(unsigned int port);

#endif
