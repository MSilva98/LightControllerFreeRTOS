/* 
 * File:   myMax32.h
 * Author: Diogo & Marco
 *
 * Created on 2 de Dezembro de 2020, 18:20
 */

#define SYSCLK  80000000L // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz

#define SW_ON_OFF_PIN   PORTCbits.RC3
#define LED_PIN         PORTCbits.RC2
#define LEFT_BUTTON     PORTEbits.RE1
#define RIGHT_BUTTON    PORTEbits.RE0
#define MODE_SW_1       PORTFbits.RF1
#define MODE_SW_0       PORTFbits.RF0
