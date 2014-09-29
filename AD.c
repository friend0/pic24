/*
 * File:   AD.c
 * Author: mdunne
 *
 * Created on November 22, 2011, 8:57 AM
 */

#include <xc.h>
#include "adc.h"
#include "I2.h"
#include "PIC24.h"
#include "AD.h"
#include "timer.h"
#include "uart.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

//#define AD_DEBUG_VERBOSE

//#define AD_DEBUG_VERBOSE
#ifdef AD_DEBUG_VERBOSE
#include "serial.h"
#define dbprintf(...) while(!IsTransmitEmpty()); printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

/*******************************************************************************
 * PRIVATE VARIABLES                                                            *
 ******************************************************************************/
//static unsigned int ActivePins;
//static unsigned int PinsToAdd;
//static unsigned int PinsToRemove;
//static unsigned int PinCount;
//static char ADActive;
//static char ADNewData = FALSE;

/*******************************************************************************
 * PRIVATE VARIABLES                                                            *
 ******************************************************************************/
//static unsigned int ActivePins;
//static unsigned int PinsToAdd;
//static unsigned int PinsToRemove;
//static unsigned int PinCount;
//static char ADActive;
//static char ADNewData = FALSE;

volatile unsigned int ADC_Values[NUM_ANALOG * 2] = {0, 0, 0, 0, 0, 0, 0, 0}, temp;
volatile unsigned int ADC16_Values[NUM_ANALOG] = {0, 0, 0};


void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0; //TMR3 interrupt flag must be cleared in software
    //_SWDTEN = 1;
    //ClrWdt();
    TRISBbits.TRISB7 = ~TRISBbits.TRISB7; //Toggle LED

    while (!AD1CON1bits.DONE); // wait to complete the conversion



    //load new analog values into memory; take an average of samples
    //Think about implementing the same filter as on pic32 end?
    //temp = (ADC1BUF0 + ADC1BUF4 + ADC1BUF8 + ADC1BUFC) >> 2;
    //ADC_Values[6] = (temp & 0x00FF);
    //ADC_Values[7] = ((temp & 0xFF00) >> 8);

    //ADC16_Values[3] = ((ADC_Values[7] << 8) | ADC_Values[6] );

    temp = (ADC1BUF1 + ADC1BUF5 + ADC1BUF9 + ADC1BUFD) >> 2;
    ADC_Values[0] = (temp & 0x00FF);
    ADC_Values[1] = ((temp & 0xFF00) >> 8);

    ADC16_Values[0] = ((ADC_Values[1] << 8) | ADC_Values[0] );


    temp = (ADC1BUF2 + ADC1BUF6 + ADC1BUFA + ADC1BUFE) >> 2;
    ADC_Values[2] = (temp & 0x00FF);
    ADC_Values[3] = ((temp & 0xFF00) >> 8);

    ADC16_Values[1] = ((ADC_Values[3] << 8) | ADC_Values[2] );


    temp = (ADC1BUF3 + ADC1BUF7 + ADC1BUFB + ADC1BUFF) >> 2;
    ADC_Values[4] = (temp & 0x00FF);
    ADC_Values[5] = ((temp & 0xFF00) >> 8);

    ADC16_Values[2] = ((ADC_Values[5] << 8) | ADC_Values[4] );

    TRISBbits.TRISB7 = ~TRISBbits.TRISB7; //Toggle LED

    //_SWDTEN = 0;
}

unsigned int Get_AD(short int channel) {
    if ((channel >= 0) & (channel < NUM_ANALOG * 2)) {
        return ADC_Values[channel];
    } else {
        return -1;
    }
    return -1;
}

unsigned int Get_AD16(short int channel){
    if ((channel >= 0) & (channel < NUM_ANALOG)) {
        return ADC16_Values[channel];
    } else {
        return -1;
    }
    return -1;
}
void AD_Init(void) {

    //AD1CON3 = 0x1F02; // max sample time = 31Tad, Tad = 2 x Tcy = 125ns >75ns
    AD1CON3 = 0x1F02;
    AD1CON2 = 0; // use MUXA, AVss and AVdd are used as Vref+/-
    AD1CON1bits.SSRC = 2; // set interrupt source to tmr3
    AD1CON1bits.SIMSAM = 1; //simultaneous sampling on
    AD1CON1bits.FORM = 0; //output form is unsigned int
    AD1CON1bits.ASAM = 1; //Auto Sampling On
    AD1CON2bits.CHPS = 2; //Multi-Channel (0-3)
    AD1CON2bits.SMPI = 3; //Samples per interrupt;
    AD1CON2bits.BUFM = 0; //Set buffer load mode to use all 16 bufs
    AD1CON3bits.ADRC = 0; //ADC clock is system clock;
    AD1CON1bits.ADON = 1; // turn on the ADC;

    AD1CHS0bits.CH0SA = 3; // Select AN3 for CH0 +ve input
    AD1CHS0bits.CH0NA = 0; // Select AVss for CH0 -ve input

    AD1CHS123bits.CH123SA = 0; // Select AN0 for CH1 +ve input
    // Select AN1 for CH2 +ve input
    // Select AN2 for CH3 +ve input
    AD1CHS123bits.CH123NA = 0; // Select AVss for CH1/CH2/CH3 -ve inputs
    ANSELAbits.ANSA0 = 1; //Set An0, An1, An2, An3 as inputs
    ANSELAbits.ANSA1 = 1;
    ANSELBbits.ANSB0 = 1;
    ANSELBbits.ANSB1 = 1;
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;

}



