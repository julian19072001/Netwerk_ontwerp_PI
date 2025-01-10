// gcc -Wall -o terminal terminal.c ~/Netwerk_ontwerp_PI/hva_libraries/rpitouch/*.c -I/home/julian/Netwerk_ontwerp_PI/hva_libraries/rpitouch ~/Netwerk_ontwerp_PI/julian_libraries/*.c -I/home/julian/Netwerk_ontwerp_PI/julian_libraries -lncurses -lwiringPi

#include <ncurses.h>
#include <rpitouch.h>
#include <unistd.h>
#include <math.h>
#include <mesh_radio.h>
#include <address.h>
#include <data.h>
#include <time.h>
#include <warnings.h>

#define REAL_POSITION(max, actual) (max - actual) 

void setupPlantData(plant_t *plantInfoLocation){
    plantInfoLocation[0].name = "Kentia";

    plantInfoLocation[0].minimumTemperature = -4;
    plantInfoLocation[0].maximumTemperature = 38;
    plantInfoLocation[0].optimalTemperature = 13;

    plantInfoLocation[0].minimumHumidity = 30;
    plantInfoLocation[0].maximumHumidity = 70;
    plantInfoLocation[0].optimalHumidity = 50;

    plantInfoLocation[0].minimumLightneeds = 300;
    plantInfoLocation[0].maximumLightneeds = 900;
    plantInfoLocation[0].optimalLightneeds = 700;

    plantInfoLocation[0].minimumWaterheight = 30;
    plantInfoLocation[0].maximumWaterheight = 80;


    plantInfoLocation[1].name = "Chinese Money Plant";

    plantInfoLocation[1].minimumTemperature = 10;
    plantInfoLocation[1].maximumTemperature = 30;
    plantInfoLocation[1].optimalTemperature = 20;

    plantInfoLocation[1].minimumHumidity = 30;
    plantInfoLocation[1].maximumHumidity = 50;
    plantInfoLocation[1].optimalHumidity = 40;

    plantInfoLocation[1].minimumLightneeds = 200;
    plantInfoLocation[1].maximumLightneeds = 700;
    plantInfoLocation[1].optimalLightneeds = 500;

    plantInfoLocation[1].minimumWaterheight = 5;
    plantInfoLocation[1].maximumWaterheight = 90;


    plantInfoLocation[2].name = "Spider Plant";

    plantInfoLocation[2].minimumTemperature = 10;
    plantInfoLocation[2].maximumTemperature = 30;
    plantInfoLocation[2].optimalTemperature = 20;

    plantInfoLocation[2].minimumHumidity = 30;
    plantInfoLocation[2].maximumHumidity = 70;
    plantInfoLocation[1].optimalHumidity = 55;

    plantInfoLocation[2].minimumLightneeds = 100;
    plantInfoLocation[2].maximumLightneeds = 500;
    plantInfoLocation[2].optimalLightneeds = 350;

    plantInfoLocation[2].minimumWaterheight = 20;
    plantInfoLocation[2].maximumWaterheight = 80;


    plantInfoLocation[3].name = "Jade Plant";

    plantInfoLocation[3].minimumTemperature = 10;
    plantInfoLocation[3].maximumTemperature = 26;
    plantInfoLocation[3].optimalTemperature = 22;

    plantInfoLocation[3].minimumHumidity = 30;
    plantInfoLocation[3].maximumHumidity = 80;
    plantInfoLocation[3].optimalHumidity = 55;

    plantInfoLocation[3].minimumLightneeds = 300;
    plantInfoLocation[3].maximumLightneeds = 700;
    plantInfoLocation[3].optimalLightneeds = 500;

    plantInfoLocation[3].minimumWaterheight = 30;
    plantInfoLocation[3].maximumWaterheight = 70;
}

float uint8_array_to_float(uint8_t* array) {
    float value;
    // Copy the uint8_t array's bytes into the float
    memcpy(&value, array, sizeof(float));
    return value;
}

void initColors(){
    // Define color pairs
    start_color();

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
    init_pair(11, COLOR_WHITE, COLOR_GREEN);
    init_pair(12, COLOR_GREEN, COLOR_WHITE);
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

// Function to get the current timestamp in seconds
time_t get_current_time() {
    return time(NULL);
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
    WINDOW *line2           = newwin(1, maxCols - 20, 3, 20);

    // Windows fo main window
    WINDOW *identButton     = newwin(3, 16, 0, maxCols - 33);
    WINDOW *debugButton     = newwin(3, 16, 0, maxCols - 50);
    WINDOW *plantInfo[3];
    WINDOW *roomInfo[3];
    WINDOW *warningWindow   = newwin(maxRows - 4, maxCols - 20, 4, 20);

    int mainPostions[6] = {
        3,
        7,
        11,
        15,
        20,
        25
    };

    for (int i = 0; i < 3; i++) {
        plantInfo[i] = newwin((maxRows - 4) / 6, 20, mainPostions[i], 0);
        roomInfo[i] = newwin((maxRows - 4) / 6 + 1, 20, mainPostions[i + 3], 0);
    }
    
    // Windows for debug window
    WINDOW *neighbordTable  = newwin(maxRows, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 1, 4, 0);
    WINDOW *boardcastTable  = newwin(12, 64, 5, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 3);
    WINDOW *dataTable       = newwin(12, 64, 18, 5 + ID_WIDTH + HOPS_WIDTH + WEIGHT_WIDTH + TRUSTED_WIDTH + 3);

    // Windows for identification buttons
    WINDOW *identWindows[16];
    int quarterCols = maxCols / 4;
    int quarterRows = (maxRows - 4) / 4;

    int identPositions[16][2] = {
        {4, 0}, {4, quarterCols}, {4, quarterCols * 2}, {4, quarterCols * 3},
        {4 + quarterRows, 0}, {4 + quarterRows, quarterCols}, {4 + quarterRows, quarterCols * 2}, {4 + quarterRows, quarterCols * 3},  
        {4 + (quarterRows * 2), 0}, {4 + (quarterRows * 2), quarterCols}, {4 + (quarterRows * 2), quarterCols * 2}, {4 + (quarterRows * 2), quarterCols * 3},
        {4 + (quarterRows * 3), 0}, {4 + (quarterRows * 3), quarterCols}, {4 + (quarterRows * 3), quarterCols * 2}, {4 + (quarterRows * 3), quarterCols * 3},
    };

    for (int i = 0; i < 16; i++) {
        identWindows[i] = newwin(quarterRows, quarterCols, identPositions[i][0], identPositions[i][1]);
    }

    // Windows for setting screens
    WINDOW *settingButton[6];

    int settingButtonPositions[6][2] = {
        {maxRows - 13, 20} , {maxRows - 13, maxCols - 27} ,
        {maxRows - 8, 20} , {maxRows - 8, maxCols - 27} ,
        {maxRows - 3, 20} , {maxRows - 3, maxCols - 27}
    };

    for (int i = 0; i < 6; i++) {
        settingButton[i] = newwin(3, 7, settingButtonPositions[i][0], settingButtonPositions[i][1]);
    }

    // Windows for setting screens
    WINDOW *plantSelectButton[4];

    for (int i = 0; i < 4; i++) {
        plantSelectButton[i] = newwin(3, quarterCols, 5, quarterCols * i);
    }

    WINDOW *roomInformationBox = newwin(6, 30, 8, maxCols / 2 - 15);
    WINDOW *plantLimitations = newwin(12, 30, 9, 15);
    WINDOW *plantInformationBox = newwin(7, 30, 11, maxCols - 45);

    roomSettings_t rooms[3];

    rooms[0].tempratureSensor = TEMP_HUMID_START_ADDRESS;
    rooms[0].humiditySensor = TEMP_HUMID_START_ADDRESS;
    rooms[0].lightSensor = LIGHT_START_ADDRESS;
    rooms[0].temprature = 0;
    rooms[0].humidity = 0;
    rooms[0].lightLevel = 0;
    rooms[0].solution = 0;
    rooms[0].conditionwarnings = 0;

    rooms[1].tempratureSensor = TEMP_HUMID_START_ADDRESS + 1;
    rooms[1].humiditySensor = TEMP_HUMID_START_ADDRESS + 1;
    rooms[1].lightSensor = LIGHT_START_ADDRESS + 1;
    rooms[1].temprature = 0;
    rooms[1].humidity = 0;
    rooms[1].lightLevel = 0;
    rooms[1].solution = 0;
    rooms[1].conditionwarnings = 0;

    rooms[2].tempratureSensor = TEMP_HUMID_START_ADDRESS + 2;
    rooms[2].humiditySensor = TEMP_HUMID_START_ADDRESS + 2;
    rooms[2].lightSensor = LIGHT_START_ADDRESS + 2;
    rooms[2].temprature = 0;
    rooms[2].humidity = 0;
    rooms[2].lightLevel = 0;
    rooms[2].solution = 0;
    rooms[2].conditionwarnings = 0;

    uint8_t roomShown = 0;

    plantSettings_t plants[3];

    plants[0].roomNumber = 0;
    plants[0].groundSensor = GROUND_WATER_START_ADDRESS;
    plants[0].groundWater = 0;
    plants[0].typePlant = TYPE_A;
    plants[0].warnings.warningtype = 0;

    plants[1].roomNumber = 0;
    plants[1].groundSensor = GROUND_WATER_START_ADDRESS + 1;
    plants[1].groundWater = 0;
    plants[1].typePlant = TYPE_B;
    plants[1].warnings.warningtype = 0;

    plants[2].roomNumber = 0;
    plants[2].groundSensor = GROUND_WATER_START_ADDRESS + 2;
    plants[2].groundWater = 0;
    plants[2].typePlant = TYPE_C;
    plants[2].warnings.warningtype = 0;


    uint8_t plantShown = 0;

    plant_t plantInformation[4];
    setupPlantData(plantInformation);

    uint8_t ids[MAX_SENDERS] = {0};

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
        
        uint8_t radioData[MAX_DATA_LENGTH];
        int numBytes = readRadioMessage(radioData);

        // Get new sensor data
        if(numBytes){
            uint8_t sensorDataArray[] = {radioData[2], radioData[3], radioData[4], radioData[5]};
            float sensorData = uint8_array_to_float(sensorDataArray);
            for(int i = 0; i < 3; i++){
                switch(radioData[1]){
                    case DATA_TEMP:
                        if(radioData[0] == rooms[i].tempratureSensor){
                            rooms[i].temprature = sensorData;
                            rooms[i].lastTemperature = get_current_time();
                        }
                        break;
                    case DATA_HUMID:
                        if(radioData[0] == rooms[i].humiditySensor){
                            rooms[i].humidity = sensorData;
                            rooms[i].lastHumidity = get_current_time();
                        }
                        break;
                    case DATA_LIGHT:
                        if(radioData[0] == rooms[i].lightSensor){
                            rooms[i].lightLevel = sensorData;
                            rooms[i].lastLight = get_current_time();
                        }
                        break;
                    case DATA_GROUND_HUMID:
                        if(radioData[0] == plants[i].groundSensor){
                            plants[i].groundWater = sensorData;
                            plants[i].lastGround = get_current_time();
                        }
                        break;
                }
            }
        } 

        // Clear old data if no new data is received
        for(int i = 0; i < 3; i++){
            if(rooms[i].temprature > 0 && (get_current_time() - rooms[i].lastTemperature > 5)) rooms[i].temprature = 0;
            if(rooms[i].humidity > 0 && (get_current_time() - rooms[i].lastHumidity > 5)) rooms[i].humidity = 0;
            if(rooms[i].lightLevel > 0 && (get_current_time() - rooms[i].lastLight > 5)) rooms[i].lightLevel = 0;
            if(plants[i].groundWater > 0 && (get_current_time() - plants[i].lastGround > 5)) plants[i].groundWater = 0;
        }

        plantBasedWarnings(plants, rooms, plantInformation);
        roomBasedWarnings(plants, rooms, plantInformation);

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
                if(cycles > 300){
                    drawButton(warningWindow, 8, 0,0,"");
                    returnWarnings(plants, rooms);
                    cycles = 0;
                }
                mvprintw(1, 1, "Number of connected nodes: %d   ", getOwnIds(ids));

                // Draw identification button
                drawButton(identButton, 4, 1, 4, "Identify");

                // Draw Debug button
                drawButton(debugButton, 4, 1, 6, "Debug");

                // Draw room info
                drawButton(plantInfo[0], 9, 1, 6, "Plant 1:");
                mvwprintw(plantInfo[0], 2, 2, "Water level: %2.0f%%", plants[0].groundWater);
                wrefresh(plantInfo[0]);
                drawButton(plantInfo[1], 10, 1, 6, "Plant 2:");
                mvwprintw(plantInfo[1], 2, 2, "Water level: %2.0f%%", plants[1].groundWater);
                wrefresh(plantInfo[1]);
                drawButton(plantInfo[2], 9, 1, 6, "Plant 3:");
                mvwprintw(plantInfo[2], 2, 2, "Water level: %2.0f%%", plants[2].groundWater);
                wrefresh(plantInfo[2]);

                drawButton(roomInfo[0], 10, 1, 7, "Room 1:");
                mvwprintw(roomInfo[0], 2, 0, "  Temprature: %2.0f C  ", rooms[0].temprature);
                mvwprintw(roomInfo[0], 3, 0, "    Humidity: %2.0f%%   ", rooms[0].humidity);
                mvwprintw(roomInfo[0], 4, 0, " Light level:%4.0fWm ", rooms[0].lightLevel);
                wrefresh(roomInfo[0]);
                drawButton(roomInfo[1], 9, 1, 7, "Room 2:");
                mvwprintw(roomInfo[1], 2, 0, "  Temprature: %2.0f C  ", rooms[1].temprature);
                mvwprintw(roomInfo[1], 3, 0, "    Humidity: %2.0f%%   ", rooms[1].humidity);
                mvwprintw(roomInfo[1], 4, 0, " Light level:%4.0fWm ", rooms[1].lightLevel);
                wrefresh(roomInfo[1]);
                drawButton(roomInfo[2], 10, 1, 7, "Room 3:");
                mvwprintw(roomInfo[2], 2, 0, "  Temprature: %2.0f C  ", rooms[2].temprature);
                mvwprintw(roomInfo[2], 3, 0, "    Humidity: %2.0f%%   ", rooms[2].humidity);
                mvwprintw(roomInfo[2], 4, 0, " Light level:%4.0fWm ", rooms[2].lightLevel);
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

                if(isWithinWindow(plantInfo[0], realRowPos, realColPos)){
                    windowShown = plantSettings;
                    plantShown = 0;
                    clear();
                }
                if(isWithinWindow(plantInfo[1], realRowPos, realColPos)){
                    windowShown = plantSettings;
                    plantShown = 1;
                    clear();
                }
                if(isWithinWindow(plantInfo[2], realRowPos, realColPos)){
                    windowShown = plantSettings;
                    plantShown = 2;
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

                werase(settingButton[0]);
                wrefresh(settingButton[0]);
                drawButton(settingButton[0], 2, 1, 3, "-");
                werase(settingButton[1]);
                wrefresh(settingButton[1]);
                drawButton(settingButton[1], 2, 1, 3, "+");
                werase(settingButton[2]);
                wrefresh(settingButton[2]);
                drawButton(settingButton[2], 2, 1, 3, "-");
                werase(settingButton[3]);
                wrefresh(settingButton[3]);
                drawButton(settingButton[3], 2, 1, 3, "+");
                werase(settingButton[4]);
                wrefresh(settingButton[4]);
                drawButton(settingButton[4], 2, 1, 3, "-");
                werase(settingButton[5]);
                wrefresh(settingButton[5]);
                drawButton(settingButton[5], 2, 1, 3, "+");

                mvprintw(maxRows - 13, 37, "Sensor ID for temperature:");
                mvprintw(maxRows - 8, 39, "Sensor ID for Humidity:");
                mvprintw(maxRows - 3, 37, "Sensor ID for light level:");

                mvprintw(maxRows - 12, maxCols / 2 - 2, "0X%02X", rooms[roomShown].tempratureSensor);
                mvprintw(maxRows - 7, maxCols / 2 - 2, "0X%02X", rooms[roomShown].humiditySensor);
                mvprintw(maxRows - 2, maxCols / 2 - 2, "0X%02X", rooms[roomShown].lightSensor);

                // Draw back button
                drawButton(backButton, 4, 1, 3, "<");

                // Handel buttons
                if(isWithinWindow(backButton, realRowPos, realColPos)){ 
                    windowShown = mainWindow;
                    clear();
                }

                if(cycles > 200){
                    werase(roomInformationBox);
                    wrefresh(roomInformationBox);
                    wbkgd(roomInformationBox, COLOR_PAIR(2));
                    touchwin(roomInformationBox);
                    wrefresh(roomInformationBox);
                    mvwprintw(roomInformationBox, 1, 1, "Sensor data:");
                    mvwprintw(roomInformationBox, 2, 1, "Temprature: %5.2f C", rooms[roomShown].temprature);
                    mvwprintw(roomInformationBox, 3, 1, "Humidity: %5.2f %%", rooms[roomShown].humidity);
                    mvwprintw(roomInformationBox, 4, 1, "Light level: %5.2f %%", rooms[roomShown].lightLevel);
                    wrefresh(roomInformationBox);

                    if(isWithinWindow(settingButton[0], realRowPos, realColPos)) {
                        rooms[roomShown].tempratureSensor--;
                        if(rooms[roomShown].tempratureSensor < TEMP_HUMID_START_ADDRESS)
                            rooms[roomShown].tempratureSensor = TEMP_HUMID_START_ADDRESS;
                        else rooms[roomShown].temprature = 0;
                    }
                    if(isWithinWindow(settingButton[1], realRowPos, realColPos)) {
                        rooms[roomShown].tempratureSensor++;
                        if(rooms[roomShown].tempratureSensor > TEMP_HUMID_END_ADDRESS)
                            rooms[roomShown].tempratureSensor = TEMP_HUMID_END_ADDRESS;
                        else rooms[roomShown].temprature = 0;
                    }
                    if(isWithinWindow(settingButton[2], realRowPos, realColPos)) {
                        rooms[roomShown].humiditySensor--;
                        if(rooms[roomShown].humiditySensor < TEMP_HUMID_START_ADDRESS)
                            rooms[roomShown].humiditySensor = TEMP_HUMID_START_ADDRESS;
                        else rooms[roomShown].humidity = 0;
                    }
                    if(isWithinWindow(settingButton[3], realRowPos, realColPos)) {
                        rooms[roomShown].humiditySensor++;
                        if(rooms[roomShown].humiditySensor > TEMP_HUMID_END_ADDRESS)
                            rooms[roomShown].humiditySensor = TEMP_HUMID_END_ADDRESS;
                        else rooms[roomShown].humidity = 0;
                    }
                    if(isWithinWindow(settingButton[4], realRowPos, realColPos)) {
                        rooms[roomShown].lightSensor--;
                        if(rooms[roomShown].lightSensor < LIGHT_START_ADDRESS)
                            rooms[roomShown].lightSensor = LIGHT_START_ADDRESS;
                        else rooms[roomShown].lightLevel = 0;
                    }
                    if(isWithinWindow(settingButton[5], realRowPos, realColPos)) {
                        rooms[roomShown].lightSensor++;
                        if(rooms[roomShown].lightSensor > LIGHT_END_ADDRESS)
                            rooms[roomShown].lightSensor = LIGHT_END_ADDRESS;
                        else rooms[roomShown].lightLevel = 0;
                    }
                    cycles = 0;
                }
                break;
            case plantSettings:
                // The text to center
                header = "Plant ! information";
                text_length = strlen(header);
                // Calculate the starting column to center the text
                start_col = (maxCols - text_length) / 2;
                // Print the text in the middle of the screen (middle row)
                mvprintw(1, start_col, "Plant %d information", plantShown + 1);

                werase(settingButton[2]);
                wrefresh(settingButton[2]);
                drawButton(settingButton[2], 2, 1, 3, "-");
                werase(settingButton[3]);
                wrefresh(settingButton[3]);
                drawButton(settingButton[3], 2, 1, 3, "+");
                werase(settingButton[4]);
                wrefresh(settingButton[4]);
                drawButton(settingButton[4], 2, 1, 3, "-");
                werase(settingButton[5]);
                wrefresh(settingButton[5]);
                drawButton(settingButton[5], 2, 1, 3, "+");

                mvprintw(maxRows - 8, 44, "Room number:");
                mvprintw(maxRows - 3, 36, "Sensor ID for ground water:");

                mvprintw(maxRows - 7, maxCols / 2 - 2, "%d", plants[plantShown].roomNumber);
                mvprintw(maxRows - 2, maxCols / 2 - 2, "0X%02X", plants[plantShown].groundSensor);

                mvprintw(4, 0, "Plant type:");

                if(cycles > 300){
                    for(int i = 0; i < 4; i++){
                        text_length = strlen(plantInformation[i].name);
                        start_col = (quarterCols - text_length) / 2;

                        uint8_t color = 1;
                        if(plants[plantShown].typePlant == i) color = 12;
                        else color = 11;
                        // Erase old button
                        werase(plantSelectButton[i]);
                        wrefresh(plantSelectButton[i]);

                        drawButton(plantSelectButton[i], color, 1, start_col, plantInformation[i].name);

                        wbkgd(plantLimitations, COLOR_PAIR(2));
                        touchwin(plantLimitations);
                        wrefresh(plantLimitations);
                        mvwprintw(plantLimitations, 1, 1, "Minimum temperature: %3d C", plantInformation[plants[plantShown].typePlant].minimumTemperature);
                        mvwprintw(plantLimitations, 2, 1, "Maximum temperature: %3d C", plantInformation[plants[plantShown].typePlant].maximumTemperature);

                        mvwprintw(plantLimitations, 3, 1, "Minimum humidity: %3d%%", plantInformation[plants[plantShown].typePlant].minimumHumidity);
                        mvwprintw(plantLimitations, 4, 1, "Maximum humidity: %3d%%", plantInformation[plants[plantShown].typePlant].maximumHumidity);

                        mvwprintw(plantLimitations, 7, 1, "Minimum light level: %4d Wm", plantInformation[plants[plantShown].typePlant].minimumLightneeds);
                        mvwprintw(plantLimitations, 8, 1, "Maximum light level: %4d Wm", plantInformation[plants[plantShown].typePlant].maximumLightneeds);

                        mvwprintw(plantLimitations, 9, 1, "Minimum water height: %3d%%", plantInformation[plants[plantShown].typePlant].minimumWaterheight);
                        mvwprintw(plantLimitations, 10, 1, "Maximum water height: %3d%%", plantInformation[plants[plantShown].typePlant].maximumWaterheight);
                        wrefresh(plantLimitations);

                        werase(plantInformationBox);
                        wrefresh(plantInformationBox);
                        wbkgd(plantInformationBox, COLOR_PAIR(2));
                        touchwin(plantInformationBox);
                        wrefresh(plantInformationBox);
                        mvwprintw(plantInformationBox, 1, 1, "Plant data:");
                        mvwprintw(plantInformationBox, 2, 1, "Temprature: %5.2f C", rooms[plants[plantShown].roomNumber].temprature);
                        mvwprintw(plantInformationBox, 3, 1, "Humidity: %5.2f %%", rooms[plants[plantShown].roomNumber].humidity);
                        mvwprintw(plantInformationBox, 4, 1, "Light level: %5.2f %%", rooms[plants[plantShown].roomNumber].lightLevel);
                        mvwprintw(plantInformationBox, 5, 1, "Ground water height: %5.2f %%", plants[plantShown].groundWater);
                        wrefresh(plantInformationBox);
                    }

                    if(isWithinWindow(settingButton[2], realRowPos, realColPos)) {
                        plants[plantShown].roomNumber--;
                        if(plants[plantShown].roomNumber < 0)
                            plants[plantShown].roomNumber = 0;
                    }
                    if(isWithinWindow(settingButton[3], realRowPos, realColPos)) {
                        plants[plantShown].roomNumber++;
                        if(plants[plantShown].roomNumber > 2)
                            plants[plantShown].roomNumber = 2;
                    }
                    if(isWithinWindow(settingButton[4], realRowPos, realColPos)) {
                        plants[plantShown].groundSensor--;
                        if(plants[plantShown].groundSensor < GROUND_WATER_START_ADDRESS)
                            plants[plantShown].groundSensor = GROUND_WATER_START_ADDRESS;
                        else plants[plantShown].groundWater = 0;
                    }
                    if(isWithinWindow(settingButton[5], realRowPos, realColPos)) {
                        plants[plantShown].groundSensor++;
                        if(plants[plantShown].groundSensor > GROUND_WATER_END_ADDRESS)
                            plants[plantShown].groundSensor = GROUND_WATER_END_ADDRESS;
                        else plants[plantShown].groundWater = 0;
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
                if(isWithinWindow(plantSelectButton[0], realRowPos, realColPos)){ 
                    plants[plantShown].typePlant = TYPE_A;
                }
                if(isWithinWindow(plantSelectButton[1], realRowPos, realColPos)){ 
                    plants[plantShown].typePlant = TYPE_B;
                }
                if(isWithinWindow(plantSelectButton[2], realRowPos, realColPos)){ 
                    plants[plantShown].typePlant = TYPE_C;
                }
                if(isWithinWindow(plantSelectButton[3], realRowPos, realColPos)){ 
                    plants[plantShown].typePlant = TYPE_D;
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