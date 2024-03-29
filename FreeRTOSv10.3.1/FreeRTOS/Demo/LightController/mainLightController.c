
/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h" 


/* App includes */
#include "../UART/uart.h"
#include "timer2.h"
#include "myMax32.h"


/* Periods of the tasks*/
#define SENSOR_PERIOD_MS    (100 / portTICK_RATE_MS)
#define SW_INT_PERIOD_MS    (350 / portTICK_RATE_MS)
#define BT_INT_PERIOD_MS    (150 / portTICK_RATE_MS)
#define KEY_INT_PERIOD_MS   (100 / portTICK_RATE_MS)
#define DECISION_PERIOD_MS  (80 / portTICK_RATE_MS)
#define PRINTS_PERIOD_MS  (500 / portTICK_RATE_MS)

/* Priorities of the application tasks (high numb. -> high prio.) */
#define SENSOR_ACQ_PRIORITY (tskIDLE_PRIORITY + 2)
#define SW_INT_PRIORITY     (tskIDLE_PRIORITY + 2)
#define BT_INT_PRIORITY     (tskIDLE_PRIORITY + 2)
#define KEY_INT_PRIORITY    (tskIDLE_PRIORITY + 2)
#define DECISION_PRIORITY   (tskIDLE_PRIORITY + 3)
#define ACTUATION_PRIORITY  (tskIDLE_PRIORITY + 3)
#define CONFIG_PRIORITY      (tskIDLE_PRIORITY + 1)
#define PRINTS_PRIORITY      (tskIDLE_PRIORITY + 1)

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



//Semaphores

SemaphoreHandle_t xSem_ldr_values, xSem_mode, xSem_light_int, xSem_on_off, xSem_swModes_enable, xSem_swOnOff_enable  = NULL;

/*global vars*/

uint32_t ldr_values[NR_ADC_SAMPLES]; // LDR / ADC values
uint8_t mode = 1; //system mode
int light_int = PRVALUE/2; //Light intensity value OC1R 
uint8_t on_off = 1;
uint8_t swModes_enable = 1; // 0 - disable     1 - enable
uint8_t swOnOff_enable = 1; // 0 - disable     1 - enable

//default values mode 3
int intensity_light_onOff = (20*MAXLDR)/100; 
int hysteresis = (5*MAXLDR)/100;
int light_off_3 = (0*PRVALUE)/100; 
int light_on_3 = (100*PRVALUE)/100; 

//default values mode 4
int ligth_level = 200; 
int minL = (0*PRVALUE)/100; 
int maxL = (100*PRVALUE)/100;

void config(void);
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
        xSemaphoreTake( xSem_ldr_values , ( TickType_t ) 10 );
        ldr_values[i] = ADC1BUF0;
        // preemption
        xSemaphoreGive( xSem_ldr_values );
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
            xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
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
            xSemaphoreGive( xSem_mode );
        }
        if(swOnOff_enable == 1){
            xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
            on_off = SW_ON_OFF_PIN;
            xSemaphoreGive( xSem_on_off );
        }
    }
    
}

void btInt(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, BT_INT_PERIOD_MS);
        xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
        if(LEFT_BUTTON){        // increase
            light_int = light_int + INC_DEC_VALUE;
            if(light_int>PRVALUE){
                light_int = PRVALUE;
            }
        }
        if(RIGHT_BUTTON){ // decrease
            light_int = light_int - INC_DEC_VALUE;
            if(light_int<0){
                light_int = 0;
            }
        }
        xSemaphoreGive(xSem_light_int);
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
                xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
                on_off = !on_off;
                xSemaphoreGive(xSem_on_off);
            }else if(byte == 121 || byte == 89){ // y or Y
                xSemaphoreTake( xSem_swOnOff_enable , ( TickType_t ) 10 );
                swOnOff_enable = !swOnOff_enable;
                xSemaphoreGive(xSem_swOnOff_enable);
            }else if(byte == 97 || byte == 65){ //a or A
                xSemaphoreTake( xSem_swModes_enable , ( TickType_t ) 10 );
                swModes_enable = !swModes_enable;
                xSemaphoreGive(xSem_swModes_enable);
            }else if(byte == 49){ //1
                xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
                mode = 1;
                xSemaphoreGive(xSem_mode);
            }else if(byte == 50){ //2
                xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
                mode = 2;
                xSemaphoreGive(xSem_mode);
            }else if(byte == 51){ //3
                xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
                mode = 3;
                xSemaphoreGive(xSem_mode);
            }else if(byte == 52){ //4
                xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
                mode = 4;
                xSemaphoreGive(xSem_mode);
            }else if(byte == 43){ //+
                xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                light_int = light_int + INC_DEC_VALUE;
                if(light_int>PRVALUE){
                    light_int = PRVALUE;
                }
                xSemaphoreGive(xSem_light_int);
            }else if(byte == 45){ //-
                xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                light_int = light_int - INC_DEC_VALUE;
                if(light_int<0){
                    light_int = 0;
                }
                xSemaphoreGive(xSem_light_int);
            }
        }
    }
    
}

void prints(void *pvParam)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;){
        vTaskDelayUntil(&xLastWakeTime, PRINTS_PERIOD_MS);
        xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
        printf("Mode %d /", mode);
        xSemaphoreTake( xSem_swModes_enable, ( TickType_t ) 10 );
        if(swModes_enable == 1){
            printf(" SW Modes Enable");
        }else{
            printf(" SW Modes Disable");
        }
        xSemaphoreGive(xSem_swModes_enable);
        if(mode == 1 || mode == 2){
            xSemaphoreTake( xSem_swOnOff_enable , ( TickType_t ) 10 );
            if(swOnOff_enable == 1){
                printf(" / SW On_Off Enable");
            }else{
                printf(" / SW On_Off Disable");
            }
            xSemaphoreGive(xSem_swOnOff_enable);
            xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
            if(on_off == 1){
                printf(" / Light ON");
            }else{
                printf(" / Light OFF");
            }
            xSemaphoreGive(xSem_on_off);
        }
        xSemaphoreGive(xSem_mode);
        printf("\n\r");
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
        xSemaphoreTake( xSem_mode , ( TickType_t ) 10 );
        switch (mode){
            case 1:
                QLightData.light_val = PRVALUE;
                xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
                QLightData.on_off = on_off;
                xSemaphoreGive(xSem_on_off);
                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 2:
                xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                QLightData.light_val = light_int;
                xSemaphoreGive(xSem_light_int);
                xSemaphoreTake( xSem_on_off , ( TickType_t ) 10 );
                QLightData.on_off = on_off;
                xSemaphoreGive(xSem_on_off);
                xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                break;
            case 3:
                avg_ldr = 0;
                xSemaphoreTake( xSem_ldr_values , ( TickType_t ) 10 );
                for(i = 0; i<NR_ADC_SAMPLES; i++){
                    avg_ldr += ldr_values[i]; 
                }
                xSemaphoreGive(xSem_ldr_values);
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
                avg_ldr = 0;
                xSemaphoreTake( xSem_ldr_values , ( TickType_t ) 10 );
                for(i = 0; i<NR_ADC_SAMPLES; i++){
                    avg_ldr += ldr_values[i]; 
                }
                xSemaphoreGive(xSem_ldr_values);
                avg_ldr = avg_ldr/NR_ADC_SAMPLES;
                if(avg_ldr < ligth_level - 20){
                    xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                    light_int = light_int - INC_DEC_VALUE;
                    if(light_int<minL){
                        light_int = minL;
                    }
                    QLightData.light_val = light_int;
                    xSemaphoreGive(xSem_light_int);
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }else if(avg_ldr > ligth_level + 20){
                    xSemaphoreTake( xSem_light_int , ( TickType_t ) 10 );
                    light_int = light_int + INC_DEC_VALUE;
                    if(light_int>maxL){
                        light_int = maxL;
                    }
                    QLightData.light_val = light_int;
                    xSemaphoreGive(xSem_light_int);
                    QLightData.on_off = 1;
                    xQueueSend( xLightQueue,( void * ) &QLightData,( TickType_t ) 100 );
                }
                break;
        }
        xSemaphoreGive(xSem_mode);
    }
            
}

void actuation(void *pvParam)
{
    struct QueueLightData_Type QLightData;
    portBASE_TYPE xStatus;
    
    for(;;){
        xStatus=xQueueReceive(xLightQueue,(void *)&QLightData,portMAX_DELAY);
        if(QLightData.on_off == 0){
            OC1R = 0;
            LED_TRIS = 1;
        }else{
            OC1R = QLightData.light_val;
            if(QLightData.light_val == 0){
                LED_TRIS = 1;
            }else{
                LED_TRIS = 0;
            }
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
    
    timer2control(1);
    
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
    
    /*Create Semaphores*/
    xSem_ldr_values = xSemaphoreCreateMutex();
    xSem_mode = xSemaphoreCreateMutex();
    xSem_light_int = xSemaphoreCreateMutex();
    xSem_on_off = xSemaphoreCreateMutex();
    xSem_swModes_enable = xSemaphoreCreateMutex();
    xSem_swOnOff_enable = xSemaphoreCreateMutex();
    
    /* Create the tasks defined within this file. */
    xTaskCreate( sensorAcq, ( const signed char * const ) "sensorAcq", configMINIMAL_STACK_SIZE, NULL, SENSOR_ACQ_PRIORITY, NULL );
    xTaskCreate( swInt, ( const signed char * const ) "swInt", configMINIMAL_STACK_SIZE, NULL, SW_INT_PRIORITY, NULL );
    xTaskCreate( btInt, ( const signed char * const ) "btInt", configMINIMAL_STACK_SIZE, NULL, BT_INT_PRIORITY, NULL );
    xTaskCreate( keyInt, ( const signed char * const ) "keyInt", configMINIMAL_STACK_SIZE, NULL, KEY_INT_PRIORITY, NULL );
    xTaskCreate( decision, ( const signed char * const ) "decision", configMINIMAL_STACK_SIZE, NULL, DECISION_PRIORITY, NULL );
    xTaskCreate( actuation, ( const signed char * const ) "actuation", configMINIMAL_STACK_SIZE, NULL, ACTUATION_PRIORITY, NULL );
    xTaskCreate( prints, ( const signed char * const ) "prints", configMINIMAL_STACK_SIZE, NULL, PRINTS_PRIORITY, NULL );
    
    config();
        /* Finally start the scheduler. */
	//vTaskStartScheduler();

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


void config( )
{
    uint8_t mesg[50];
    sprintf(mesg, "\n\rMODE3 - Insert the level of light intensity at which light switches [0 - 100] :");
    PrintStr(mesg);
    intensity_light_onOff = (getNumber(3)*MAXLDR)/100;
    if(intensity_light_onOff > MAXLDR){
        intensity_light_onOff = MAXLDR;
    }
    sprintf( mesg, "\n\rMODE3 - Insert hysteresis percentage [0 - 100] :");
    PrintStr(mesg);
    hysteresis = (getNumber(3)*MAXLDR)/100;
    if(hysteresis > MAXLDR){
        hysteresis = MAXLDR;
    }
    sprintf(mesg, "\n\rMODE3 - Insert  light level when off [0 - 100] :");
    PrintStr(mesg);
    light_off_3 = (getNumber(3)*PRVALUE)/100;
    if(light_off_3 > PRVALUE){
        light_off_3 = PRVALUE;
    }
    sprintf(mesg, "\n\rMODE3 - Insert  light level when on [0 - 100] :");
    PrintStr(mesg);
    light_on_3 = (getNumber(3)*PRVALUE)/100;
    if(light_on_3 > PRVALUE){
        light_on_3 = PRVALUE;
    }
    sprintf(mesg, "\n\r");
    PrintStr(mesg);
    sprintf(mesg, "\n\rMODE4 - Insert  maximum light intensity[0 - 100] :");
    PrintStr(mesg);
    maxL = (getNumber(3)*PRVALUE)/100;
    if(maxL > PRVALUE){
        maxL = PRVALUE;
    }
    sprintf(mesg, "\n\rMODE4 - Insert  minimum light intensity [0 - 100] :");
    PrintStr(mesg);
    minL = (getNumber(3)*PRVALUE)/100;
    if(minL > PRVALUE){
        minL = PRVALUE;
    }
    sprintf(mesg, "\n\r");
    PrintStr(mesg);
    
    vTaskStartScheduler();
}
  