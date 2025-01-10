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
            } else if(temprature < currentPlantSetting.minimumTemperature * 1.2){
                currentPlantWarnings.tempWarning = 3;
            } else if (temprature > currentPlantSetting.maximumTemperature * 0.8){
                currentPlantWarnings.tempWarning = 4;
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
            } else if(humidity < currentPlantSetting.minimumHumidity * 1.2){
                currentPlantWarnings.humWarning = 3;
            } else if (humidity > currentPlantSetting.maximumHumidity * 0.8){
                currentPlantWarnings.humWarning = 4;
            } else {
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
            } else if(lightLevel < currentPlantSetting.minimumLightneeds * 1.2){
                currentPlantWarnings.LightWarning = 3;
            } else if (lightLevel > currentPlantSetting.maximumLightneeds * 0.8){
                currentPlantWarnings.LightWarning = 4;
            } else {
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
            } else if(groundWater < currentPlantSetting.minimumWaterheight * 1.2){
                currentPlantWarnings.waterWarning = 3;
            } else if (groundWater > currentPlantSetting.maximumWaterheight * 0.8){
                currentPlantWarnings.waterWarning = 4;
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
    if (plantWarnings.Plantwarning & TEMP) {
        score = plantSetting.optimalTemperature - rooms[newroom].temprature;

        if(score < 0){
            score = -score;
        }

        bestRoomScore+= score;
    }
    if (plantWarnings.Plantwarning & HUM) {
        score = plantSetting.optimalHumidity - rooms[newroom].humidity;

        if(score < 0){
            score = -score;
        }

        bestRoomScore+= score;
    }
    if (plantWarnings.Plantwarning & LIGHT) {
        score = plantSetting.optimalLightneeds - rooms[newroom].lightLevel;

        if(score < 0){
            score = -score;
        }

        bestRoomScore+= score;
    }

    return bestRoomScore;
}

int findLowestValueIndex(int16_t array[], int size, int plant) {
    int lowestIndex = 0;  // Start by assuming the first element has the lowest value
    
    for (int i = 1; i < size; i++) {
        if (array[i] < array[lowestIndex]) {
            lowestIndex = i;  // Update the index of the lowest value
        }
    }
    
    return lowestIndex;
}

void warningloop(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings, uint16_t room, uint8_t warningType, int plantNumber){
    warnings_t functionWarnings = plants[plantNumber].warnings;
    
    for (int plantindex = 0; plantindex < 3; plantindex++)
    {
        if (plantNumber == plantindex || plants[plantindex].roomNumber != room) continue;
        rooms[room].conditionFlag = 0;

        warnings_t loopWarnings = plants[plantindex].warnings;

        if(loopWarnings.warningtype & warningType){
            switch (warningType) {
                case TEMP:
                    if (((loopWarnings.tempWarning == 1) || (loopWarnings.tempWarning == 3)) &&
                        ((functionWarnings.tempWarning == 1) || (functionWarnings.tempWarning == 3))) {
                                
                        // Update the warningtype bit for temperature
                        if (rooms[room].solution > 0 && !(rooms[room].solution & TOCOLD)) {
                            rooms[room].solution |= TOCOLD;  // Set the TEMP bit if it wasn't already set
                        } 

                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }

                    } else if (((loopWarnings.tempWarning == 2) || (loopWarnings.tempWarning == 4)) &&
                               ((functionWarnings.tempWarning == 2) || (functionWarnings.tempWarning == 4))) {

                        if (rooms[room].solution > 0 && !(rooms[room].solution & TOHOT)) {
                            rooms[room].solution |= TOHOT;  // Set the TEMP bit if it wasn't already set
                        }
                            
                            
                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }

                    }
                    break;

                case HUM:
                    if (((loopWarnings.humWarning == 1) || (loopWarnings.humWarning == 3)) &&
                        ((functionWarnings.humWarning == 1) || (functionWarnings.humWarning == 3))) {
                        // Update the warningtype bit for Hum
                        if (rooms[room].solution > 0 && !(rooms[room].solution & TODRY)) {
                            rooms[room].solution |= TODRY;  // Set the TEMP bit if it wasn't already set
                        }
                        
                        
                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }

                    } else if (((loopWarnings.humWarning == 2) || (loopWarnings.humWarning == 4)) &&
                                ((functionWarnings.humWarning == 2) || (functionWarnings.humWarning == 4))) {
                        // Update the warningtype bit for Hum
                        if (rooms[room].solution > 0 && !(rooms[room].solution & TOWET)) {
                            rooms[room].solution |= TOWET;  // Set the TEMP bit if it wasn't already set
                        }
                        
                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }
                    }
                    break;

                case LIGHT:
                    if (((loopWarnings.LightWarning == 1) || (loopWarnings.LightWarning == 3)) &&
                        ((functionWarnings.LightWarning == 1) || (functionWarnings.LightWarning == 3))) {
                        if (rooms[room].solution > 0 && !(rooms[room].solution & TODARK)) {
                            rooms[room].solution |= TODARK;  // Set the TEMP bit if it wasn't already set
                        }
                        
                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }

                    } else if (((loopWarnings.LightWarning == 2) || (loopWarnings.LightWarning == 4)) &&
                                ((functionWarnings.LightWarning == 2) || (functionWarnings.LightWarning == 4))) {
                        if (rooms[room].solution > 0 && !(rooms[room].solution & TOLIGHT)) {
                            rooms[room].solution |= TOLIGHT;  // Set the TEMP bit if it wasn't already set
                        }
                        
                        if(!(rooms[room].conditionFlag & SAMETYPE)) {
                            rooms[room].conditionFlag |= SAMETYPE;  // Set the TEMP bit if it wasn't already set
                        }

                    }
                    break;

                default:
                    break;
            }
        }
        
        if(!(loopWarnings.warningtype & warningType)) {
                // Check if the tempWarning and waterWarning are active for either plant
                if (((loopWarnings.tempWarning == 1) || (loopWarnings.tempWarning == 3)) && 
                    ((functionWarnings.waterWarning == 1) || (functionWarnings.waterWarning == 3))) {
                    // Apply condition for the DRYING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & DRYING)) {
                        rooms[room].conditionwarnings |= DRYING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                } 

                // Check the opposite arrangement (swapped plant index and number)
                else if (((functionWarnings.tempWarning == 1) || (functionWarnings.tempWarning == 3)) && 
                        ((loopWarnings.waterWarning == 1) || (loopWarnings.waterWarning == 3))) {
                    // Apply condition for the DRYING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & DRYING)) {
                        rooms[room].conditionwarnings |= DRYING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                }

                // Check if the waterWarning and humWarning are active for either plant
                if (((loopWarnings.waterWarning == 4) || (loopWarnings.waterWarning == 3)) && 
                    ((functionWarnings.humWarning == 4) || (functionWarnings.humWarning == 3))) {
                    // Apply condition for the OVERWATERING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & OVERWATERING)) {
                        rooms[room].conditionwarnings |= OVERWATERING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                } 

                // Check the opposite arrangement (swapped plant index and number)
                else if (((functionWarnings.waterWarning == 4) || (functionWarnings.waterWarning == 3)) && 
                        ((loopWarnings.humWarning == 4) || (loopWarnings.humWarning == 3))) {
                    // Apply condition for the OVERWATERING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & OVERWATERING)) {
                        rooms[room].conditionwarnings |= OVERWATERING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                }

                // Check for FREEZING condition
                if (((loopWarnings.tempWarning == 3) || (loopWarnings.tempWarning == 4)) && 
                    ((functionWarnings.waterWarning == 4) || (functionWarnings.waterWarning == 3))) {
                    // Apply condition for the FREEZING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & FREEZING)) {
                        rooms[room].conditionwarnings |= FREEZING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                } 

                // Check the opposite arrangement (swapped plant index and number)
                else if (((functionWarnings.tempWarning == 3) || (functionWarnings.tempWarning == 4)) && 
                        ((loopWarnings.waterWarning == 4) || (loopWarnings.waterWarning == 3))) {
                    // Apply condition for the FREEZING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & FREEZING)) {
                        
                        rooms[room].conditionwarnings |= FREEZING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                }

                // Check for WARMING condition
                if (((loopWarnings.tempWarning == 4) || (loopWarnings.tempWarning == 3)) && 
                    ((functionWarnings.LightWarning == 4) || (functionWarnings.LightWarning == 3))) {
                    // Apply condition for the WARMING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & WARMING)) {
                        rooms[room].conditionwarnings |= WARMING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                } 

                // Check the opposite arrangement (swapped plant index and number)
                else if (((functionWarnings.tempWarning == 4) || (functionWarnings.tempWarning == 3)) && 
                        ((loopWarnings.LightWarning == 4) || (loopWarnings.LightWarning == 3))) {
                    // Apply condition for the WARMING warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & WARMING)) {
                        rooms[room].conditionwarnings |= WARMING;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                }

                // Check for WINDOW condition
                if (((loopWarnings.humWarning == 4) || (loopWarnings.humWarning == 3)) && 
                    ((functionWarnings.LightWarning == 4) || (functionWarnings.LightWarning == 3))) {
                    // Apply condition for the WINDOW warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & ROOM_WINDOW)) {
                        rooms[room].conditionwarnings |= ROOM_WINDOW;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                } 

                // Check the opposite arrangement (swapped plant index and number)
                else if (((functionWarnings.humWarning == 4) || (functionWarnings.humWarning == 3)) && 
                        ((loopWarnings.LightWarning == 4) || (loopWarnings.LightWarning == 3))) {
                    // Apply condition for the WINDOW warning only if it's not already set
                    if (!(rooms[room].conditionwarnings & ROOM_WINDOW)) {
                        rooms[room].conditionwarnings |= ROOM_WINDOW;
                    }
                    
                    if(!(rooms[room].conditionFlag & DIFFTYPE)) {
                        rooms[room].conditionFlag |= DIFFTYPE;  // Set the TEMP bit if it wasn't already set
                    }
                }
            }
        plants[plantindex].warnings = loopWarnings;
    }
    plants[plantNumber].warnings = functionWarnings;
}

void roomBasedWarnings(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings){
    int16_t roomvalues[3];

    for (int i = 0; i < 3; i++) {
        roomvalues[i] = 100;
    }
    

    for (int plant = 0; plant < 3; plant++)
    {   
        warnings_t currentWarnings = plants[plant].warnings;

        currentWarnings.Plantwarning = 0;
        if(currentWarnings.tempWarning > 0){
            warningloop(plants, rooms, plantSettings, plants[plant].roomNumber, TEMP, plant);
            if(rooms[plants[plant].roomNumber].conditionFlag == 0){
                currentWarnings.Plantwarning |= TEMP;
            }
        } 
        
        if(currentWarnings.humWarning > 0) {
            warningloop(plants, rooms, plantSettings, plants[plant].roomNumber, HUM, plant);
            if(rooms[plants[plant].roomNumber].conditionFlag == 0){
                currentWarnings.Plantwarning |= HUM;
            }
        } 
        
        if(currentWarnings.LightWarning > 0) {
            warningloop(plants, rooms, plantSettings, plants[plant].roomNumber, LIGHT, plant);
            if(rooms[plants[plant].roomNumber].conditionFlag == 0){
                currentWarnings.Plantwarning |= LIGHT;
            }
        } 

        if(currentWarnings.Plantwarning > 0){
            for(int newroom = 0; newroom < 3; newroom++) {
                roomvalues[newroom] = compareRoomSuitability(plants, rooms, plantSettings, plant, newroom);
            }

            currentWarnings.newroom = findLowestValueIndex(roomvalues, 3, plant);
        }

        plants[plant].warnings = currentWarnings;
    }
}

void higherOrLower(uint8_t warningtype, uint8_t line, uint8_t placement){
    if(warningtype == 1){
        mvprintw(line + 6, 30 + placement, "too low                ");
    } else if (warningtype == 2){
        mvprintw(line + 6, 30 + placement, "too high               ");
    }
}

void returnWarnings (plantSettings_t *plants, roomSettings_t *rooms){
    uint8_t numberOfWarningLines = 0;
    uint8_t numberOfWarningLinesRoom = 0;

    for (int plantIndex = 0; plantIndex < 3; plantIndex++)
    {   
        warnings_t currentWarnings = plants[plantIndex].warnings;
        
        if(!currentWarnings.warningtype) continue;
        
        mvprintw(numberOfWarningLines + 6, 30, "Plant warnings for %d:         ", plantIndex + 1);
        numberOfWarningLines++;

        if (currentWarnings.Plantwarning > 0){
            if (currentWarnings.warningtype & TEMP) {
                if(currentWarnings.tempWarning < 3){
                    mvprintw(numberOfWarningLines + 6, 30, "Temperature is: ");
                    higherOrLower(currentWarnings.tempWarning, numberOfWarningLines, strlen("Temperature is: "));
                    numberOfWarningLines++;
                }
            }
            if (currentWarnings.warningtype & HUM) {
                if(currentWarnings.humWarning < 3){
                    mvprintw(numberOfWarningLines + 6, 30, "Humidity is: %d ", currentWarnings.humWarning);
                    higherOrLower(currentWarnings.humWarning, numberOfWarningLines, strlen("Humidity is: "));
                    numberOfWarningLines++;
                }
            }
            if (currentWarnings.warningtype & LIGHT) {
                if(currentWarnings.LightWarning < 3){
                    mvprintw(numberOfWarningLines + 6, 30, "Light intensity is: ");
                    higherOrLower(currentWarnings.LightWarning, numberOfWarningLines, strlen("Light intensity is: "));
                    numberOfWarningLines++;
                }
            }
            if (currentWarnings.warningtype & WATER) {
                if(currentWarnings.waterWarning < 3){
                    mvprintw(numberOfWarningLines + 6, 30, "Water level is: ");
                    higherOrLower(currentWarnings.waterWarning, numberOfWarningLines, strlen("Water level is: "));
                    numberOfWarningLines++;
                }
            }
            if(currentWarnings.newroom != plants[plantIndex].roomNumber){
                mvprintw(numberOfWarningLines + 6, 30, "Move plant to room %d     ", currentWarnings.newroom + 1);
            } else {
                mvprintw(numberOfWarningLines + 6, 30, "fix at source      ");
            }
            numberOfWarningLines += 2;
        } 
    }

    for (int room = 0; room < 3;room++)
    {   
        //if(!rooms[room].solution && !rooms[room].conditionwarnings) continue;
        mvprintw(numberOfWarningLinesRoom + 6, 63, "Warnings for room: %d", room + 1);
        numberOfWarningLinesRoom++;
        if (rooms[room].conditionFlag & SAMETYPE)
        {
            // Check if each warning is active and print the corresponding message
            if (rooms[room].solution & TOHOT) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Room to hot                                   ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOCOLD) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Room to cold                                  ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOWET) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Humidity in the room is to high               ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TODRY) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Humidity in the room is to low                ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TODARK) {  
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Room has too little light                     ");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].solution & TOLIGHT) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Room has too much light                       ");
                numberOfWarningLinesRoom++;
            }
        }
        if (rooms[room].conditionFlag & DIFFTYPE)
        {
            // Check if each warning is active and print the corresponding message
            if (rooms[room].conditionwarnings & DRYING) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Plants in the room are drying out (low humid + ground water)");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & OVERWATERING) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Plants in the room are drowning (high humid + ground water)");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & FREEZING) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Plants in the room are freezing (high ground water + low temp)");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & WARMING) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Plants in the room are too hot (high temp + light level)");
                numberOfWarningLinesRoom++;
            }
            if (rooms[room].conditionwarnings & ROOM_WINDOW) {
                mvprintw(numberOfWarningLinesRoom + 6, 63, "Plants in the room are window");
                numberOfWarningLinesRoom++;
            }
        }
        numberOfWarningLinesRoom++;
    }
    
    
}
