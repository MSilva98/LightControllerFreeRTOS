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
#include "queue.h"


/* App includes */
#include "../UART/uart.h"
#include "timer2.h"
#include "myMax32.h"


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
#define INC_DEC_VALUE 10
#define PRVALUE 519
#define MAXLDR 1000


/* Queue defs, structures and vars */
#define lightQueueQUEUE_LEN         (1)        // Number of elements of queue

static QueueHandle_t xLightQueue = NULL;

struct QueueLightData_Type {
    uint32_t light_val;
    uint8_t on_off;
};



/*global vars*/

uint32_t ldr_values[NR_ADC_SAMPLES]; //valores que vem do LDR / ADC
uint8_t mode = 1; //system mode
uint32_t light_int; //Light intensity value OC1R - valor com a luz há de ficar
uint8_t on_off;
uint8_t swModes_enable; // 0 - disable     1 - enable
uint8_t swOnOff_enable; // 0 - disable     1 - enable

//default values mode 3
int intensity_light_onOff = (20*MAXLDR)/100; //intensidade a que a luz liga e desliga
int hysteresis = (5*MAXLDR)/100;
int light_off_3 = (0*PRVALUE)/100; //Valor quando a luz está "desligada"
int light_on_3 = (100*PRVALUE)/100; //Valor quando a luz está "ligada"

//default values mode 4
int ligth_level = 200; //valor de intensidade pretendido
int maxL = (0*PRVALUE)/100; 
int minL = (100*PRVALUE)/100;


/*
 * Prototypes and tasks
 */


void sensorAcq(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    uint8_t i = 0;
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, SENSOR_PERIOD_MS);
        
        IFS1bits.AD1IF = 0; // Reset interrupt flag
        AD1CON1bits.ASAM=1; // Start conversion
        while(IFS1bits.AD1IF == 0); // Wait for EOC
        // no_preemption
        ldr_values[i] = ADC1BUF0;
        // preemption
        i++;
        if(i>NR_ADC_SAMPLES)
            i = 0;
            
    }
    
}

void swInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, SW_INT_PERIOD_MS);
        if(swModes_enable){
            // no_preemption
            switch(10*MODE_SW_1 + MODE_SW_0){
                case 0:
                    mode = 1;
                    break;
                    
                case 1:
                    mode = 2;
                    break;
                    
                case 10:
                    mode = 3;
                    break;
                    
                case 11:
                    mode = 4;
                    break;
                    
                default:
                    break;
            }
            // preemption
        }
        if(swOnOff_enable == 1){
            // no_preemption
            on_off = SW_ON_OFF_PIN;
            // preemption
        }
    }
    
}

void btInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, BT_INT_PERIOD_MS);
        if(LEFT_BUTTON){        // increase
            // no_preemption
            light_int = light_int + INC_DEC_VALUE;
            if(light_int>PRVALUE){
                light_int = PRVALUE;
            }
            // preemption
        }
        if(RIGHT_BUTTON){ // decrease
            // no_preemption
            light_int = light_int - INC_DEC_VALUE;
            if(light_int<1){
                light_int = 1;
            }
            // preemption
        }
    }
    
}

void keyInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    uint8_t byte;
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, KEY_INT_PERIOD_MS);
        
        if(GetChar( &byte ) == UART_SUCCESS){
            if(byte == 116 || byte == 84){ //t or T
                if(on_off == 0){
                    on_off = 1;
                }else{
                    on_off = 0;
                }
            }else if(byte == 121 || byte == 89){ // y or Y
                if(swOnOff_enable == 0){
                    swOnOff_enable = 1;
                }else{
                    swOnOff_enable = 0;
                }
            }else if(byte == 99 || byte == 67){ //C or c
                //config
            }else if(byte == 97 || byte == 65){ //a or A
                if(swModes_enable == 0){
                    swModes_enable = 1;
                }else{
                    swModes_enable = 0;
                }
            }else if(byte == 49){ //1
                mode = 1;
            }else if(byte == 50){ //2
                mode = 2;
            }else if(byte == 51){ //3
                mode = 3;
            }else if(byte == 52){ //4
                mode = 4;
            }else if(byte == 43){ //+
                light_int = light_int + INC_DEC_VALUE;
                if(light_int>PRVALUE){
                    light_int = PRVALUE;
                }
            }else if(byte == 45){ //-
                light_int = light_int - INC_DEC_VALUE;
                if(light_int<1){
                    light_int = 1;
                }
            }
        }
    }
    
}

void decision(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    struct QueueLightData_Type QLightData;
    int i;
    double avg_ldr = 0;
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, DECISION_PERIOD_MS);
        switch (mode){
            case 1:
                QLightData.light_val = light_int;
                QLightData.on_off = on_off;
                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 2:
                QLightData.light_val = light_int;
                QLightData.on_off = on_off;
                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 3:
                for(i = 0; i<NR_ADC_SAMPLES; i++){
                    avg_ldr += ldr_values[i]; 
                }
                avg_ldr = avg_ldr/NR_ADC_SAMPLES;
                if(avg_ldr < (intensity_light_onOff - hysteresis)){
                    QLightData.light_val = light_off_3;
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }else if(avg_ldr > (intensity_light_onOff + hysteresis)){
                    QLightData.light_val = light_on_3;
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }
                break;
            case 4:
                for(i = 0; i<NR_ADC_SAMPLES; i++){
                    avg_ldr += ldr_values[i]; 
                }
                avg_ldr = avg_ldr/NR_ADC_SAMPLES;
                if(avg_ldr < ligth_level - 20){
                    QLightData.light_val = light_int + 15;
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }else if(avg_ldr > ligth_level + 20){
                    QLightData.light_val = light_int - 15;
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }
                break;
        }
    }
}

void actuation(void *pvParam)
{
    struct QueueLightData_Type QLightData;
    portBASE_TYPE xStatus;
    
    for(;;){
        xStatus=xQueueReceive(xLightQueue,(void *)&QLightData,portMAX_DELAY);
        if(QLightData.on_off = 0){
            OC1RS = 0;
            LED_TRIS = 1;
        }else{
            OC1RS = QLightData.light_val;
        }
    }
}




/*
 * Create the tasks then start the scheduler.
 */
int mainLightController( void )
{   
    
    LED_TRIS = 0;
    SW_ON_OFF_TRIS = 1;
    TRIS_SW_0 = 1;
    TRIS_SW_1 = 1;
    LEFT_BUTTON_TRIS = 1;
    RIGHT_BUTTON_TRIS = 1;
    
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
    
    
    set_timer2_int_freq(256, 300);
    void __attribute__( (interrupt(IPL2AUTO), vector(_TIMER_2_VECTOR))) visr_tmr2(void);

    
    //OC1 config
    
    IPC1bits.OC1IP=2; //set interrupt priority to 2
    IEC0bits.OC1IE = 1; // Enable OC1 interrupts
    
    OC1CONbits.OCM = 3; // Operation mode
    OC1CONbits.OCTSEL=0; // Select clock source T2/T3 
    OC1CONbits.ON=1;// Timer on
    IFS0bits.OC1IF=0;
    
    void __attribute__( (interrupt(IPL2AUTO), vector(_OUTPUT_COMPARE_1_VECTOR))) visr_oc1(void);
    
	// Init UART and redirect tdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
    /* Welcome message*/
    printf("Starting Light Controller \n\r");
    
    /* Create Queues*/
    xLightQueue = xQueueCreate(lightQueueQUEUE_LEN, sizeof(struct QueueLightData_Type));  
    
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

void visr_tmr2(void){
    LED_PIN = 1;
    IFS0bits.T2IF = 0;

}

void visr_oc1(void){
    LED_PIN = 0;
    IFS0bits.OC1IF=0;
    OC1CONbits.ON=1;
}