#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

	//this shit doesnt work dont build it
	System_Clock_Init();
	UART2_Init();

	LED_Init();
	Timer_Init();
	Start_Timer();
	Init_Servos();
	//Change_Width(2000,1);
	//Change_Width(2000,0);
	Move_Buffering(5);
	Run_State();

}
