/*
 * Commands.c
 *
 *  Created on: Nov 29, 2017
 *      Author: kxl7131
 */

#include "commands.h"

int servo_number;
int override_flag = 0;
int wait_time = 1;
int in_loop[2] = {0, 0};
enum status program_status = status_running;
char line[10];
char output[50];

char rxByte;
servo servos[2];

char recipe1[100] =
{
	MOV | 0,
	MOV | 5,
	MOV | 0,
	MOV | 3,
	LOOP | 0,
	MOV | 1,
	MOV | 4,
	END_LOOP,
	MOV | 0,
	MOV | 2,
	WAIT | 0,
	MOV | 3,
	WAIT | 0,
	MOV | 2,
	MOV | 3,
	WAIT | 31,
	WAIT | 31,
	WAIT | 31,
	MOV | 4,
	RECIPE_END

};

char recipe_ascend[100] =
{
	MOV | 0,
	MOV | 1,
	MOV | 2,
	MOV | 3,
	MOV | 4,
	MOV | 5,
	RECIPE_END
};

char recipe_descend[100] =
{
	MOV | 5,
	MOV | 4,
	MOV | 3,
	MOV | 2,
	MOV | 1,
	MOV | 0,
	MOV | 1,
	RECIPE_END
};

char recipe_loop[100] = {
	MOV | 0,
	LOOP | 3,
	MOV | 4,
	MOV | 5,
	0xA0,
	MOV | 2,
	RECIPE_END

};

char recipe_nested[100] = {
	MOV | 0,
	LOOP | 2,
	LOOP | 3,
	END_LOOP,
	END_LOOP,
	RECIPE_END
};

char recipe_command[100] = {
	MOV | 0,
	0xFF,
	RECIPE_END
};


void Init_Servos(){
	//will run through both servos and setting the default values for both
	int i;
	for ( i = 0; i < NUM_SERVOS; i++ ){
		servos[i].servo_state = state_running;
		servos[i].current_index = 0;
		servos[i].loop_counter = 0;
		servos[i].position = DEFAULT_STATE;
	}
}

void Move_Buffering( int moves ){

	int delayTime = ( moves ) * 400;
	// set delay of 400ms per movement to milliseconds
	//	sprintf(output, "waiting %lu \r\n", delay);
	//	Write_Line(output);
	delay( delayTime );
}

void Clear_Input_Buffer(){
	int i = 0;
	while(line[i] != '\0'){
		line[i] = '\0';
	}
}

void *Override_Thread(){
    int index = 0;
    int c;
    while(1){
        index = 0;
        while( (c = getchar()) != '\n' ) {
            line[index] = c;
            index++;
            if ( c == 'x' || c == 'X'){
            	break;
            }
        }
        line[10] = '\0';
        printf("line is %s", line);
        override_flag = 1;
    }
}

void Run_State(){
	//main function that handles the servo states and the program status
	printf("\r\n>");
	pthread_create (NULL, NULL, Override_Thread, NULL);
	while( program_status != status_done ){
		int i;
		int num_servos_done = 0;

		if ( override_flag == 1 ){
			//as the input is 1 char, it needs to be added to the start of the input
			program_status = status_input_read;
		}

		 switch( program_status ){

			case status_input_read:
				if ( line[6] == '>' ){
					//checking for correct format
					for ( i = 0; line[i] != '\0'; i++ ){
						if (line[i] == 'x' || line[i] == 'X'){
							//if x is in the input, the override is cancelled
							printf("\r\n>");
							servos[0].servo_state = state_running;
							servos[1].servo_state = state_running;
							override_flag = 0;
						}
					}
						if ( override_flag == 1 ){
							//if override has no x or X, it goes ahead
							for ( servo_number = 0; servo_number < NUM_SERVOS; servo_number++ ){
								override_process( line[servo_number], servo_number );
							}
						}

					}
				printf("\r\n>");
				override_flag = 0;
				//clear override flag
				fflush(stdout);
				Clear_Input_Buffer();
				program_status = status_running;


			case status_running:

				for ( servo_number = 0; servo_number < NUM_SERVOS; servo_number++ ){

					if ( servos[servo_number].servo_state == state_running ){
						//process recipe and increment index
						process_recipe( servos[servo_number].current_index, servo_number );
						servos[servo_number].current_index++;
					}

					else if ( servos[servo_number].servo_state == state_recipe_end ){
						//need to check if all the servos are done

						for ( servo_number = 0; servo_number < NUM_SERVOS; servo_number++ ){
							if ( servos[servo_number].servo_state == state_recipe_end ){
								num_servos_done++;
							}
						}

						if ( num_servos_done == NUM_SERVOS ){
							program_status = status_done;
						}
					}

					else if ( override_flag == 1 ){
						//user has input something
						program_status = status_input_read;
						}

					else{
						//error and pause states do nothing
					}
				}
			 case status_buffering:
				 Move_Buffering(wait_time);
				 wait_time = 0;
				 program_status = status_running;
		}
	}
}

void process_recipe( int index_number, int servo ){
	//function that will process a opcode + value from the recipe
	unsigned char temp;
	int value;
	int opcode;

	if ( servo == 0 ){
		//if its servo 0
		temp = recipe1[index_number];
	}
	else if ( servo == 1 ){
		// or servo 1
		temp = recipe_loop[index_number];
	}

	opcode = temp >> 5;
	//shifting the recipe 5 bits to get just the opcode

	if ( opcode == OP_MOV ){
		// MOV opcode - moves the servo to the specified position
		int pulse_width;
		int delay_cycles;

		temp &= 0x1F;
		//clearing the opcode to get just the value
		value = temp;
		delay_cycles = abs(servos[servo].position - value);
		if (delay_cycles > wait_time){
			wait_time = delay_cycles;
		}
		servos[servo].position = value;
		//	sprintf(output, "moving servo %d to %d\r\n", servo, servos[servo].position);
		//	Write_Line(output);
		pulse_width = SMALLEST_WIDTH + value*STEP_INTERVAL;
		if ( pulse_width < SMALLEST_WIDTH ){
			pulse_width = SMALLEST_WIDTH;
		}

		Change_Width( pulse_width, servo );
		//	sprintf(output, "servo moving %d positions\r\n", delay_cycles);
		//	Write_Line(output);
		//caluculate the difference in positions to set delay
		program_status = status_buffering;

	}

	else if ( opcode == OP_WAIT ){
		//WAIT opcode
		long delayTime;
		temp &= 0x1F;
		value = temp;

		value++;
		//wait 0 is a wait of 1 cycle so the value must be adjusted

		delayTime = value*100;
		delay(delayTime);

		}

	else if ( opcode == OP_LOOP ){
		//LOOP opcode
		//save current index
		temp &= 0x1F;
		servos[servo].times_to_loop = temp;
		servos[servo].loop_start_index = index_number;

		if( in_loop[servo] != 0 ){
		//NESTED_LOOP_ERROR: BOTH RED AND GREEN LEDS ON
			servos[servo].servo_state = state_nested_error;
		}
		in_loop[servo] = 1;
	}

	else if ( opcode == OP_END_LOOP ){
		//END_LOOP opcode
		//set index to index of the loop
		//if loop counter is = to value, dont change index

		if ( !in_loop[servo] ){
			//the recipe is bad. there is an END_LOOP before a START_LOOP
			//nested error
			servos[servo].servo_state = state_nested_error;
		}

		if ( servos[servo].loop_counter == servos[servo].times_to_loop ){
			//if loop_counter is equal to times_to_loop
			//it has reached the number of times to loop
			//turn off the in loop flag
			in_loop[servo] = 0;
		}

		else{
			//increment loop counter
			servos[servo].loop_counter++;
			servos[servo].current_index = servos[servo].loop_start_index;

		}
	}

	else if ( opcode == RECIPE_END ){
		//end of recipe opcode
		servos[servo].servo_state = state_recipe_end;

	}

	else{
		//command unrecognized
		servos[servo].servo_state = state_command_error;
	}
}

void override_process( char input, int servo ){
	//function that processes the override command

	if (input == 'P' || input == 'p'){
		//pause override - sets state to pause as long as
		//it isnt in an error state or at the end of a recipe
		if (servos[servo].servo_state != state_recipe_end &&
			servos[servo].servo_state != state_nested_error &&
			servos[servo].servo_state != state_command_error){

			servos[servo].servo_state = state_paused;
			//	sprintf(output, "current state is %d\r\n", servos[servo].servo_state);
			//	Write_Line(output);
		}
	}

	else if ( input == 'C' || input == 'c' ){
		//continue state - will unpause the recipe as long as
		//its not in error or at the end
		if (servos[servo].servo_state != state_recipe_end &&
			servos[servo].servo_state != state_nested_error &&
			servos[servo].servo_state != state_command_error){

			servos[servo].servo_state = state_running;
		}
	}

	else if ( input == 'R' || input == 'r' ){
		//turn servo right as long as its paused and not
		//at the right most position (0)
		if (servos[servo].servo_state == state_paused &&
			servos[servo].position != 0 ){

			Turn_Right( servo );
			Move_Buffering(1);
		}
	}

	else if ( input == 'L' || input == 'l' ){
		//turn servo left as long as its paused and not
		//at the left most position (5)
		if (servos[servo].servo_state == state_paused &&
			servos[servo].position != 5 ){

			Turn_Left( servo );
			Move_Buffering(1);
		}
	}

	else if ( input == 'N' || input == 'n' ){
		//do nothing

	}

	else if ( input == 'B' || input == 'b' ){
		//restart recipe - sets the current index to 0
		servos[servo].servo_state = state_running;
		servos[servo].current_index = 0;
	}
}

void Turn_Left( int servo ){
	int pulse_width;
	servos[servo].position++;
	//	sprintf(output, "moving servo %d left to %d\r\n", servo, servos[servo].position);
	//	Write_Line(output);
	pulse_width = SMALLEST_WIDTH + servos[servo].position*STEP_INTERVAL;
	if ( pulse_width < SMALLEST_WIDTH ){
		pulse_width = SMALLEST_WIDTH;
	}
	Change_Width( pulse_width, servo );
}

void Turn_Right( int servo ){
	int pulse_width;
	servos[servo].position--;
	//	sprintf(output, "moving servo %d right to %d\r\n", servo, servos[servo].position);
	//	Write_Line(output);
	pulse_width = SMALLEST_WIDTH + servos[servo].position*STEP_INTERVAL;
	if ( pulse_width < SMALLEST_WIDTH ){
		pulse_width = SMALLEST_WIDTH;
	}
	Change_Width( pulse_width, servo );
}

