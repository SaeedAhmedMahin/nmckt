#include "net_helper.h"
#include <ncurses.h>
#include <menu.h>

char *choices[] = {
	"Choice 1",
	"Choice 2",
	"Choice 3",
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

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	n_choices = ARRAY_SIZE(choices);
	my_items = (ITEM **)calloc(n_choices + 1, size_of(ITEM *));

	for(i = 0; i < n_choices; ++i){
		my_items[i] = new_item(choices[i], choices[i]);
	my_items[n_choices] = (ITEM *)NULL;

	my_menu = new_menu((ITEM **)my_items);
	mvprintw(LINES - 2, 0, "F1 to exit");
	post_menu(my_menu);
	refresh();

	while((c = getch()) != KEY_F(1))
	{
		switch(c)
		{	case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
					break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;
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
