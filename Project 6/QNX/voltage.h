/*
 * voltage.h
 *
 *  Created on: Dec 14, 2017
 *      Author: kxl7131
 */

#ifndef VOLTAGE_H_
#define VOLTAGE_H_

#include <stdint.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>

#define PORT_SIZE (8)
#define INT_VALUE (32768.0)
#define BASE (0x280)

uintptr_t command_register;
uintptr_t a2d_msb;
uintptr_t a2d_channel_register;
uintptr_t a_input_status;
uintptr_t a2d_output_port;
uintptr_t output_control_register;

void init_A2D();
double A2D();
void send_voltage(double voltage);

#endif /* VOLTAGE_H_ */
