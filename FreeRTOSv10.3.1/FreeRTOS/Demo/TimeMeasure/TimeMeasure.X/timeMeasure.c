#include "../config_bits.h"

#include "../../LightController/myMax32.h"
#include "../../UART/uart.h"

#include <xc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

 
#define BILLION  1000000000.0

/* Prototypes for internal functions */
void termInit(void);

int main(int argc, char** argv)
{

    TRISAbits.TRISA3 = 0; /* Bit RA3 is output */
    /* Start with LED OFF */
    PORTAbits.RA3 = 0;
    
    // Init UART
    if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
      // LD4 will go on if there is any error
      PORTAbits.RA3 = 1;
      while (1);
    }
    
    termInit();
    
    // EXECUTION TIME MEASURE
//    clock_t begin = clock();
//
//    /* here, do your time-consuming job */
//
//    clock_t end = clock();
//    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    static uint32_t StartTime;
    static uint32_t time_spent;

    StartTime = _CP0_GET_COUNT();

// My code's elapsed time to be measured.

    time_spent = (uint32_t)( _CP0_GET_COUNT() - StartTime);
    
    printf("Time elpased is %d seconds", time_spent);
    
    
    UartClose();

    return (EXIT_SUCCESS);
}

/*
 * Terminal initialization 
 * 
 * Uses VT-100 commands to clear the monitor.
 */
void termInit(void)
{
    PutChar(0x1b);
    PutChar('[');
    PutChar('2');
    PutChar('J');

    PutChar(0x1b);
    PutChar('[');
    PutChar('H');
}