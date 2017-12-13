#include "timer.h"


int pwmRunning = 1;
uintptr_t ctrlHandle;
uintptr_t portA;


typedef struct {
    int servoNum;
	} PWMArgs;

int duty_cycles[NUMOFPWM] = { DEFAULT_POSITION, DEFAULT_POSITION };

/*
 * Thread to do the PWM stuff.
 */
void *pwmThread(void *args){
    ThreadCtl(_NTO_TCTL_IO, NULL);

	uintptr_t pwm;
	int servoID;

    struct timespec startTime, finishTime, timeSpent, totalTime, sleepTime;

	PWMArgs *arg;
	arg = args;
    servoID = arg -> servoNum;

	if(servoID == 0){
		pwm = mmap_device_io(IO_PORT_SIZE, BASE + 8);
	}
	else{
		pwm = mmap_device_io(IO_PORT_SIZE, BASE + 9);
	}

    totalTime.tv_nsec = PERIOD;

	while (pwmRunning) {    // When pwm starts to run..
	    clock_gettime(CLOCK_REALTIME, &startTime);
	    out8(pwm, HIGH);
	    sleepTime.tv_nsec = duty_cycles[servoID];
	    nanospin( &sleepTime);

	    out8(pwm, LOW);
	    clock_gettime(CLOCK_REALTIME, &finishTime);
	    timeSpent.tv_nsec = finishTime.tv_nsec - startTime.tv_nsec;
	    sleepTime.tv_nsec = totalTime.tv_nsec - timeSpent.tv_nsec;
	    clock_nanosleep(CLOCK_REALTIME, 0, &sleepTime, NULL);
	}

    out8(pwm, Low);
    return NULL;
}

void Change_Width( int pulseWidth, int servoNum ){
	duty_cycles[servoNum] = pulseWidth;

}

void msSleep(double milliseconds, int servoNum){

    if(servoNum == 0){
    	struct timespec req0 = {0};
        //printf("servo%d sleeping %f\n", servoNum, milliseconds);
    	req0.tv_sec = 0;
    	req0.tv_nsec = milliseconds;
    	nanospin_ns(req0.tv_nsec);
    }
    else{
    	struct timespec req1 = {0};
        //printf("servo%d sleeping %f\n", servoNum, milliseconds);
    	req1.tv_sec = 0;
    	req1.tv_nsec = milliseconds;
    	nanospin_ns(req1.tv_nsec);
    }
}

/*
 * Initializes pwm and does root access request.
 * Also sets up memory space
 */
void pwmInit(){
    int i;
    PWMArgs pwmArgs;

    // see http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Ft%2Fthreadctl.html
    // allows the thread to access lower level I/O and memory privledges
    ThreadCtl(_NTO_TCTL_IO, NULL);

    // gets access to device register.  Reg. length is 8, and want to use Digital I/O port C

    // set register I/O settings
    uintptr_t ctrl = mmap_device_io(8, BASE + 10);


    if(ctrl == MAP_DEVICE_FAILED)
    {
        perror("map control register fails");
    }

    out8(ctrl, LOW);                             // write an 8 bit value to port C of all 0s

    for (i = 0; i < NUMOFPWM; i++){
        pwmArgs.servoNum = i;
        pthread_create (NULL, NULL, pwmThread, (void *)&pwmArgs);
    }
}
