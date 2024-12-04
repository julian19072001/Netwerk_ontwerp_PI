// gcc -Wall -o terminal terminal.c ~/hva_libraries/rpitouch/*.c -I/home/julian/Netwerk_ontwerp_PI/hva_libraries/rpitouch -lncurses

#include <ncurses.h>
#include <rpitouch.h>
#include <unistd.h>

int main(int nArgc, char* aArgv[]) {

    int nRet;

    // Start to search for the correct event-stream
     nRet = RPiTouch_InitTouch();
    if (nRet < 0) {
        printf("RaspberryPi 7\" Touch display is not found!\nError %d\n\n", nRet);
        return -1;
    }

    // Init ncurses
    initscr();
    clear();
    noecho();
    cbreak();

    int rows, cols;
    getmaxyx(stdscr, rows, cols); // Get terminal size
   
    WINDOW *shutdownWindow = newwin(3, 16, 0, cols - 16);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);   // Pair 1: Red text, black background
    wattron(shutdownWindow, COLOR_PAIR(1));                  // Activate color pair 1
    mvwprintw(shutdownWindow, 0, 0, "                                                ");
    mvwprintw(shutdownWindow, 1, 4, "Shutdown");
    wattroff(shutdownWindow, COLOR_PAIR(1));                 // Deactivate color pair
    wrefresh(shutdownWindow);


    sleep(5);

    // Close the device
    nRet = RPiTouch_CloseTouch();
    if (nRet < 0) {
        printw("Close error %d!\n", nRet);
    }

    // Close ncurses
    endwin();

    return 0;
}