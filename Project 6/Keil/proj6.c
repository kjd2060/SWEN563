#include "stm32l476xx.h"    // Board
#include "SysClock.h"       // Sysclock
#include "LED.h"            // LED
#include "UART.h"           // UART
#include "TIMER.h"          // TIMER

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SMALLEST_WIDTH (400)
#define MINIMUM_VOLTAGE (0)
#define MAXIMUM_VOLTAGE (255)

uint8_t bounds[50]; //UART print buffer

/* 
 * STM Program.  Monitor pins e10-15 and h0-1 for voltage from the QNX program.  Resolve CCR value with equation from
 * trendline found in excel.  Equation scales and converts 0-255 values to the correct CCR for the scaled voltage.
*/
int main(void){
	// vars
    int n;
    int pin_voltage;    
    int pulse_width;
    double fine_ccr;

    // init everything
    System_Clock_Init();
    LED_Init();
    UART2_Init();
    PWM_Init();
    GPIO_Init();
	
		TIM2->CCR1 = 400;
	
		//init joystick up and down
		//RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN ;       // enable GPIOA clock
		//GPIOA->MODER &= ~0xF ;  										// clear GPIOA bits		GPIOA->MODER &= 0xC3;
		//GPIOA->PUPDR &= 0x28;

    // initial message; helpful to tell we got passed the inits
    USART_Write(USART2, (uint8_t *)"\r\nProject 6 Starting\r\n", 20);

    // Poll indefinitely to read output of purplebox
    while(1){ 
        pin_voltage = Get_Voltage();

        if (pin_voltage < MINIMUM_VOLTAGE || pin_voltage > MAXIMUM_VOLTAGE){ 
            Red_LED_On();
            USART_Write(USART2, (uint8_t *)"\r\nVoltage out of range.\r\n", 25);
        }
        else{
						/*if ( GPIOA->IDR & 0x2 != 0x00){
							Enable_PWM();
							Green_LED_On();
							while((GPIOA->IDR & 0x2) != 0x00);
						}
						
						if ( GPIOA->IDR & 0x4 != 0x00){
							Disable_PWM();
							Green_LED_Off();
							while((GPIOA->IDR & 0x4) != 0x00);
						}*/
					
						if (pin_voltage < 1){
								// attempt to start the pwm when the servo is at the initial position
								Enable_PWM();
						}
            Red_LED_Off();
						// see graph for actual convertion
						fine_ccr = (double)pin_voltage * 6.275 + SMALLEST_WIDTH;
            pulse_width = (int)(fine_ccr+.5); 		
						//work around for no math round function
            n = sprintf((char *)bounds, "Seen voltage: [%f] || CCR: [%f], rounded [%d]\r\n\r\n", ((pin_voltage/25.5)-5.0),fine_ccr,rounded_ccr); // buffering trick, used in proj. 2
            USART_Write(USART2, bounds, n);
            TIM2->CCR1 = pulse_width;
				}
    }
}
