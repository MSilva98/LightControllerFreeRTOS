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
    TRISCbits.TRISC1 = 0;
    /* Start with LED OFF */
    PORTAbits.RA3 = 0;
    
    // Init UART
    if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
      // LD4 will go on if there is any error
      PORTAbits.RA3 = 1;
      while (1);
    }
    
    termInit();
    
    T2CONbits.ON = 0;
    T3CONbits.ON = 0;
    T2CONbits.TCS = 0;      // INTERNAL CLOCK - TIME
    T2CONbits.T32 = 1;
    T2CONbits.TCKPS = 0;    // NO PRESCALER
    TMR2 = 0;
    PR2 = 0xFFFFFFFF;
    IFS0bits.T2IF = 0;
    IFS0bits.T3IF = 0; 
    
    uint32_t n = 100;       // NUMBER OF RUNS
    double vals[n];
    uint32_t tmrs[n];
    uint32_t i = 0;
    char msg[200];
    
    sprintf(msg, "TIMER CONFIGURATION COMPLETE\n\r");
    PrintStr(msg);
    
    for(i = 0; i < n; i++){
        PORTCbits.RC1 = 1;
        T2CONbits.TON = 1;
        // START EXECUTION TIME MEASURE
        
        sprintf(msg, "TESTING EXECUTION TIME\n\r");
        PrintStr(msg);
        double d = 247821.312;
        double x = 34252.155;
        double y = 1425.2342;
        
        double t = d*x;
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        t = x*x*x*y;
        sprintf(msg, "RESULT %f\n\r", t);
        PrintStr(msg);
        
        // END EXECUTION TIME MEASURE
        T2CONbits.ON = 0;
        T3CONbits.ON = 0;
        
        vals[i] = (double)TMR2/(double)PBCLOCK;
        tmrs[i] = TMR2;
        sprintf(msg,"Iteration: %d TMR2: %d Time: %f\n\r", i, TMR2, vals[i]);
        PrintStr(msg);
        // CLEAR ALL VARIABLES
        TMR2 = 0;
        IFS0bits.T2IF = 0;
        IFS0bits.T3IF = 0;
        PORTCbits.RC1 = 0;
    }
    
    
    // MEAN VALUE
    double mean = 0;
    for(i=0;i<n;i++){
        mean += vals[i];
    }
    
    sprintf(msg, "Mean execution time: %f\n\r", mean/(double)n);
    PrintStr(msg);
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