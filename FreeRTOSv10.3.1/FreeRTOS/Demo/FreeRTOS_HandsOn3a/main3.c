/*
 * Paulo Pedreiras, Apr/2020
 *
 * FREERTOS demo for ChipKit MAX32 board
 * - Creates two periodic tasks
 * - One toggles Led LD4, other is a long (interfering)task that 
 *      activates LD5 when executing 
 * - When the interfering task has higher priority interference becomes visible
 *      - LD4 does not blink at the right rate
 *
 * Environment:
 * - MPLAB 5.35
 * - XC32 V2.240
 * - FreeRTOS V10.3.1
 *
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"


/* App includes */
#include "../UART/uart.h"

/* The rate at which the LED LD5 should flash */
#define LED_FLASH_PERIOD_MS 	( 250 / portTICK_RATE_MS )
#define INTERF_TASK_PERIOD_MS 	( 3000 / portTICK_RATE_MS )
#define INTERF_LOAD_MS          ( 1000 / portTICK_RATE_MS)

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define LED_FLASH_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define INTERF_PRIORITY	    ( tskIDLE_PRIORITY + 1 )

#define ACQ_PERIOD_MS   (100 / portTICK_RATE_MS)
#define PROC_PERIOD_MS  (500 / portTICK_RATE_MS)
#define OUT_PERIOD_MS   (600 / portTICK_RATE_MS)

#define ACQ_PRIORITY    (tskIDLE_PRIORITY + 3)
#define PROC_PRIORITY   (tskIDLE_PRIORITY + 2)
#define OUT_PRIORITY    (tskIDLE_PRIORITY + 1)

#define NR_SAMPLES 5

/*
 * Prototypes and tasks
 */
uint32_t res[NR_SAMPLES];
double mean;

void pvAcq(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    uint8_t i = 0;
        
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, ACQ_PERIOD_MS);
        
        IFS1bits.AD1IF = 0; // Reset interrupt flag
        AD1CON1bits.ASAM=1; // Start conversion
        while(IFS1bits.AD1IF == 0); // Wait for EOC
        // Convert to 0..3.3V (in mV)
        res[i] = (ADC1BUF0 * 3300) / 1023;
        
        i++;
        if(i>4)
            i = 0; 
    }
    
}

void pvProc(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    uint32_t sum = 0;
    uint8_t i;
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, PROC_PERIOD_MS);
        for(i=0;i<NR_SAMPLES;i++){
            sum += res[i];
        }
        mean = sum/5.0;
        sum = 0;
    }
    
}

void pvOut(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    uint8_t mesg[80];
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, OUT_PERIOD_MS);
        
        sprintf(mesg, "Mean Voltage: %4.1f\n\r", mean);
        PrintStr(mesg);
    }
    
}
/*
 * Create the demo tasks then start the scheduler.
 */
int main3a( void )
{   
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


	// Init UART and redirect tdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
    /* Welcome message*/
    printf("Starting FreeRTOS Hands On - 3a) \n\r");
    
      
    /* Create the tasks defined within this file. */
    xTaskCreate( pvAcq, ( const signed char * const ) "Acq", configMINIMAL_STACK_SIZE, NULL, ACQ_PRIORITY, NULL );
    xTaskCreate( pvProc, ( const signed char * const ) "Proc", configMINIMAL_STACK_SIZE, NULL, PROC_PRIORITY, NULL );
    xTaskCreate( pvOut, ( const signed char * const ) "Out", configMINIMAL_STACK_SIZE, NULL, OUT_PRIORITY, NULL );

        /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}
