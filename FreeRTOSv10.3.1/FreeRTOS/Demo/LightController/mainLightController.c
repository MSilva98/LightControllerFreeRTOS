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

/* Periods of the tasks*/
#define SENSOR_PERIOD_MS    (100 / portTICK_RATE_MS)
#define SW_INT_PERIOD_MS    (500 / portTICK_RATE_MS)
#define BT_INT_PERIOD_MS    (150 / portTICK_RATE_MS)
#define KEY_INT_PERIOD_MS   (100 / portTICK_RATE_MS)
#define DECISION_PERIOD_MS  (80 / portTICK_RATE_MS)
//#define ACTUATION_PERIOD_MS (100 / portTICK_RATE_MS)

/* Priorities of the application tasks (high numb. -> high prio.) */
#define SENSOR_ACQ_PRIORITY (tskIDLE_PRIORITY + 2)
#define SW_INT_PRIORITY     (tskIDLE_PRIORITY + 2)
#define BT_INT_PRIORITY     (tskIDLE_PRIORITY + 2)
#define KEY_INT_PRIORITY    (tskIDLE_PRIORITY + 2)
#define DECISION_PRIORITY   (tskIDLE_PRIORITY + 3)
#define ACTUATION_PRIORITY  (tskIDLE_PRIORITY + 3)

#define NR_ADC_SAMPLES 5


uint32_t ldr_values[NR_ADC_SAMPLES]; //valores que bem do LDR / ADC
uint8_t mode = 1; //system mode
uint32_t light_int; //Light intensity value OC1R - valor com a luz há de ficar
uint8_t on_off;
uint8_t keyboard_enable; // 0 - disable     1 - enable








/*
 * Prototypes and tasks
 */


void sensorAcq(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
        
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, SENSOR_PERIOD_MS);
        
        
    }
    
}

void swInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, SW_INT_PERIOD_MS);
        
    }
    
}

void btInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, BT_INT_PERIOD_MS);
        
    }
    
}

void keyInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, KEY_INT_PERIOD_MS);
        
    }
    
}

void decision(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, DECISION_PERIOD_MS);
        
        
    }
    
}

void actuation(void *pvParam)
{
    
    
}




/*
 * Create the tasks then start the scheduler.
 */
int mainLightController( void )
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
    xTaskCreate( sensorAcq, ( const signed char * const ) "sensorAcq", configMINIMAL_STACK_SIZE, NULL, SENSOR_ACQ_PRIORITY, NULL );
    xTaskCreate( swInt, ( const signed char * const ) "swInt", configMINIMAL_STACK_SIZE, NULL, SW_INT_PRIORITY, NULL );
    xTaskCreate( btInt, ( const signed char * const ) "btInt", configMINIMAL_STACK_SIZE, NULL, BT_INT_PRIORITY, NULL );
    xTaskCreate( keyInt, ( const signed char * const ) "keyInt", configMINIMAL_STACK_SIZE, NULL, KEY_INT_PRIORITY, NULL );
    xTaskCreate( decision, ( const signed char * const ) "decision", configMINIMAL_STACK_SIZE, NULL, DECISION_PRIORITY, NULL );
    xTaskCreate( actuation, ( const signed char * const ) "actuation", configMINIMAL_STACK_SIZE, NULL, ACTUATION_PRIORITY, NULL );

        /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}
