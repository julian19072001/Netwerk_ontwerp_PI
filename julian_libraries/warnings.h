#ifndef WARNINGS_H_
#define WARNINGS_H_

#include <data.h>
#include <stdint.h>
#include <stdio.h>

#define SAMETYPE 0x01
#define DIFFTYPE 0x02


#define DRYING  0x01  // 0 0001
#define OVERWATERING   0x02  // 0 0010
#define FREEZING  0x04  // 0 0100
#define WARMING   0x08  // 0 1000
#define ROOM_WINDOW   0x10  // 1 0000



//defines for the warning system
#define TOHOT  0x01  // 0 0001
#define TOCOLD   0x02  // 0 0010
#define TOWET  0x04  // 0 0100
#define TODRY   0x08  // 0 1000
#define TOLIGHT   0x10  // 1 0000
#define TODARK  0x20  // 11 0000

//defines for the warningtypes
#define TEMP  0x01  // 0001
#define HUM   0x02  // 0010
#define LIGHT  0x04  // 0100
#define WATER   0x08  // 1000

void plantBasedWarnings(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings);
void roomBasedWarnings(plantSettings_t *plants, roomSettings_t *rooms, plant_t *plantSettings);
void returnWarnings (plantSettings_t *plants, roomSettings_t *rooms);

#endif 