/*
 * Data.h
 *
 * Created: 20/12/2024 13:20:49
 *  Author: teunv
 */ 


#ifndef PLANT_DATA_H_
#define PLANT_DATA_H_

#include <unistd.h>

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

		//lichtwensen
		uint16_t minimumLightneeds;
		uint16_t maximumLightneeds;

		//Waterwensen
		uint16_t minimumWaterheight;
		uint16_t maximumWaterheight;

        char *name;
	} plant_t;

#endif // PLANT_DATA_H_