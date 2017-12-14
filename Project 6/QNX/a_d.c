#include <stdio.h>
#include <stdlib.h>       
#include <stdint.h>       
#include <hw/inout.h>     
#include <sys/neutrino.h> 
#include <sys/syspage.h>  
#include <sys/mman.h>     
#include <unistd.h>       
#include <math.h>

#define IO_PORT_SIZE                (1)
#define BASE_ADDRESS                (0x280) // QNX base address
#define A_D_MSB                     (BASE_ADDRESS+1)
#define A_D_INPUT_CHANNEL           (BASE_ADDRESS+2)
#define A_D_GAIN_STATUS             (BASE_ADDRESS+3)
#define INTRPT_DMA_CONTROL_COUNTER  (BASE_ADDRESS+4)
#define FIFO_THRESHOLD              (BASE_ADDRESS+5)
#define D_A_LSB                     (BASE_ADDRESS+6)
#define D_A_MSB                     (BASE_ADDRESS+7)
#define PORT_A_OUT                  (BASE_ADDRESS+8)
#define PORT_B_OUT                  (BASE_ADDRESS+9)
#define PORT_C_OUT                  (BASE_ADDRESS+0xA)
#define DIRECTION_CONTROL           (BASE_ADDRESS+0xB)
#define COUNTER_TIMER_B0            (BASE_ADDRESS+0xC)
#define COUNTER_TIMER_B1            (BASE_ADDRESS+0xD)
#define COUNTER_TIMER_B2            (BASE_ADDRESS+0xE)
#define COUNTER_TIMER_CONTROL       (BASE_ADDRESS+0xF)

uintptr_t baseAddress;

uintptr_t adChannelAddress;
uintptr_t adMSBAddress;
uintptr_t adGainAddress;

uintptr_t dioPortA_Address;
uintptr_t dioDirectionAddress;

void request_io_access(void){
    if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1){
        perror("Failed to get I/O access permission");
    }
}

/*
 * Maps the IO ports to make them available to use.  Need all ports associated with A_D,
 * Port A for output, and the direction control register
*/
void map_ports(void){
    baseAddress = mmap_device_io(IO_PORT_SIZE, BASE_ADDRESS);
    if(baseAddress == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }

    adMSBAddress = mmap_device_io(IO_PORT_SIZE, A_D_MSB);
    if(adMSBAddress == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }

    adChannelAddress = mmap_device_io(IO_PORT_SIZE, A_D_INPUT_CHANNEL);
    if(adChannelAddress == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }

    adGainAddress = mmap_device_io(IO_PORT_SIZE, A_D_GAIN_STATUS);
    if(adGainAddress == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }

    dioPortA_Address = mmap_device_io(IO_PORT_SIZE, PORT_A_OUT);
    if(dioPortA_Address == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }

    dioDirectionAddress = mmap_device_io(IO_PORT_SIZE, DIRECTION_CONTROL);
    if(dioDirectionAddress == MAP_DEVICE_FAILED){
        perror("Failed to map base address");
    }
}

/*
 * Initializes the A_D converter by first setting the input channel and range, then resetting to 0 and
 * setting the direction
 * Note that settings come from the manual on the course website
*/
void ad_Init(void){
    //select the input channel
    out8(adChannelAddress,0x44); // Vin4

    //Select the input range
    out8(adGainAddress,0x01) // +-5V bipolar

    // Reset fifo depth to 0
    out8(baseAddress,0x10); 

    // set IO direction
    out8(dioDirectionAddress,0x00);
}

double ad_convert(void){
    int LSB, MSB = 0
    unsigned ad_value; // unsigned int since never negative
    double voltage = 0.0;

    // need to wait for A/D to settle to 0
    while (in8(adGainAddress) & 0x20){
        ; // do nothing
    }

    // Begin conversion
    out8(baseAddress,0x80);

    // Wait for conversion to finish
    while (in8(adGainAddress) & 0x80){
        ; // status 1 in progress, 0 idle
    }

    // get the ADC values from its register
    LSB = in8(baseAddress);
    MSB = in8(adMSBAddress);
    ad_value = (MSB*256)+LSB;
    /*
     * convert to voltage. conversion from 
     * http://www.se.rit.edu/~swen-563/resources/helios/Helios%20User%20Manual.pdf
     * section 16.9.1.  Note that the 'full-scale input voltage' magnitude would be 5V
    */

    voltage = ((double)ad_value/32768.0)*5.0;
    printf("%f\n",voltage);

    return voltage; //-5V to 5V
}

int scale_converted_signal(double voltage_post_conversion){
    return round((voltage_post_conversion+5.0)*25.5); //O to 255 scaled for output -> STM eq. rounded(-0.0667*scaled_sig)+21.242
}

void output_stm(int scaled_signal){
    out8(dioPortA_Address,scaled_signal)
}
