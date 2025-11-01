#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_helper.h" // or #include "net_helper_basic.h"

#define BUF 512

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        fprintf(stderr, "Example to run client in terminal:\n");
        fprintf(stderr, "  ./client 127.0.0.1 9000\n"); // comment for terminal
        return 1;
    }

    const char *server_ip = argv[1];
    unsigned int port = atoi(argv[2]);
    if (port == 0) {
        fprintf(stderr, "Invalid port number.\n");
        return 1;
    }

    Connection *srv = client_connect(server_ip, port);
    if (!srv) {
        perror("client_connect");
        return 1;
    }

    printf("Connected to server %s:%u\n", server_ip, port);
    printf("Type ':q' to quit.\n");

    char buf[BUF];
    int logged_in = 0;
    while(!logged_in){
        char username[BUF], password[BUF];
        printf("Enter username: ");
        if (!fgets(username, BUF, stdin)) break;
        username[strcspn(username, "\n")] = '\0';

        printf("Enter password: ");
        if (!fgets(password, BUF, stdin)) break;
        password[strcspn(password, "\n")] = '\0';

        // concatenate with separator and prefix
        snprintf(buf, BUF, "LOGIN:%s|%s", username, password);

        conn_write(srv, buf, strlen(buf));
            ssize_t n = conn_read(srv, buf, BUF - 1);
            
        if (n > 0) {
            buf[n] = '\0';
            if (strcmp(buf, "OK") == 0) {
                logged_in = 1;
                printf("Logged in!\n");
            } else {
                printf("Login failed, try again.\n");
            }
        }
    }

    while (logged_in) {
        printf("You: ");
        if (!fgets(buf, BUF, stdin)) break;

        // remove trailing newline
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

        // send to server
        conn_write(srv, buf, (unsigned int) strlen(buf));

        // check for quit command
        if (strcmp(buf, ":q") == 0) break;

        // read server reply
        ssize_t n = conn_read(srv, buf, (unsigned int) (BUF - 1));
        if (n > 0) {
            buf[n] = '\0';
            printf("Server: %s\n", buf);
        } else if (n == 0) {
            printf("Server closed the connection.\n");
            break;
        } else {
            perror("conn_read");
            break;
        }
    }

    conn_close(srv);
    printf("Disconnected from server.\n");
    return 0;
}
