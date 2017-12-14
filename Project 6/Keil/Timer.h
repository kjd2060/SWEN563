#ifndef Timer_H
#define Timer_H

#include "stm32l476xx.h"
#include "SysClock.h"   // Sysclock
#include <stdio.h>
#include <stdlib.h>
#include "UART.h"       // UART

#define PORT_SIZE (8)

// function prototypes for Timer
void PWM_Init(void);
int fetch_voltage(void);
void GPIO_Init(void);



#endif /* Timer_H */


