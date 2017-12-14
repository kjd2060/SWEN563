#include <stdlib.h>
#include <stdio.h>
#include "voltage.h"

#define INT_VALUE (32768.0)
#define MINIMUM_VOLTAGE (-5)
#define MAXIMUM_VOLTAGE (5)


int main(int argc, char *argv[]) {

	init_A2D();
	double voltage = 0;

	while(1){
		voltage = A2D();
		voltage = (double)voltage/INT_VALUE;
		voltage *= 5;
		//voltage = (voltage/INT_VALUE) * 5;
		//printf("voltage before scal is %f\n", voltage);
		if ( voltage > 5.0 || voltage < -5.0 ){
			printf("VOLTAGE NOT IN ACCEPTABLE RANGE\n");
		}
		else{
			//scale voltage from -5.0-5.0 to 0-255
			voltage += 5.0;
			voltage *= 25.5;
			//printf("voltage is %d\n", voltage);

			send_voltage(voltage);
		}
	}

}
