#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>
#include "net_helper.h"

#define BUF 512

typedef struct {
    Connection *conn;
} ChatClient;

WINDOW *inputwin;
pthread_mutex_t io_lock = PTHREAD_MUTEX_INITIALIZER;

void *receiver(void *arg) {
    ChatClient *client = arg;
    char buf[BUF];
    ssize_t n;

    // Print incoming messages directly to stdout (not ncurses)
    while ((n = conn_read(client->conn, buf, BUF - 1)) > 0) {
        buf[n] = '\0';

        pthread_mutex_lock(&io_lock);
        // Clear input line first
        printf("\r\33[K"); // erase current input line
        printf("%s\n", buf);
        printf("> "); // reprint prompt
        fflush(stdout);
        pthread_mutex_unlock(&io_lock);
    }

    printf("\n[Disconnected from server]\n");
    pthread_exit(NULL);
}

void chat_client_run(const char *ip, unsigned int port) {
    Connection *conn = client_connect(ip, port);
    if (!conn) {
        perror("client_connect");
        exit(1);
    }

    initscr();
    cbreak();
    noecho();
    curs_set(TRUE);
    endwin(); // we'll use plain terminal I/O for stability

    ChatClient client = {conn};
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receiver, &client);

    char buf[BUF];
    printf("> ");
    fflush(stdout);

    while (fgets(buf, BUF, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';

        if (strcmp(buf, ":q") == 0) {
            conn_write(conn, buf, strlen(buf));
            break;
        }

        conn_write(conn, buf, strlen(buf));
        printf("> ");
        fflush(stdout);
    }

    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    conn_close(conn);

    printf("\nDisconnected.\n");
}
