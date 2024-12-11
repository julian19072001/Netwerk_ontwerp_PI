// gcc -Wall -o terminal terminal.c ~/Netwerk_ontwerp_PI/hva_libraries/rpitouch/*.c -I/home/julian/Netwerk_ontwerp_PI/hva_libraries/rpitouch ~/Netwerk_ontwerp_PI/julian_libraries/*.c -I/home/julian/Netwerk_ontwerp_PI/julian_libraries -lncurses -lwiringPi

#include <ncurses.h>
#include <rpitouch.h>
#include <unistd.h>
#include <mesh_radio.h>

#define REAL_POSITION(max, actual) (max - actual) 

void initColors(){
    // Define color pairs
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);     // Default color
    init_pair(2, COLOR_BLACK, COLOR_WHITE);     // 
    init_pair(3, COLOR_BLACK, COLOR_CYAN);      // 
    init_pair(4, COLOR_WHITE, COLOR_RED);       // 
    init_pair(5, COLOR_RED, COLOR_BLACK);       // 
    init_pair(6, COLOR_WHITE, COLOR_BLACK);       // 
}

int main(int nArgc, char* aArgv[]) {

    int nRet;

    // Start to search for the correct event-stream
     nRet = RPiTouch_InitTouch();
    if (nRet < 0) {
        printf("RaspberryPi 7\" Touch display is not found!\nError %d\n\n", nRet);
        return -1;
    }

    radioInit(BASE_ADDRESS);

    // Init ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    // Hide the cursor
    curs_set(0);

    initColors();

    uint16_t cycles = 0;
    int maxRows, maxCols;
    getmaxyx(stdscr, maxRows, maxCols); // Get terminal size
   
    WINDOW *shutdownWindow = newwin(3, 16, 0, maxCols - 16);
    WINDOW *neighbordTable = newwin(maxRows, ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 1, 0, 0);
    WINDOW *boardcastTable = newwin(12, 64, 4, ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 3);

    while(1){

        // Update touch
        RPiTouch_UpdateTouch();

        int realColPos = REAL_POSITION(maxCols, _oRPiTouch_Touched.nCol);
        int realRowPos = REAL_POSITION(maxRows, _oRPiTouch_Touched.nRow);
        int neighbordTableWidth = ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 1; 

        wattron(shutdownWindow, COLOR_PAIR(4));                  // Activate color pair 1
        mvwprintw(shutdownWindow, 0, 0, "                                                ");
        mvwprintw(shutdownWindow, 1, 4, "Shutdown");
        wattroff(shutdownWindow, COLOR_PAIR(4));                 // Deactivate color pair
        wrefresh(shutdownWindow);

        mvvline(0, neighbordTableWidth + 1, '|', maxRows);
        
        if(cycles > 1000){
            // Draw the table
            printNeighbors(maxRows, neighbordTable);
            printBroadcasts(boardcastTable);
            cycles = 0;
        }

        refresh();

        if(realColPos <= maxCols && realColPos >= maxCols - 16 
        && realRowPos <= maxRows - 2) RPiTouch_ApplyShutdown();
        
        cycles++;
        usleep(1000);
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