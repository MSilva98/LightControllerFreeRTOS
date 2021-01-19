#include "../config_bits.h"

#include "../../LightController/myMax32.h"
#include "../../UART/uart.h"

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
 
#define NR_ADC_SAMPLES 5
    
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
    
    uint32_t n = 200;       // NUMBER OF RUNS
    double vals[n];
    uint32_t tmrs[n];
    uint32_t i = 0;
    char msg[200];
    
    // ADC config.
    AD1CON1bits.SSRC=7; // Internal counter ends sampling and starts conversion
    AD1CON1bits.CLRASAM=1; //Stop conversion when 1st A/D converter interrupt is
    // generated and clear ASAM bit automatically
    AD1CON1bits.FORM=0; // Integer 16 bit output format
    AD1CON2bits.VCFG=0; // VR+=AVdd; VR-=AVss
    AD1CON2bits.SMPI=0; // Number (+1) of consecutive coversions, stored in buffer
    AD1CON3bits.ADRC=1; // ADC uses internal RC clock
    AD1CON3bits.SAMC=16; // Sample time is 16TAD ( TAD = 100ns)
    // Set AN0 as input
    AD1CHSbits.CH0SA=0; // Select AN0 as input for A/D converter
    TRISBbits.TRISB0=1; // AN0 in input mode
    AD1PCFGbits.PCFG0=0; // AN0 as analog input
    // Enable module
    AD1CON1bits.ON = 1; // Enable A/D module
    
    uint8_t j = 0;
    uint32_t ldr_values[NR_ADC_SAMPLES]; //valores que vem do LDR / ADC
    
    for(i = 0; i < n; i++){
        PORTCbits.RC1 = 1;
        T2CONbits.TON = 1;
        
        //------------------------------\\
        // START EXECUTION TIME MEASURE \\
        //------------------------------\\
        
        IFS1bits.AD1IF = 0; // Reset interrupt flag
        AD1CON1bits.ASAM=1; // Start conversion
        while(IFS1bits.AD1IF == 0); // Wait for EOC
        ldr_values[j] = ADC1BUF0;
        j++;
        if(j>NR_ADC_SAMPLES)
            j = 0;
        
        //------------------------------\\        
        // END EXECUTION TIME MEASURE --\\
        //------------------------------\\

        T2CONbits.ON = 0;
        T3CONbits.ON = 0;
        
        vals[i] = ((double)TMR2/(double)PBCLOCK)/25.0;
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
    
    sprintf(msg, "Mean execution time: %.10f ms\n\r", (mean*1000000)/(double)n);
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