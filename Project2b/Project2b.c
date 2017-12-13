#include <stdlib.h>
#include <stdio.h>
#include <timer.h>
#include "Commands.h"

#include <unistd.h>
#include <time.h>


#define BASE (0x280)  // default base address

int main(int argc, char *argv[]) {

	pwmInit();
	sleep(2);
	Init_Servos();
	Run_State();
	sleep(400);
	return 0;

}

