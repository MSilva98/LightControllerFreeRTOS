#include "../config_bits.h"

#include "../../LightController/myMax32.h"
#include "../../UART/uart.h"

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Defines for the tasks
#define NR_ADC_SAMPLES 5
#define NR_ADC_SAMPLES 5
#define INC_DEC_VALUE 10
#define PRVALUE 519
#define MAXLDR 1000

struct QueueLightData_Type {
    uint32_t light_val;
    uint8_t on_off;
};

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
    
    
    //--------------------------------------------\\
    // CONFIGURATIONS AND VARIABLES USED IN TASKS \\
    //--------------------------------------------\\
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
    
    uint32_t ldr_values[NR_ADC_SAMPLES]; //valores que vem do LDR / ADC
    int light_int = PRVALUE/2; //Light intensity value OC1R - valor com a luz h� de ficar
    uint8_t mode = 1; //system mode
    uint8_t on_off = 1;
    uint8_t swModes_enable = 1; // 0 - disable     1 - enable
    uint8_t swOnOff_enable = 1; // 0 - disable     1 - enable

    //default values mode 3
    int intensity_light_onOff = (20*MAXLDR)/100; //intensidade a que a luz liga e desliga
    int hysteresis = (5*MAXLDR)/100;
    int light_off_3 = (0*PRVALUE)/100; //Valor quando a luz est� "desligada"
    int light_on_3 = (100*PRVALUE)/100; //Valor quando a luz est� "ligada"

    //default values mode 4
    int ligth_level = 200; //valor de intensidade pretendido
    int minL = (0*PRVALUE)/100; 
    int maxL = (100*PRVALUE)/100;
    
    uint8_t byte;
    uint8_t j = 0;
    uint8_t mesg[200];
    
    struct QueueLightData_Type QLightData;
    double avg_ldr = 0;
    uint32_t randLDR = 0;
    
    mode = 4;
    
    ldr_values[0] = 624;
    ldr_values[1] = 999;
    ldr_values[2] = 842;
    ldr_values[3] = 975;
    ldr_values[4] = 999;

    PORTCbits.RC1 = 1;
    for(i = 0; i < n; i++){
        
        T2CONbits.TON = 1;
        //------------------------------\\
        // START EXECUTION TIME MEASURE \\
        //------------------------------\\
        
        switch (mode){
            case 1:
                QLightData.light_val = PRVALUE;
//                xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
                QLightData.on_off = on_off;
//                xSemaphoreGive(xSem_on_off);
//                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 2:
//                xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                QLightData.light_val = light_int;
//                xSemaphoreGive(xSem_light_int);
//                xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
                QLightData.on_off = on_off;
//                xSemaphoreGive(xSem_on_off);
//                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 3:
                avg_ldr = 0;
//                xSemaphoreTake( xSem_ldr_values , ( TickType_t ) 10 );
                for(j = 0; j<NR_ADC_SAMPLES; j++){
                    avg_ldr += ldr_values[j]; 
                }
//                xSemaphoreGive(xSem_ldr_values);
                avg_ldr = avg_ldr/NR_ADC_SAMPLES;
                if(avg_ldr < (intensity_light_onOff - hysteresis)){
                    QLightData.light_val = light_off_3;
                    QLightData.on_off = 1;
//                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }else if(avg_ldr > (intensity_light_onOff + hysteresis)){
                    QLightData.light_val = light_on_3;
                    QLightData.on_off = 1;
//                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }
                break;
            case 4:
                avg_ldr = 0;
//                xSemaphoreTake( xSem_ldr_values , ( TickType_t ) 10 );
                for(j = 0; j<NR_ADC_SAMPLES; j++){
                    avg_ldr += ldr_values[j]; 
                }
//                xSemaphoreGive(xSem_ldr_values);
                avg_ldr = avg_ldr/NR_ADC_SAMPLES;
                if(avg_ldr < ligth_level - 20){
//                    xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                    light_int = light_int - INC_DEC_VALUE;
                    if(light_int<minL){
                        light_int = minL;
                    }
                    QLightData.light_val = light_int;
//                    xSemaphoreGive(xSem_light_int);
                    QLightData.on_off = 1;
//                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }else if(avg_ldr > ligth_level + 20){
//                    xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                    light_int = light_int + INC_DEC_VALUE;
                    if(light_int>maxL){
                        light_int = maxL;
                    }
                    QLightData.light_val = light_int;
//                    xSemaphoreGive(xSem_light_int);
                    QLightData.on_off = 1;
//                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }
                break;
        }
        
        //------------------------------\\        
        // END EXECUTION TIME MEASURE --\\
        //------------------------------\\

        T2CONbits.ON = 0;
        T3CONbits.ON = 0;
        
        
        
        vals[i] = ((double)TMR2/(double)PBCLOCK);
        tmrs[i] = TMR2;
        sprintf(msg,"Iteration: %d TMR2: %d Time: %f\n\r", i, TMR2, vals[i]);
        PrintStr(msg);
        
        // CLEAR ALL VARIABLES
        TMR2 = 0;
        IFS0bits.T2IF = 0;
        IFS0bits.T3IF = 0;
    }
    
    PORTCbits.RC1 = 0;

    // MEAN VALUE
    double mean = 0;
    for(i=0;i<n;i++){
        mean += vals[i];
    }
    
    sprintf(msg, "Mean execution time: %.10f ms\n\r", (mean*1000)/(double)n);
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