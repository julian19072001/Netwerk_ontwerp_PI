// gcc -Wall -o terminal terminal.c ~/Netwerk_ontwerp_PI/hva_libraries/rpitouch/*.c -I/home/julian/Netwerk_ontwerp_PI/hva_libraries/rpitouch ~/Netwerk_ontwerp_PI/julian_libraries/*.c -I/home/julian/Netwerk_ontwerp_PI/julian_libraries -lncurses -lwiringPi

#include <ncurses.h>
#include <rpitouch.h>
#include <unistd.h>
#include <mesh_radio.h>

#define REAL_POSITION(max, actual) (max - actual) 

// Table column widths
#define ID_WIDTH 4
#define HOPS_WIDTH 5
#define WEIGHT_WIDTH 7
#define TRUSTED_WIDTH 7

void draw_table();

int main(int nArgc, char* aArgv[]) {

    int nRet;

    // Start to search for the correct event-stream
     nRet = RPiTouch_InitTouch();
    if (nRet < 0) {
        printf("RaspberryPi 7\" Touch display is not found!\nError %d\n\n", nRet);
        return -1;
    }

    radioInit(0x40);

    // Init ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    // Hide the cursor
    curs_set(0);

    int maxRows, maxCols;
    getmaxyx(stdscr, maxRows, maxCols); // Get terminal size
   
    WINDOW *shutdownWindow = newwin(3, 16, 0, maxCols - 16);


    while(1){

        // Update touch
        RPiTouch_UpdateTouch();

        int realColPos = REAL_POSITION(maxCols, _oRPiTouch_Touched.nCol);
        int realRowPos = REAL_POSITION(maxRows, _oRPiTouch_Touched.nRow);

        start_color();
        init_pair(4, COLOR_WHITE, COLOR_RED);   // Pair 1: Red text, black background
        wattron(shutdownWindow, COLOR_PAIR(4));                  // Activate color pair 1
        mvwprintw(shutdownWindow, 0, 0, "                                                ");
        mvwprintw(shutdownWindow, 1, 4, "Shutdown");
        wattroff(shutdownWindow, COLOR_PAIR(4));                 // Deactivate color pair
        wrefresh(shutdownWindow);

        move(5,5);
        printw("%d, %d, %d", realColPos, realRowPos, _oRPiTouch_Touched.bButton);

        // Define color pairs
        init_pair(1, COLOR_WHITE, COLOR_BLACK);   // Header color
        init_pair(2, COLOR_BLACK, COLOR_WHITE);   // Row color 1
        init_pair(3, COLOR_BLACK, COLOR_CYAN);   // Row color 1

        // Draw the table
        draw_table(maxRows);

        refresh();

        if(realColPos <= maxCols && realColPos >= maxCols - 16 
        && realRowPos <= maxRows - 2) RPiTouch_ApplyShutdown();
    }

    // Close the device
    nRet = RPiTouch_CloseTouch();
    if (nRet < 0) {
        printw("Close error %d!\n", nRet);
    }

    // Close ncurses
    endwin();

    return 0;
}

void draw_table(int maxRows) {
    int start_y = 1, start_x = 1; // Start position for the table
    int row;

    // Define headers and data
    const char *headers[] = {"ID", "Hops", "Weight", "Trusted"};
    const char *data[][4] = {
        {"1", "1", "50", "1"},
        {"2", "3", "0", "1"},
        {"3", "2", "0", "1"},
        {"4", "1", "15", "0"}
    };

    int rows = sizeof(data) / sizeof(data[0]);

    // Draw headers
    attron(COLOR_PAIR(1)); // Use header color
    mvprintw(start_y, start_x, "Neighbord table:");
    mvprintw(start_y + 2, start_x, "%-*s%-*s%-*s%-*s",
             ID_WIDTH, headers[0],
             HOPS_WIDTH, headers[1],
             WEIGHT_WIDTH, headers[2],
             TRUSTED_WIDTH, headers[3]);
    attroff(COLOR_PAIR(1));

    // Draw rows with alternating colors
    for (row = 0; row < rows; row++) {
        int color_pair = (row % 2 == 0) ? 2 : 3; // Alternate colors
        attron(COLOR_PAIR(color_pair));
        mvprintw(start_y + 3 + row, start_x, "%-*s%-*s%-*s%-*s",
                 ID_WIDTH, data[row][0],
                 HOPS_WIDTH, data[row][1],
                 WEIGHT_WIDTH, data[row][2],
                 TRUSTED_WIDTH, atoi(data[row][3]) == 1 ? "yes" : "no");
        attroff(COLOR_PAIR(color_pair));
    }

    mvvline(0, ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 2, '|', maxRows);
}