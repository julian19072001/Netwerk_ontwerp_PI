// gcc -Wall -o terminal terminal.c ~/Netwerk_ontwerp_PI/hva_libraries/rpitouch/*.c -I/home/julian/Netwerk_ontwerp_PI/hva_libraries/rpitouch ~/Netwerk_ontwerp_PI/julian_libraries/*.c -I/home/julian/Netwerk_ontwerp_PI/julian_libraries -lncurses -lwiringPi

#include <ncurses.h>
#include <rpitouch.h>
#include <unistd.h>
#include <math.h>
#include <mesh_radio.h>
#include <address.h>

#define REAL_POSITION(max, actual) (max - actual) 

typedef enum windowTypes{
    mainWindow,
    identification,
    debug,
    plantSettings,
    roomSettings
}windowType;

typedef struct roomSettings{
    uint8_t tempratureSensor;
    uint8_t humiditySensor;
    uint8_t lightSensor;

    float temprature;
    float humidity;
    float lightLevel;
}roomSettings_t;

void initColors(){
    // Define color pairs
    start_color();

    init_color(8, 0, 500, 200);

    init_pair(1, COLOR_WHITE, COLOR_BLACK);     // Default color
    init_pair(2, COLOR_BLACK, COLOR_WHITE);      
    init_pair(3, COLOR_BLACK, COLOR_CYAN);       
    init_pair(4, COLOR_WHITE, COLOR_RED);       
    init_pair(5, COLOR_RED, COLOR_BLACK);       
    init_pair(6, COLOR_WHITE, COLOR_BLACK);        
    init_pair(7, COLOR_WHITE, COLOR_BLUE);
    init_pair(8, COLOR_BLACK, COLOR_BLACK);     // Clear screen
    init_pair(9, COLOR_BLACK, COLOR_RED);
    init_pair(10, COLOR_BLACK, COLOR_YELLOW);
}

// Function to check if a position is within the window
bool isWithinWindow(WINDOW *win, int rowPos, int colPos) {
    int max_y, max_x, start_y, start_x;

    // Get the dimensions of the window
    getmaxyx(win, max_y, max_x);

    // Get the starting position of the window
    getbegyx(win, start_y, start_x);

    // Check if the position is within the window's bounds
    if (rowPos >= start_y && rowPos < start_y + max_y &&
        colPos >= start_x && colPos < start_x + max_x) {
        return true;
    }

    return false;
}

void drawButton(WINDOW *win, int colorPair, int textX, int textY, char* text){
    wbkgd(win, COLOR_PAIR(colorPair));
    touchwin(win);
    wrefresh(win);
    mvwprintw(win, textX, textY, text);
    wrefresh(win);
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

    windowType windowShown = mainWindow;

    uint16_t cycles = 0;
    int maxRows, maxCols;
    getmaxyx(stdscr, maxRows, maxCols); // Get terminal size

    const char *header;
    int text_length;
    int start_col;
    
    // Windows used in multiple screens
    WINDOW *shutdownWindow  = newwin(3, 16, 0, maxCols - 16);
    WINDOW *backButton      = newwin(3, 7, 0, 0);
    WINDOW *line            = newwin(1, maxCols, 3, 0);
    WINDOW *line2           = newwin(1, maxCols - 18, 3, 18);

    // Windows fo main window
    WINDOW *identButton     = newwin(3, 16, 0, maxCols - 33);
    WINDOW *debugButton     = newwin(3, 16, 0, maxCols - 50);
    WINDOW *plantInfo[3];
    WINDOW *roomInfo[3];

    int mainPostions[6] = {
        3,
        7,
        11,
        15,
        20,
        25
    };

    for (int i = 0; i < 3; i++) {
        plantInfo[i] = newwin((maxRows - 4) / 6, 18, mainPostions[i], 0);
        roomInfo[i] = newwin((maxRows - 4) / 6 + 1, 18, mainPostions[i + 3], 0);
    }
    
    // Windows for debug window
    WINDOW *neighbordTable  = newwin(maxRows, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 1, 4, 0);
    WINDOW *boardcastTable  = newwin(12, 64, 5, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 3);
    WINDOW *dataTable       = newwin(12, 64, 18, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 3);

    // Windows for identification buttons
    WINDOW *identWindows[16];
    int quarterCols = maxCols / 4;
    int quarterRows = (maxRows - 4) / 4;

    // Dimensions and starting positions for windows
    int identPositions[16][2] = {
        {4, 0}, {4, quarterCols}, {4, quarterCols * 2}, {4, quarterCols * 3},
        {4 + quarterRows, 0}, {4 + quarterRows, quarterCols}, {4 + quarterRows, quarterCols * 2}, {4 + quarterRows, quarterCols * 3},  
        {4 + (quarterRows * 2), 0}, {4 + (quarterRows * 2), quarterCols}, {4 + (quarterRows * 2), quarterCols * 2}, {4 + (quarterRows * 2), quarterCols * 3},
        {4 + (quarterRows * 3), 0}, {4 + (quarterRows * 3), quarterCols}, {4 + (quarterRows * 3), quarterCols * 2}, {4 + (quarterRows * 3), quarterCols * 3},
    };

    roomSettings_t rooms[3];

    rooms[0].tempratureSensor = TEMP_HUMID_START_ADDRESS;
    rooms[0].humiditySensor = TEMP_HUMID_START_ADDRESS;
    rooms[0].lightSensor = LIGHT_START_ADDRESS;
    rooms[0].temprature = 0.2;
    rooms[0].humidity = 0.2;
    rooms[0].lightLevel = 0.2;

    rooms[1].tempratureSensor = TEMP_HUMID_START_ADDRESS + 1;
    rooms[1].humiditySensor = TEMP_HUMID_START_ADDRESS + 1;
    rooms[1].lightSensor = LIGHT_START_ADDRESS + 1;
    rooms[1].temprature = 0.2;
    rooms[1].humidity = 0.2;
    rooms[1].lightLevel = 0.2;

    rooms[2].tempratureSensor = TEMP_HUMID_START_ADDRESS + 2;
    rooms[2].humiditySensor = TEMP_HUMID_START_ADDRESS + 2;
    rooms[2].lightSensor = LIGHT_START_ADDRESS + 2;
    rooms[2].temprature = 0.2;
    rooms[2].humidity = 0.2;
    rooms[2].lightLevel = 0.2;

    uint8_t roomShown = 0;

    for (int i = 0; i < 16; i++) {
        identWindows[i] = newwin(quarterRows, quarterCols, identPositions[i][0], identPositions[i][1]);
    }

    while(1){

        // Update touch
        RPiTouch_UpdateTouch();

        int realColPos;
        int realRowPos;

        if(_oRPiTouch_Touched.bButton){
            realColPos = REAL_POSITION(maxCols, _oRPiTouch_Touched.nCol);
            realRowPos = REAL_POSITION(maxRows, _oRPiTouch_Touched.nRow);
        } else{
            realColPos = -1;
            realRowPos = -1;
        }
        int neighbordTableWidth = ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 1; 

        // Draw things that always need to be displayed
        // Draw line underneath navigation buttons
        if(windowShown != mainWindow){
            wbkgd(line, COLOR_PAIR(2));
            touchwin(line);
            wrefresh(line);
        } else {
            wbkgd(line2, COLOR_PAIR(2));
            touchwin(line2);
            wrefresh(line2);
        }
        // Draw shutdown button
        drawButton(shutdownWindow, 4, 1, 4, "Shutdown");
        
        // Draw window specific things and handle their buttons
        switch(windowShown){
            case mainWindow:
                // Draw identification button
                drawButton(identButton, 4, 1, 4, "Identify");

                // Draw Debug button
                drawButton(debugButton, 4, 1, 6, "Debug");

                // Draw room info
                drawButton(plantInfo[0], 9, 1, 5, "Plant 1:");
                mvwprintw(plantInfo[0], 2, 1, "Water level: %02d%%", 3);
                wrefresh(plantInfo[0]);
                drawButton(plantInfo[1], 10, 1, 5, "Plant 2:");
                mvwprintw(plantInfo[1], 2, 1, "Water level: %02d%%", 3);
                wrefresh(plantInfo[1]);
                drawButton(plantInfo[2], 9, 1, 5, "Plant 3:");
                mvwprintw(plantInfo[2], 2, 1, "Water level: %02d%%", 3);
                wrefresh(plantInfo[2]);

                drawButton(roomInfo[0], 10, 1, 6, "Room 1:");
                mvwprintw(roomInfo[0], 2, 1, "Temprature: %2.0f C", rooms[0].temprature);
                mvwprintw(roomInfo[0], 3, 3, "Humidity: %2.0f%%", rooms[0].humidity);
                mvwprintw(roomInfo[0], 4, 1, "Light level:%3.0fWm", rooms[0].lightLevel);
                wrefresh(roomInfo[0]);
                drawButton(roomInfo[1], 9, 1, 6, "Room 2:");
                mvwprintw(roomInfo[1], 2, 1, "Temprature: %2.0f C", rooms[1].temprature);
                mvwprintw(roomInfo[1], 3, 3, "Humidity: %2.0f%%", rooms[1].humidity);
                mvwprintw(roomInfo[1], 4, 1, "Light level:%3.0fWm", rooms[1].lightLevel);
                wrefresh(roomInfo[1]);
                drawButton(roomInfo[2], 10, 1, 6, "Room 3:");
                mvwprintw(roomInfo[2], 2, 1, "Temprature: %2.0f C", rooms[2].temprature);
                mvwprintw(roomInfo[2], 3, 3, "Humidity: %2.0f%%", rooms[2].humidity);
                mvwprintw(roomInfo[2], 4, 1, "Light level:%3.0fWm", rooms[2].lightLevel);
                wrefresh(roomInfo[2]);

                // Handel buttons
                if(isWithinWindow(identButton, realRowPos, realColPos)){ 
                    windowShown = identification;
                    clear();
                }
                
                if(isWithinWindow(debugButton, realRowPos, realColPos)){ 
                    windowShown = debug;
                    clear();
                }

                if(isWithinWindow(roomInfo[0], realRowPos, realColPos)){
                    windowShown = roomSettings;
                    roomShown = 0;
                    clear();
                }
                if(isWithinWindow(roomInfo[1], realRowPos, realColPos)){
                    windowShown = roomSettings;
                    roomShown = 1;
                    clear();
                }
                if(isWithinWindow(roomInfo[2], realRowPos, realColPos)){
                    windowShown = roomSettings;
                    roomShown = 2;
                    clear();
                }
                break;

            case debug:
                // The text to center
                header = "Debug window";
                text_length = strlen(header);
                // Calculate the starting column to center the text
                start_col = (maxCols - text_length) / 2;
                // Print the text in the middle of the screen (middle row)
                mvprintw(1, start_col, "%s", header);


                // Draw information on screen
                mvvline(4, neighbordTableWidth + 1, '|', maxRows - 4);
                
                if(cycles > 300){
                    // Draw the table
                    printNeighbors(maxRows, neighbordTable);
                    printBroadcasts(boardcastTable);
                    printDataMessages(dataTable);
                    cycles = 0;
                }

                // Draw back button
                drawButton(backButton, 4, 1, 3, "<");

                // Handel buttons
                if(isWithinWindow(backButton, realRowPos, realColPos)){ 
                    windowShown = mainWindow;
                    clear();
                }
                break;

            case identification:
                // The text to center
                header = "Identification window";
                text_length = strlen(header);
                // Calculate the starting column to center the text
                start_col = (maxCols - text_length) / 2;
                // Print the text in the middle of the screen (middle row)
                mvprintw(1, start_col, "%s", header);

                if(cycles > 300){
                    uint8_t ids[MAX_SENDERS] = {0};
                    uint8_t numberFound = getOwnIds(ids);

                    if(numberFound){
                        int temp = 0;
                        for(int i = 0; i < 16; i++){
                            if(i % 4 == 0) temp++;
                            int color_pair = ((temp + i) % 2 == 0) ? 3 : 7; // Alternate colors

                            // Erase old button
                            werase(identWindows[i]);
                            wrefresh(identWindows[i]);

                            // Draw ident button
                            if(ids[i]){
                                wbkgd(identWindows[i], COLOR_PAIR(color_pair));
                                touchwin(identWindows[i]);
                                wrefresh(identWindows[i]);
                                mvwprintw(identWindows[i], quarterRows / 2, quarterCols / 2 - 2, "0X%02X", ids[i]);
                                wrefresh(identWindows[i]);
                            } else {
                                wbkgd(identWindows[i], COLOR_PAIR(8));
                                touchwin(identWindows[i]);
                                wrefresh(identWindows[i]);
                            }
                        }
                    }

                    for(int i = 0; i < 16; i++){
                        uint8_t data[2] = {BASE_ADDRESS, DATA_IDENTFY};
                        if(isWithinWindow(identWindows[i], realRowPos, realColPos)) sendRadioData(ids[i], data, 2, true);
                    }
                    cycles = 0;
                }

                // Draw back button
                drawButton(backButton, 4, 1, 3, "<");

                // Handel buttons
                if(isWithinWindow(backButton, realRowPos, realColPos)){ 
                    windowShown = mainWindow;
                    clear();
                }
                break;
            
            case roomSettings:
                // The text to center
                header = "Room ! information";
                text_length = strlen(header);
                // Calculate the starting column to center the text
                start_col = (maxCols - text_length) / 2;
                // Print the text in the middle of the screen (middle row)
                mvprintw(1, start_col, "Room %d information", roomShown + 1);



                // Draw back button
                drawButton(backButton, 4, 1, 3, "<");

                // Handel buttons
                if(isWithinWindow(backButton, realRowPos, realColPos)){ 
                    windowShown = mainWindow;
                    clear();
                }
        }

        // Refresh screen
        refresh();

        // Handel shutdown button
        if(isWithinWindow(shutdownWindow, realRowPos, realColPos)) RPiTouch_ApplyShutdown();
        
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