/*Julian Della Guardia*/

#define F_CPU 32000000UL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <mesh_radio.h>

#define DEVICE_ADDRESS  0x45                           //address of device (each address can only be used once in a network)


int main(void)
{
	radioInit(DEVICE_ADDRESS);
	
	while(1)
	{	
		uint8_t data[MAX_DATA_LENGTH] = {0};
		
		uint8_t numByte = readRadioMessage(data);

		if (numByte){
			data[numByte] = '\0';
			printf("From: %02d, Payload: %s, numByte: %d\n", data[0], data + 1, numByte);  // Process the payload
		} 

	}
}

//gcc -Wall -o nrftest nrftest.c ~/Netwerk_ontwerp_PI/julian_libraries/*.c -I/home/julian/Netwerk_ontwerp_PI/julian_libraries -lncurses -lwiringPi

