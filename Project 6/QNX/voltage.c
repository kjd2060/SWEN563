/*
 * voltage.c
 *
 *  Created on: Dec 14, 2017
 *      Author: kxl7131
 */

#include "voltage.h"

void init_A2D(){

    ThreadCtl(_NTO_TCTL_IO, NULL);
    //get access to registers

    command_register = mmap_device_io(PORT_SIZE, BASE + 0);
    a2d_msb = mmap_device_io(PORT_SIZE, BASE + 1);
    a2d_channel_register = mmap_device_io(PORT_SIZE, BASE + 2);
    a_input_status = mmap_device_io(PORT_SIZE, BASE + 3);
    a2d_output_port = mmap_device_io(PORT_SIZE, BASE + 8);
    output_control_register = mmap_device_io(PORT_SIZE, BASE + 11);

    out8(command_register, 0x10);
    //clear stack
    out8(output_control_register, 0x00);
    //set port a to output
    out8(a_input_status, 0x01);
    //bind range to 5V
    out8(a2d_channel_register, 0xFF);
    //set channel to VI15


}

double A2D(){

	//double voltage_temp;
	short raw = 0;
	int LSB, MSB = 0;

	//wait for a/d circuit to settle on a value
	while(in8(a_input_status) & 0x20){
		;
	}
	//initiate conversion
	out8(command_register, 0x80);

	//wait while conversion is happening
	while (in8(a_input_status) & 0x80){
		;
	}

	//get value and convert
	LSB = in8(command_register);
	MSB = in8(a2d_msb);
	raw = (MSB << 8) + LSB;
	//voltage_temp = ((double)raw/INT_VALUE) * 5;
	//printf("voltage is %f\n", voltage);

	//return voltage
	return raw;
}


void send_voltage(double voltage){
	out8( a2d_output_port, voltage );
}
