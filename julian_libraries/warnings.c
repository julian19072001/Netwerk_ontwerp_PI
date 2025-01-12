#include <warnings.h>
#include <string.h>
#include <ncurses.h>

void plantBasedWarnings(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings){
    //1 and 2 are critical warnings, 3 and 4 are mild warnings
    for(int i = 0; i < 3; i++){
        float temprature = rooms[plants[i].roomNumber].temprature;
        float humidity = rooms[plants[i].roomNumber].humidity;
        float lightLevel = rooms[plants[i].roomNumber].lightLevel;
        float groundWater = plants[i].groundWater;

        plant_t currentPlantSetting = plantSettings[plants[i].typePlant];

        warnings_t currentPlantWarnings = plants[i].warnings;

        // Check if there should be a temperature warning
        if(temprature){
            if(temprature < currentPlantSetting.minimumTemperature){
                currentPlantWarnings.tempWarning = 1;
            } else if (temprature > currentPlantSetting.maximumTemperature){
                currentPlantWarnings.tempWarning = 2;
            } else {
                currentPlantWarnings.tempWarning = 0;
            }
        } else {
            currentPlantWarnings.tempWarning = 0;
        }

        // Check if there should be a humidity is warning
        if (humidity){
            if(humidity < currentPlantSetting.minimumHumidity){
                currentPlantWarnings.humWarning = 1;
            } else if (humidity > currentPlantSetting.maximumHumidity){
                currentPlantWarnings.humWarning = 2;
            }  else {
                currentPlantWarnings.humWarning = 0;
            }
        } else {
            currentPlantWarnings.humWarning = 0;
        }

        // Check if there should be a light level warning
        if(lightLevel){
            if(lightLevel < currentPlantSetting.minimumLightneeds){
                currentPlantWarnings.LightWarning = 1;
            } else if (lightLevel > currentPlantSetting.maximumLightneeds){
                currentPlantWarnings.LightWarning = 2;
            }  else {
                currentPlantWarnings.LightWarning = 0;
            }
        } else {
            currentPlantWarnings.LightWarning = 0;
        }

        // Check if there should be a ground water warning
        if(groundWater){
            if(groundWater < currentPlantSetting.minimumWaterheight){
                currentPlantWarnings.waterWarning = 1;
            } else if (groundWater > currentPlantSetting.maximumWaterheight){
                currentPlantWarnings.waterWarning = 2;
            } else {
                currentPlantWarnings.waterWarning = 0;
            }
        } else {
            currentPlantWarnings.waterWarning = 0;
        }

        // Update the warningtype bit for temperature
        if (currentPlantWarnings.tempWarning > 0) {
           currentPlantWarnings.warningtype |= TEMP;  // Set the TEMP bit if it wasn't already set
        } else currentPlantWarnings.warningtype &= ~TEMP; // Reset the TEMP bit if it was previously set


        // Update the warningtype bit for humidity
        if (currentPlantWarnings.humWarning > 0) {
            currentPlantWarnings.warningtype |= HUM;  // Set the HUM bit if it wasn't already set
        } else currentPlantWarnings.warningtype &= ~HUM; // Reset the HUM bit if it was previously set


        // Update the warningtype bit for light
        if (currentPlantWarnings.LightWarning > 0) {
            currentPlantWarnings.warningtype |= LIGHT;  // Set the LIGHT bit if it wasn't already set
        } else currentPlantWarnings.warningtype &= ~LIGHT; // Reset the LIGHT bit if it was previously set


        // Update the warningtype bit for water
        if (currentPlantWarnings.waterWarning > 0) {
            currentPlantWarnings.warningtype |= WATER;  // Set the WATER bit if it wasn't already set
        } else currentPlantWarnings.warningtype &= ~WATER; // Reset the WATER bit if it was previously set

        plantSettings[plants[i].typePlant] = currentPlantSetting;

        plants[i].warnings = currentPlantWarnings;
    }
}

int compareRoomSuitability(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings, int plant, int newroom) {
    // Compare rooms based on the active warnings (e.g., temperature, humidity, etc.)
    uint16_t bestRoomScore = 0;  // A score that helps decide the best room
    int16_t score = 0;

    warnings_t plantWarnings = plants[plant].warnings;
    plant_t plantSetting = plantSettings[plants[plant].typePlant];

    // We will assign a score based on the factors
    if (plantWarnings.Plantwarning & TEMP && rooms[newroom].temprature) {
        score = plantSetting.optimalTemperature - rooms[newroom].temprature;

        if(score < 0) score = -score;

        bestRoomScore+= score;
    }
    if (plantWarnings.Plantwarning & HUM && rooms[newroom].humidity) {
        score = plantSetting.optimalHumidity - rooms[newroom].humidity;

        if(score < 0) score = -score;

        bestRoomScore+= score;
    }
    if (plantWarnings.Plantwarning & LIGHT && rooms[newroom].lightLevel) {
        score = plantSetting.optimalLightneeds - rooms[newroom].lightLevel;

        if(score < 0) score = -score;

        bestRoomScore+= score;
    }

    return bestRoomScore;
}

uint8_t findLowestValueIndex(int16_t array[]) {
    int lowestIndex = 0;  // Start by assuming the first element has the lowest value
    
    for (uint8_t i = 0; i < 3; i++) {
        if (array[i] < array[lowestIndex]) {
            lowestIndex = i;  // Update the index of the lowest value
        }
    }
    
    return lowestIndex;
}

void warningloop(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings, uint16_t room, int plantNumber){
    warnings_t functionWarnings = plants[plantNumber].warnings;
    
    for (int plantindex = 0; plantindex < 3; plantindex++)
    {
        if (plantNumber == plantindex || plants[plantindex].roomNumber != room) continue;
        rooms[room].conditionFlag = 0;

        warnings_t loopWarnings = plants[plantindex].warnings;

        if (((loopWarnings.tempWarning == 1)) &&
            ((functionWarnings.tempWarning == 1))) {
                    
            // Update the warningtype bit for temperature
            rooms[room].solution &= ~TOHOT;
            rooms[room].solution |= TOCOLD;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set

        } else if (((loopWarnings.tempWarning == 2)) &&
                    ((functionWarnings.tempWarning == 2))) {
            
            rooms[room].solution &= ~TOCOLD;
            rooms[room].solution |= TOHOT;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set

        } else {
            rooms[room].solution &= ~TOCOLD;
            rooms[room].solution &= ~TOHOT;
        }

        if (((loopWarnings.humWarning == 1)) && ((functionWarnings.humWarning == 1))) {
            // Update the warningtype bit for Hum
            rooms[room].solution &= ~TOWET;
            rooms[room].solution |= TODRY;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set

        } else if (((loopWarnings.humWarning == 2)) && ((functionWarnings.humWarning == 2))) {
            // Update the warningtype bit for Hum
            rooms[room].solution &= ~TODRY;
            rooms[room].solution |= TOWET;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
        } else {
            rooms[room].solution &= ~TODRY;
            rooms[room].solution &= ~TOWET;
        }

        if (((loopWarnings.LightWarning == 1)) && ((functionWarnings.LightWarning == 1))) {
            rooms[room].solution &= ~TOLIGHT;
            rooms[room].solution |= TODARK;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set

        } else if (((loopWarnings.LightWarning == 2)) && ((functionWarnings.LightWarning == 2))) {
            rooms[room].solution &= ~TODARK;
            rooms[room].solution |= TOLIGHT;  // Set the TEMP bit if it wasn't already set
            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set

        } else {
            rooms[room].solution &= ~TOLIGHT;
            rooms[room].solution &= ~TODARK;
        }

        // Check if the climate in the room is too dry
        if (((functionWarnings.tempWarning == 1) && (loopWarnings.waterWarning == 1))) {
                rooms[room].conditionwarnings |= DRYING;
                rooms[room].conditionFlag |= DIFFTYPE;  
        } else rooms[room].conditionwarnings &= ~DRYING;

        // Check if the climate in the room is too wet
        if (((loopWarnings.waterWarning == 2) && (functionWarnings.humWarning == 2))) {
                rooms[room].conditionwarnings |= OVERWATERING;
                rooms[room].conditionFlag |= DIFFTYPE; 
        } else rooms[room].conditionwarnings &= ~OVERWATERING;

        // Check if the plants are freezing to death
        if (((loopWarnings.tempWarning == 1)) && ((functionWarnings.waterWarning == 2))) {
                rooms[room].conditionwarnings |= FREEZING;
                rooms[room].conditionFlag |= DIFFTYPE;  
        } else rooms[room].conditionwarnings &= ~FREEZING;

        // Check if the plants are in too much sunlight
        if (((loopWarnings.tempWarning == 2) && (functionWarnings.LightWarning == 2)) ||
            ((functionWarnings.tempWarning == 2) && (loopWarnings.LightWarning == 2))) {
                rooms[room].conditionwarnings |= WARMING;
                rooms[room].conditionFlag |= DIFFTYPE;  
        } else rooms[room].conditionwarnings &= ~WARMING;

        // Check if a window should be opened
        if (((loopWarnings.humWarning == 2)) && ((functionWarnings.LightWarning == 2))) {
                rooms[room].conditionwarnings |= ROOM_WINDOW;
                rooms[room].conditionFlag |= DIFFTYPE; 
        } else rooms[room].conditionwarnings &= ~ROOM_WINDOW;

        plants[plantindex].warnings = loopWarnings;
    }
    plants[plantNumber].warnings = functionWarnings;
}

// Set all room based warnings
void roomBasedWarnings(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings){
    int16_t roomvalues[3];

    for (int i = 0; i < 3; i++) {
        roomvalues[i] = 100;
    }
    
    // Check if similair errors occur within the same room more then once
    for (int plant = 0; plant < 3; plant++)
    {   
        warnings_t currentWarnings = plants[plant].warnings;

        warningloop(plants, rooms, plantSettings, plants[plant].roomNumber, plant);

        currentWarnings.Plantwarning = 0;
        if(currentWarnings.tempWarning > 0 && rooms[plants[plant].roomNumber].conditionFlag == 0)
            currentWarnings.Plantwarning |= TEMP;
        
        if(currentWarnings.humWarning > 0 && rooms[plants[plant].roomNumber].conditionFlag == 0) 
            currentWarnings.Plantwarning |= HUM;
        
        if(currentWarnings.LightWarning > 0 && rooms[plants[plant].roomNumber].conditionFlag == 0)
            currentWarnings.Plantwarning |= LIGHT;

        for(int newroom = 0; newroom < 3; newroom++) {
            roomvalues[newroom] = compareRoomSuitability(plants, rooms, plantSettings, plant, newroom);
        }

        currentWarnings.newroom = findLowestValueIndex(roomvalues);

        plants[plant].warnings = currentWarnings;
    }
}

// Function to check if the found value is too high or too low and print that in the picture
void higherOrLower(uint8_t warningtype, uint8_t line, uint8_t placement){
    if(warningtype == 1){
        mvprintw(line + 6, 22 + placement, "too low");
    } else if (warningtype == 2){
        mvprintw(line + 6, 22 + placement, "too high");
    }
}

// Print all the warnings on screen
void returnWarnings (plantSettings_t *plants, roomSettings_t *rooms){
    uint8_t numberOfWarningLines = 0;
    uint8_t numberOfWarningLinesRoom = 0;

    for (int plantIndex = 0; plantIndex < 3; plantIndex++)
    {   
        warnings_t currentWarnings = plants[plantIndex].warnings;
        
        if(!currentWarnings.warningtype) continue;
         
        mvprintw(numberOfWarningLines + 6, 22, "Plant warnings for %d:       ", plantIndex + 1);

        numberOfWarningLines++;

        if (currentWarnings.warningtype & TEMP) {
            if(currentWarnings.tempWarning < 3){ 
                mvprintw(numberOfWarningLines + 6, 22, "Temperature is:                        ");
                higherOrLower(currentWarnings.tempWarning, numberOfWarningLines, strlen("Temperature is: "));
                numberOfWarningLines++;
            }
        }
        if (currentWarnings.warningtype & HUM) {
            if(currentWarnings.humWarning < 3){
                mvprintw(numberOfWarningLines + 6, 22, "Humidity is:                           ");
                higherOrLower(currentWarnings.humWarning, numberOfWarningLines, strlen("Humidity is: "));
                numberOfWarningLines++;
            }
        }
        if (currentWarnings.warningtype & LIGHT) {
            if(currentWarnings.LightWarning < 3){
                mvprintw(numberOfWarningLines + 6, 22, "Light intensity is:                     ");
                higherOrLower(currentWarnings.LightWarning, numberOfWarningLines, strlen("Light intensity is: "));
                numberOfWarningLines++;
            }
        }
        if (currentWarnings.warningtype & WATER) {
            if(currentWarnings.waterWarning < 3){
                mvprintw(numberOfWarningLines + 6, 22, "Water level is:                        ");
                higherOrLower(currentWarnings.waterWarning, numberOfWarningLines, strlen("Water level is: "));
                numberOfWarningLines++;
            }
        }
        if(currentWarnings.newroom != plants[plantIndex].roomNumber){
            mvprintw(numberOfWarningLines + 6, 22,     "Move plant to room %d                  ", currentWarnings.newroom + 1);
        } else { 
            mvprintw(numberOfWarningLines + 6, 22,     "fix at source                          ");
        }
        numberOfWarningLines += 2;
    }

    for (int room = 0; room < 3;room++)
    {   
        if(!rooms[room].solution && !rooms[room].conditionwarnings) continue;
        mvprintw(numberOfWarningLinesRoom + 6, 65, "Room warnings for %d:", room + 1);
        numberOfWarningLinesRoom++;
        if (rooms[room].conditionFlag & SAMETYPE)
        {
            // Check if each warning is active and print the corresponding message
            if (rooms[room].solution & TOHOT) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Room to hot                        ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOCOLD) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Room to cold                       ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOWET) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Humidity in the room is to high    ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TODRY) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Humidity in the room is to low     ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TODARK) { 
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Room has too little light          ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOLIGHT) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Room has too much light            ");
                numberOfWarningLinesRoom++;
            }
        }
        if (rooms[room].conditionFlag & DIFFTYPE)
        {
            // Check if each warning is active and print the corresponding message
            if (rooms[room].conditionwarnings & DRYING) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Plants in the room are drying out  ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & OVERWATERING) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Plants in the room are drowning    ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & FREEZING) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Plants in the room are freezing    ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & WARMING) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Plants in the room are too hot     ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & ROOM_WINDOW) {
                mvprintw(numberOfWarningLinesRoom + 6, 65, "Close the windows in the room      ");
                numberOfWarningLinesRoom++;
            }
        }
        numberOfWarningLinesRoom++;
    }
    
    
}
