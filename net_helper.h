#ifndef NET_HELPER_H
#define NET_HELPER_H

#include <stdint.h>
#include <sys/types.h>

typedef struct Connection {
    int fd;
    char peer_ip[46];
    uint16_t peer_port;
    char username[20];
} Connection;

int listener_create(const char *ip, uint16_t port);
Connection *listener_accept(int listener_fd);
Connection *client_connect(const char *ip, uint16_t port);
ssize_t conn_read(Connection *c, void *buf, size_t len);
ssize_t conn_write(Connection *c, const void *buf, size_t len);
void conn_close(Connection *c);
void listener_close(int listener_fd);


#endif
