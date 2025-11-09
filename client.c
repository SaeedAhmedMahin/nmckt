#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>
#include <menu.h>

#include "net_helper.h" // or #include "net_helper_basic.h"

#define BUF 512
#define ARRAY_SIZE(arr) / sizeof((arr)[0])


// Like whatsapp, locally stored chat info
char username[BUF];
int global_chat = 1;
char path_global[100] = "./DB/global.txt";
char *path = NULL;
char *path2 = NULL;
char *history[BUF];


WINDOW *create_newwin(int height, int width, int starty, int startx);




void load_history(char path[100]){
    int i = 0;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("Error: Failed to open chat database");
        return;
    }
    char line[BUF];
    printf("/***********-------Previous Chat-------***********\n");
    while (fgets(line, sizeof(line), fp)) {
        
        printf("%s", line);
        history[i] = line;
        i++;
    }
    printf("\n");
    fclose(fp);
   
}



static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

void write_to_DB(char path[100], char line[256]) {
    // Threading
    pthread_mutex_lock(&db_mutex);

    FILE *fp = fopen(path, "a");
    if (!fp) {
        perror("Error: Failed to open chat database");
        pthread_mutex_unlock(&db_mutex);  // unlock before returning
        return;
    }

    if (fprintf(fp, "%s: %s\n",username, line) < 0) {
        perror("Error: Failed to write to chat database");
        fclose(fp);
        pthread_mutex_unlock(&db_mutex);
        return;
    }

    fflush(fp);
    fclose(fp);

    // Unlock after done
    pthread_mutex_unlock(&db_mutex);
}

int main(int argc, char *argv[]) {
    WINDOW *users;
    WINDOW *wchat;
    WINDOW *winp;
    WINDOW *vwinp;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    users = create_newwin(LINES * 0.7, COLS/8, 2, 1);
    wchat = create_newwin(LINES * 0.7, COLS * (6.5/8), 2, COLS/7.4);
    winp = newwin(LINES * 0.2 - 2, COLS * (6.5/8) - 2, LINES * 0.73 + 1 , COLS/7.4 + 1);
    vwinp = create_newwin(LINES * 0.2, COLS * (6.5/8), LINES * 0.73, COLS/7.4);


    
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

    mvwprintw(wchat, 1, 1, "Connected to server %s:%u\n", server_ip, port);
    mvwprintw(wchat, 2, 1, "Type ':q' to quit.\n");
    wrefresh(wchat);
    char buf[BUF];
    int logged_in = 0;
    while(!logged_in){
        char  password[BUF];
        mvwprintw(wchat, 3, 1, "Enter User Name: ");
        wrefresh(wchat);
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
       
        printf("%s:", username);
        if (fgets(buf, BUF, stdin)){
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
               
                if (strncmp(buf, "/", 1) == 0){
                    if (strncmp(buf, "/global", 7) == 0){
                        global_chat = 1;
                        load_history(path_global);

                    }
                    else if (strncmp(buf, "/active", 7) == 0){
                        // LOAD ACTIVE USERS
                        FILE *fp_active_users = fopen("./DB/active_users.txt", "r");
                        if (!fp_active_users) {
                            perror("Error opening user files");
                            return 0;
                        }
                        int no_of_active_users = 0;
                        char active_users[100][20];
                        while (fgets(active_users[no_of_active_users], sizeof(active_users[no_of_active_users]), fp_active_users)) {
                                // remove newline if present
                                active_users[no_of_active_users][strcspn(active_users[no_of_active_users], "\n")] = '\0';
                                no_of_active_users++;
                            }

                        fclose(fp_active_users);

                            // print active users
                        if (no_of_active_users == 1){
                            printf("1 Active User: [ %s ]\n", username);
                        }
                        else{
                            printf("%d Active Users: [", no_of_active_users);
                            for (int i = 0; i < no_of_active_users; i++) {
                                printf(" %s", active_users[i]);
                            }
                            printf(" ]\n" );
                        }
                        
                        
                    }
                    else{
                        global_chat = 0;
                        char *to = strtok(buf, "/");
                        
                            if (asprintf(&path, "./DB/chats/%s/%s.txt", username, to) == -1) {
                                perror("asprintf");
                                return 0;
                            }
                        if (asprintf(&path2, "./DB/chats/%s/%s.txt", to, username) == -1) {
                            perror("asprintf");
                            return 0;
                        }
                        load_history(path);

                    }
                }
                else {
                    // check for quit command
                    if (strcmp(buf, ":q") == 0) break;
                        // send to server
                    if (global_chat){
                        write_to_DB(path_global,buf);
                        load_history(path_global);
                        
                        }
                    else{
                        write_to_DB(path,buf);
                        write_to_DB(path2,buf);
                        load_history(path);
                    }
                    conn_write(srv, buf, (unsigned int) strlen(buf));
                        
                       
                        
                        // read server reply
                        ssize_t n = conn_read(srv, buf, (unsigned int) (BUF - 1));
                        if (n > 0) {
                        buf[n] = '\0';
                       
                        } else if (n == 0) {
                        printf("Server closed the connection.\n");
                        break;
                        } else {
                            perror("conn_read");
                            break;
                        }
                    }
                    
                    
                }
            
    }
        
    endwin();

    conn_close(srv);
    printf("Disconnected from server.\n");
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);

    wrefresh(local_win);

    return local_win;
}
