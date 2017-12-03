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
#define IO_PORT_SIZE (8)
#define BASE (0x280)
#define NUMOFPWM (2)
#define NANOSECOND (1000000000)

void pwmInit();
void * pwmThread(void *arguments);
void changePWM(uint8_t PWMNO, uint32_t pwmValue);

#endif
