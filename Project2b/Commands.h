/*
 * Commands.h
 *
 *  Created on: Nov 29, 2017
 *      Author: kxl7131
 *
 *      #todo make program state account for if it needs a pause
 *      instead of each servo calculating pause
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#ifndef COMMANDS_H
#define COMMANDS_H

#include "commands.h"
#include <stdlib.h>

#define MOV (0x20)
#define WAIT (0x40)
#define LOOP (0x80)
#define END_LOOP (0xA0)
#define RECIPE_END (0)
#define STEP_INTERVAL (320)
#define SMALLEST_WIDTH (400)
#define DEFAULT_STATE (3)
#define NUM_SERVOS (2)

enum servo_states
{

	state_paused,
	state_running,
	state_command_error,
	state_nested_error,
	state_recipe_end

} ;

enum status
{
	status_input_read,
	status_running,
	status_buffering,
	status_done

} ;

typedef struct{
	enum servo_states servo_state;

	int current_index;
	int loop_start_index;
	int loop_counter;
	int times_to_loop;
	int position;

}	servo;

void Init_Servos( void );
void Move_Buffering( int moves );
void Update_LEDs( void );
void Run_State( void );
void process_recipe( int index_number, int servo );
void override_process( char input, int servo );
void Turn_Left( int servo );
void Turn_Right( int servo );


#endif /* COMMANDS_H */


#endif /* COMMANDS_H_ */
