/*
 * Data.h
 *
 * Created: 20/12/2024 13:20:49
 *  Author: teunv
 */ 


#ifndef PLANT_DATA_H_
#define PLANT_DATA_H_

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

	// Define plant types as an enum
	typedef enum {
		TYPE_A,  // Plant type A
		TYPE_B,  // Plant type B
		TYPE_C,  // Plant type C
		TYPE_D   // Plant type D
	} plantType_t;

	typedef struct {
		//Temperatuurwensen
		int16_t minimumTemperature;
		int16_t maximumTemperature;
		int16_t optimalTemperature;

		//Luchtvochtigheidswensen
		uint16_t minimumHumidity;
		uint16_t maximumHumidity;
		uint16_t optimalHumidity;

		//lichtwensen
		uint16_t minimumLightneeds;
		uint16_t maximumLightneeds;
		uint16_t optimalLightneeds;

		//Waterwensen
		uint16_t minimumWaterheight;
		uint16_t maximumWaterheight;

        char *name;
	} plant_t;

	typedef struct {
		uint8_t tempWarning;
		uint8_t humWarning;
		uint8_t LightWarning;
		uint8_t waterWarning;
		uint8_t warningtype;

		uint8_t Plantwarning;
		uint8_t newroom;
	} warnings_t;

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

		time_t lastTemperature;
		time_t lastHumidity;
		time_t lastLight;

		uint8_t solution;
		uint8_t conditionwarnings;
		uint8_t conditionFlag;
	}roomSettings_t;

	typedef struct plantSettings{
		uint8_t groundSensor;
		int roomNumber;

		float groundWater;

		time_t lastGround;

		plantType_t typePlant; 
		warnings_t warnings;
	}plantSettings_t;

#endif // PLANT_DATA_H_