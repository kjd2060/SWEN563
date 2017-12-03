#include "timer.h"

int pwmRunning = 1;
uintptr_t ctrlHandle;
uintptr_t portA;

typedef struct {
    int arg1;
} PWMArgs;

int Duty_cycle[2] = {500000, 500000};

/* PWM controls how far the servo moves. */
void changePWM(uint8_t pwmNum, uint32_t pwmVal){
    if(pwmNum == PWM0){
        Duty_cycle[pwmNum] = pwmVal;
    }
    
    if(pwmNum == PWM1){ 
        Duty_cycle[pwmNum] = pwmVal;
    }
    
}

/* 
 * Thread to do the PWM stuff.
 */
void * pwmThread(void *args){

}

/* 
 * Initializes pwm and does root access request. 
 * Also sets up memory space
 */ 
void pwmInit(){
    PWMArgs pwmArgs;
    ThreadCtl(_NTO_TCTL_IO, NULL);
    uintptr_t ctrl = mmap_device_io(8, BASE + 10);
    if(ctrlHandle == MAP_DEVICE_FAILED)
    {
        perror("map control register fails");
    }
    out8(ctrl, 0x00 );
    for (j = 0; j < NUMOFPWM; j++) {
        pwmArgs.arg1 = j;
        pthread_create (NULL, NULL, pwmThreadFunc, (void *)&pwmArgs);
    }
}
