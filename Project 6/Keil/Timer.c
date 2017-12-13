#include "stm32l476xx.h"
#include "SysClock.h"   // Sysclock
#include <stdio.h>
#include <stdlib.h>
#include "UART.h"       // UART

uint8_t scaleDisplay[50];

/* Timer init and pin config for PWM on PA0 */
void PWM_Init(void){
    
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN ;       // enable GPIOA clock
    GPIOA->MODER &= ~0xF ;  										// clear GPIOA bits
    GPIOA->MODER |= 0x02 ;                      // Enable alternate function mode (binary 10) for PA0
	GPIOA->MODER |= 0x08 ;											// enable AF mode for PA1
	
    GPIOA->AFR[0] |= 0x1;                   		// set alternate function for PA0; using AF1 (TIM2_CH1)
    GPIOA->AFR[0] |= 0x10;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;   		// TIM2 timer clock enable

    TIM2->PSC = 100;				        						// set prescale to 100 us per count
    TIM2->EGR |= TIM_EGR_UG;										// set the update generation bit; see RM for more info, but forces an update of the register

	TIM2->CCMR1 &= ~0x303;
    TIM2->CCMR1 |= 0x68;                    		// setup channel for output compare 1 preload, enable PWN mode 1
	TIM2->CCMR1 |= 0x6800;
	TIM2->CR1 |= 0x80;													// Enable auto-reload preload enable
	TIM2->CCER |= 0x01;													// enable channel 1 output bit
	TIM2->CCER |= 0x10;

	TIM2->ARR = 0x4E20;													// set the period of the PWN to 2000 
	TIM2->CCR1 = 1360;													// set the width of the pwn to 1360 (position 3)
	TIM2->CCR2 = 1360;
	
    TIM2->EGR |= TIM_EGR_UG;				// set the update generation bit; see RM for more info, but forces an update of the register
	TIM2->SR &= ~TIM_SR_UIF;
    
}

/* Read voltage */
int get_voltage(void){
    
}

/* Set up pins as inputs for QNX signal to be read from. */
void GPIO_Init(void){
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN ;       // enable GPIOA clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOHEN ;       // enable GPIOA clock
    
    GPIOE->MODER &= 0x000FFFFF;
    GPIOH->MODER &= 0xFFFFFFF0;

    
    
}
