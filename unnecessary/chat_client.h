#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "net_helper.h"

typedef struct {
    Connection *conn;
} ChatClient;

void *receiver(void *arg);
void chat_client_run(const char *ip, unsigned int port);


#endif

