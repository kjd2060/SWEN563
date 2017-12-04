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
    
    // see http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Ft%2Fthreadctl.html
    // allows the thread to access lower level I/O and memory privledges
    ThreadCtl(_NTO_TCTL_IO, NULL);
    
    uintptr_t ctrl = mmap_device_io(8, BASE + 10); // use Digital I/O port C
    if(ctrlHandle == MAP_DEVICE_FAILED)
    {
        perror("map control register fails");
    }
    out8(ctrl, 0x00 );                             // write an 8 bit value to port C of all 0s
    for (j = 0; j < NUMOFPWM; j++) {
        pwmArgs.arg1 = j;
        pthread_create (NULL, NULL, pwmThreadFunc, (void *)&pwmArgs);
    }
}
