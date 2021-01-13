/*
 * timer2.c
 * 
 * Módulo para controlo do Timer2
 */

#include <xc.h>
#include "timer2.h"

#define PBCLKfreq   40000000L


int8_t set_timer2_int_freq(uint16_t Prescaler, uint32_t fout)
{
    uint32_t    PR2val;
    int8_t result;
    
    PR2val = (PBCLKfreq/(Prescaler*fout))-1;
    
    if(PR2val>65535){
        return -2; // -2 código erro: frequência impossível de gerar
                    // TODO: criar código de erro
    }
    
    
    result = config_timer2_int(Prescaler, (uint16_t) PR2val);
    
    return result;
    
}


int8_t set_timer2_freq(uint16_t Prescaler, uint32_t fout)
{
    uint32_t    PR2val;
    int8_t result;
    
    PR2val = (PBCLKfreq/(Prescaler*fout))-1;
    
    if(PR2val>65535){
        return -2; // -2 código erro: frequência impossível de gerar
                    // TODO: criar código de erro
    }
    
    
    result = config_timer2(Prescaler, (uint16_t) PR2val);
    
    return result;
    
}

int8_t config_timer2_int(uint16_t Prescaler, uint16_t ValPR2)
{   
    uint8_t TCKPSval;
    
    switch(Prescaler){
        case 1:
            TCKPSval = 0;
            break;
        case 2:
            TCKPSval = 1;
            break;
        case 4:
            TCKPSval = 2;
            break;
        case 8:
            TCKPSval = 3;
            break;
        case 16:
            TCKPSval = 4;
            break;
        case 32:
            TCKPSval = 5;
            break;
        case 64:
            TCKPSval = 6;
            break;
        case 256:
            TCKPSval = 7;
            break;
        default:
            return -1;  // -1: error: impossible prescaler value 
                        // TODO: create error codes
    }
    
    INTCONSET=_INTCON_MVEC_MASK;
    
    T2CONbits.ON = 0;
    TMR2 = 0;
    
    T2CONbits.TGATE = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = TCKPSval;
    PR2 = ValPR2;
    
    T2CONbits.T32 = 0; // 32 bit timer operation
    
    IFS0bits.T2IF=0;  // Reset the interrupt flag 
    IPC2bits.T2IP=2;//set interrupt priority (1..7)
    IEC0bits.T2IE=1;// Enable T2 interrupts
    
    return 0;
    
}

int8_t config_timer2(uint16_t Prescaler, uint16_t ValPR2)
{   
    uint8_t TCKPSval;
    
    switch(Prescaler){
        case 1:
            TCKPSval = 0;
            break;
        case 2:
            TCKPSval = 1;
            break;
        case 4:
            TCKPSval = 2;
            break;
        case 8:
            TCKPSval = 3;
            break;
        case 16:
            TCKPSval = 4;
            break;
        case 32:
            TCKPSval = 5;
            break;
        case 64:
            TCKPSval = 6;
            break;
        case 256:
            TCKPSval = 7;
            break;
        default:
            return -1;  // -1: error: impossible prescaler value 
                        // TODO: create error codes
    }
    
    T2CONbits.ON = 0;
    TMR2 = 0;
    
    T2CONbits.TGATE = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = TCKPSval;
    PR2 = ValPR2;
    
    return 0;
    
}

void timer2control(uint8_t trun)
{
    if(trun)
    {
        T2CONbits.ON = 1;   // Timer is ON
    }
    else
    {
        T2CONbits.ON = 0;   // Timer is OFF
    }   
}