#include <ncurses.h>
#include "ui.h"

int main() {
    /**/
    /* Initialize ncurses */
    /**/
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /**/
    /* Handle the main menu */
    /**/
    handle_main_menu();

    /**/
    /* End ncurses mode */
    /**/
    endwin();

    return 0;
}
