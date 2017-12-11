#include "stm32l476xx.h"    // Board
#include "SysClock.h"       // Sysclock
#include "LED.h"            // LED
#include "UART.h"           // UART
#include "TIMER.h"          // TIMER

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t bounds[50]; //UART print buffer

/* 
 * STM Program.  Monitor pins e10-15 and h0-1 for voltage from the QNX program.  Resolve CCR value with equation from
 * trendline found in excel.  Equation scales and converts 0-255 values to the correct CCR for the scaled voltage.
*/
int main(void){
	// vars
    int n;
    int pin_voltage;    
    int rounded_ccr;
    double fine_ccr;

    // init everything
    System_Clock_Init();
    LED_Init();
    UART2_Init();
    PWM_Init();
    GPIO_Init();

    // initial message; helpful to tell we got passed the inits
    USART_Write(USART2, (uint8_t *)"\r\nProject 6 Starting\r\n", 20);

    // Poll indefinitely to read output of purplebox
    while(1){ 
        pin_voltage = fetch_voltage();

        //New scaling from QNX
        if (pin_voltage < 0 || pin_voltage > 255){ 
            Red_LED_On();
            USART_Write(USART2, (uint8_t *)"\r\nVoltage out of range.\r\n", 25);
        }
        else{
            Red_LED_Off();
            fine_ccr = (-0.0667*(double)pin_voltage)+21.242; //Equation to convert qnx scale to a ccr value - dependent on QNX scale used
            rounded_ccr = (int)(fine_ccr+.5); //work around for no math round function
            n = sprintf((char *)bounds, "Seen voltage: [%f] || CCR: [%f], rounded [%d]\r\n\r\n", ((pin_voltage/25.5)-5.0),fine_ccr,rounded_ccr); // buffering trick, used in proj. 2
            USART_Write(USART2, bounds, n);
            TIM2->CCR1 = rounded_ccr;
        }
    }
}
