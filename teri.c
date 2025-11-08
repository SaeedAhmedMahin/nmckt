
#include <stdlib.h>
#include "net_helper.h"
#include <ncurses.h>
#include <menu.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

WINDOW *create_newwin(int height, int width, int starty, int startx);

char *choices[] = {
    "Terry",
    "Makichut",
    "George",
    "Jon Blow",
};
//making a makeshift ncurses window generator, and browsable list. use a makeshift list for displaying stuff.
//later on, we will turn it into a function we can use in other files.
int main(){
    ITEM **my_items;
    int c;
    MENU *my_menu;
    int n_choices, i;
    ITEM *cur_item;
    //WINDOW *users = create_newwin(LINES * 0.7, COLS/8, 2, 2);
    //WINDOW *users = create_newwin(14, 4, 2, 2);
    WINDOW *users;
    WINDOW *wchat;
    WINDOW *winp;
    WINDOW *vwinp;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

    for(i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], "-Online");
    my_items[n_choices] = (ITEM *)NULL;

    my_menu = new_menu((ITEM **)my_items);
    mvprintw(LINES - 2, 0, "F1 to exit");
    //put menu in that shi
    //wborder(users, '|', '|', '-', '-', '+', '+', '+', '+');
    refresh();
    users = create_newwin(LINES * 0.7, COLS/8, 2, 1);
    wchat = create_newwin(LINES * 0.7, COLS * (6.5/8), 2, COLS/7.4);
    winp = newwin(LINES * 0.2 - 2, COLS * (6.5/8) - 2, LINES * 0.73 + 1 , COLS/7.4 + 1);
    vwinp = create_newwin(LINES * 0.2, COLS * (6.5/8), LINES * 0.73, COLS/7.4);
    //users = newwin(LINES * 0.7, COLS/8, 2, 2);
    keypad(users, TRUE);
    keypad(winp, TRUE);
    
    set_menu_win(my_menu, users);
    set_menu_sub(my_menu, derwin(users, LINES * 0.7, COLS/8, 0, 0));

    //box(users, 0, 0);
    mvwprintw(users, 1, 1, "Sex:");
    //draw a gayass chat history
    mvwprintw(wchat, 0.8, 1, "W Speed");
    wrefresh(wchat);
    //finalize menu
    post_menu(my_menu);
    wrefresh(users);
    WINDOW *active = users;
    //textinput string for input box
    char message[512];
    int j = 0;
    while((c = wgetch(active)) != KEY_F(1)){
        switch(c)
        {    case KEY_DOWN:
                if (active == users);
                menu_driver(my_menu, REQ_DOWN_ITEM);
                wrefresh(users);
                break;
            case KEY_UP:
                if (active == winp);
                menu_driver(my_menu, REQ_UP_ITEM);
                wrefresh(users);
                break;
            case KEY_F(2):
                active = users;
                break;
            case KEY_F(3):
                active = winp;
                break;
                
        }
                if (active == winp && c > 31 && c < 128){
                    /*message[j++] = c;
                    message[j] = '\0';
                    mvwprintw(winp, 0, 0, "%s", message);*/
                    waddch(winp, (chtype)c);
                    wrefresh(winp);
                    j++;
                }else if (active == winp && c == '\n'){
                    winnstr(winp, message, j);
                    wclear(winp);
                    wrefresh(winp);
                    j = 0;
                }
    }
    free_item(my_items[0]);
    free_item(my_items[1]);
    free_menu(my_menu);
    endwin();

}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);

    wrefresh(local_win);

    return local_win;
}
