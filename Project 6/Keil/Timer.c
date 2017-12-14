#include "Timer.h"


//enable PWM
void Enable_PWM(){
    TIM2->CR1 |= 0x01;													// control register enabled to start timing
}

//disable PWM
void Disable_PWM(){
    TIM2->CR1 &= 0xFFFE;                       // control register diabled
}

//initialize registers for PWM on output pin PA0
void PWM_Init(void){
    
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN ;       // enable GPIOA clock
    GPIOA->MODER &= ~0xF ;  										// clear GPIOA bits
    GPIOA->MODER |= 0x02 ;                      // Enable alternate function mode (binary 10) for PA0
	
    GPIOA->AFR[0] |= 0x1;                   		// set alternate function for PA0; using AF1 (TIM2_CH1)
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;   		// TIM2 timer clock enable

    TIM2->PSC = 100;				        						// set prescale to 100 us per count
    TIM2->EGR |= TIM_EGR_UG;										// set the update generation bit; see RM for more info, but forces an update of the register

		TIM2->CCMR1 &= ~0x303;
    TIM2->CCMR1 |= 0x68;                    		// setup channel for output compare 1 preload, enable PWN mode 1
		TIM2->CCMR1 |= 0x6800;
		TIM2->CR1 |= 0x80;													// Enable auto-reload preload enable
		TIM2->CCER |= 0x01;													// enable channel 1 output bit

		TIM2->ARR = 0x4E20;													// set the period of the PWN to 2000 
		TIM2->CCR1 = 1360;													// set the width of the pwn to 1360 (position 3)
	
    TIM2->EGR |= TIM_EGR_UG;										// set the update generation bit; see RM for more info, but forces an update of the register
		TIM2->SR &= ~TIM_SR_UIF;
    
}


/*
* Bit map -> ports is as follows:
* bit 8 A7 -> PH0			128
* bit 7 A6 -> PH1			64
* bit 6 A5 -> PE11 		32
* bit 5 A4 -> PE10 		16
* bit 4 A3 -> PE12 		8
* bit 3 A2 -> PE13 		4
* bit 2 A1 -> PE14  	2
* bit 1 A0 -> PE15 		1
*/

int Get_Voltage(void){
		int total_input = 0;
		int i;
	
		if(GPIOH->IDR & 0x0001){
			total_input += 128;
		}
		if(GPIOH->IDR & 0x0002){
			total_input += 64;
		}
		if(GPIOE->IDR & 0x0800){
			total_input += 32;
		}
		if(GPIOE->IDR & 0x0400){
			total_input += 16;
		}
		if(GPIOE->IDR & 0x1000){
			total_input += 8;
		}
		if(GPIOE->IDR & 0x2000){
			total_input += 4;
		}
		if(GPIOE->IDR & 0x4000){
			total_input += 2;
		}
		if(GPIOE->IDR & 0x8000){
			total_input += 1;
		}
		
		
		return total_input;
	
}

/* Set up pins as inputs for QNX signal to be read from. */
void GPIO_Init(void){
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN ;       // enable GPIOE clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOHEN ;       // enable GPIOH clock
    
    GPIOE->MODER &= 0x000FFFFF;									// enable ports E10 - E15
    GPIOH->MODER &= 0xFFFFFFF0;									// enable ports H0-H1

    
    
}
