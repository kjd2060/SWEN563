#ifndef __timer_H
#define __timer_H

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

#define PWM0 (0)
#define PWM1 (1)
#define IO_PORT_SIZE (8) // port size for digital io ports on our platform
#define BASE (0x280)  // default base address
#define NUMOFPWM (2)  // need 2 pwms
#define NANOSECOND (1000000000) // I think we'll need this after looking through the ref manual
#define PERIOD (20000000)
#define DEFAULT_POSITION (1380000)
#define HIGH (0x01)
#define LOW (0x00)

void pwmInit();
void *pwmThread(void *args);
void Change_Width( int servoNum, int position );
void msSleep(double milliseconds, int servoNum);

#endif
