
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


typedef struct Connection {
    int fd;                      // socket file descriptor
    char peer_ip[INET6_ADDRSTRLEN]; // End point of connection ip
    uint16_t peer_port;             // ||                      port
    char username[20];
} Connection;

/* Create a listening socket.
 * ip: e.g. "0.0.0.0" or "127.0.0.1". If NULL, binds to INADDR_ANY.
 * port: host byte order (e.g. 8080).
 * Returns listening socket fd on success, -1 on failure (errno set).
 */
int listener_create(const char *ip, uint16_t port) {
    int srvfd = -1;
    int opt = 1;

    srvfd = socket(AF_INET, SOCK_STREAM, 0);
    if (srvfd < 0) return -1;

    // Allow reuse of address/port
    if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(srvfd);
        return -1;
    }

    struct sockaddr_in addr4;
    memset(&addr4, 0, sizeof(addr4));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(port);
    if (ip == NULL || strcmp(ip, "0.0.0.0") == 0) {
        addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_pton(AF_INET, ip, &addr4.sin_addr) != 1) {
            close(srvfd);
            errno = EINVAL;
            return -1;
        }
    }

    if (bind(srvfd, (struct sockaddr *)&addr4, sizeof(addr4)) < 0) {
        close(srvfd);
        return -1;
    }

    if (listen(srvfd, 10) < 0) {
        close(srvfd);
        return -1;
    }

    return srvfd;
}

/* Accept a single incoming connection on listener_fd.
 * Returns a freshly malloc'ed Connection* on success, NULL on failure (errno set).
 * Caller must call conn_close() to free.
 */
Connection *listener_accept(int listener_fd) {
    struct sockaddr_in peer;
    socklen_t plen = sizeof(peer);
    int client_fd = accept(listener_fd, (struct sockaddr *)&peer, &plen);
    if (client_fd < 0) return NULL;

    Connection *c = malloc(sizeof(Connection));
    if (!c) {
        close(client_fd);
        errno = ENOMEM;
        return NULL;
    }

    c->fd = client_fd;
    // store printable peer address
    if (inet_ntop(AF_INET, &peer.sin_addr, c->peer_ip, sizeof(c->peer_ip)) == NULL) {
        // fallback if inet_ntop fails
        strncpy(c->peer_ip, "unknown", sizeof(c->peer_ip));
        c->peer_ip[sizeof(c->peer_ip)-1] = '\0';
    }
    c->peer_port = ntohs(peer.sin_port);
    return c;
}

/* Connect to server at ip:port. Returns Connection* on success, NULL on failure (errno set). */
Connection *client_connect(const char *ip, uint16_t port) {
    if (!ip) { errno = EINVAL; return NULL; }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return NULL;

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv.sin_addr) != 1) {
        close(sockfd);
        errno = EINVAL;
        return NULL;
    }

    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        close(sockfd);
        return NULL;
    }

    Connection *c = malloc(sizeof(Connection));
    if (!c) {
        close(sockfd);
        errno = ENOMEM;
        return NULL;
    }

    c->fd = sockfd;
    strncpy(c->peer_ip, ip, sizeof(c->peer_ip));
    c->peer_ip[sizeof(c->peer_ip)-1] = '\0';
    c->peer_port = port;
    return c;
}

/* Read up to len bytes from connection into buf.
 * Returns number of bytes read (0 = peer closed), or -1 on error (errno set).
 * This is a thin wrapper around read(2).
 */
ssize_t conn_read(Connection *c, void *buf, size_t len) {
    if (!c || c->fd < 0) { errno = EBADF; return -1; }
    return read(c->fd, buf, len);
}

/* Write len bytes from buf to connection.
 * Returns number of bytes written or -1 on error (errno set).
 */
ssize_t conn_write(Connection *c, const void *buf, size_t len) {
    if (!c || c->fd < 0) { errno = EBADF; return -1; }
    return write(c->fd, buf, len);
}

/* Close connection and free object */
void conn_close(Connection *c) {
    if (!c) return;
    if (c->fd >= 0) close(c->fd);
    free(c);
}

/* Close listener socket */
void listener_close(int listener_fd) {
    if (listener_fd >= 0) close(listener_fd);
}


 
