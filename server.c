#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "net_helper.h"

#define BUF 512
#define MAX_CLIENTS 100  // max concurrent clients
char messages[100][4][50];
char username_code[3] = {'1','0','1'};

int no_of_active_users = 0;

char active_users[100][20];

    /* copy the literals into the buffers */
    
//    strcpy(active_users[1], "Shammo");

    /* print the stored names */
//    for (int i = 0; i < 2; ++i) {
//        int j = 0;
//        while (active_users[i][j] != '\0') {
//            putchar(active_users[i][j]);
//            ++j;
//        }
//        putchar('\n');
//    }


int verify_login(char *username, char *password) {
    FILE *fp_user_data = fopen("./DB/users.txt", "r");
    if (!fp_user_data) {
        perror("Error opening user files");
        return 0;
    }
    char name[BUF], pass[BUF];
    int id;
    
    // ACTIVE USERS LOGIC
    while (fscanf(fp_user_data, "%d %s %s", &id, name, pass) == 3) {
        if (strcmp(username, name) == 0 && strcmp(password, pass) == 0) {
            strcpy(active_users[no_of_active_users], name );
            no_of_active_users++;
            fclose(fp_user_data);
            return 1;
        }
    }
    return 0;
}

typedef struct {
    Connection *client;
} ClientThreadArgs;

void *client_handler(void *arg) {
    ClientThreadArgs *args = arg;
    Connection *c = args[0].client;
    free(arg);

    char buf[BUF];
    ssize_t n;
    int logged_in = 0;

    while ((n = conn_read(c, buf, BUF - 1)) > 0) {
        buf[n] = '\0';
        if (!logged_in) {
            if (strncmp(buf, "LOGIN:", 6) == 0) {
                char *credentials = buf + 6;
                char *username = strtok(credentials, "|");
                char *password = strtok(NULL, "|");

                if (verify_login(username, password)) {
                    logged_in = 1;
                    conn_write(c, "OK", 2);
                    printf("User %s logged in.\n", username);
                    
                } else {
                    conn_write(c, "INVALID", 7);
                    printf("Invalid login attempt from %s:%u\n", c->peer_ip, c->peer_port);
                }
            } else {
                conn_write(c, "Please login first", 18);
            }
        } else {
            // CHAT LOGIC
            printf("[%s:%u]: %s\n", c->peer_ip, c->peer_port, buf);
            // printf("%s: %s\n", username, buf);

            if (strncmp(buf, ":q", 2) == 0) {
                printf("Client [%s:%u] requested to quit.\n", c->peer_ip, c->peer_port);
                break;
            }

            // echo back (or later: broadcast)
            conn_write(c, buf, (unsigned int)n);
        }
    }

    if (n < 0) perror("conn_read");

    conn_close(c);
    printf("Client [%s:%u] disconnected.\n", c[0].peer_ip, c[0].peer_port);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "Example to run server in terminal:\n");
        fprintf(stderr, "  ./server 9000\n"); // comment for terminal
        return 1;
    }
    
    unsigned int port = (unsigned int) atoi(argv[1]);
    if (port == 0) {
        fprintf(stderr, "Invalid port number.\n");
        return 1;
    }

    int listenfd = listener_create(NULL, port);
    if (listenfd < 0) {
        perror("listener_create");
        return 1;
    }

    printf("Server listening on port %u...\n", port);

    unsigned int client_count = 0;

    while (client_count < MAX_CLIENTS) {
        Connection *client = listener_accept(listenfd);
        if (!client) {
            perror("listener_accept");
            continue;
        }

        client_count++;
        printf("New client connected: %s:%u\n", client->peer_ip, client->peer_port);

        ClientThreadArgs *args = malloc(sizeof(ClientThreadArgs));
        args->client = client;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, args) != 0) {
            perror("pthread_create");
            conn_close(client);
            free(args);
        } else {
            pthread_detach(tid);
        }
    }

    printf("Server served %u clients, shutting down.\n", MAX_CLIENTS);
    listener_close(listenfd);
    return 0;
}
