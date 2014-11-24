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
#include "cbuff.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define LEDB _LATB6
#define FILTER_DEPTH 64
#define SHIFT_ITERATION 6

#define AZIMUTH_INDEX 0
#define X_INDEX 1
#define Y_INDEX 2

/*******************************************************************************
 * PRIVATE VARIABLES                                                            *
 ******************************************************************************/
static unsigned short azArray[FILTER_DEPTH] = {[0 ... FILTER_DEPTH-1] = 0};
static unsigned short xArray[FILTER_DEPTH] = {[0 ... FILTER_DEPTH-1] = 0};
static unsigned short yArray[FILTER_DEPTH] = {[0 ... FILTER_DEPTH-1] = 0};
static unsigned short *shifterAz, *shifterX, *shifterY;

volatile unsigned int ADC_Values[NUM_ANALOG * 2] = {0, 0, 0, 0, 0, 0}, temp;
volatile unsigned int ADC16_Values[NUM_ANALOG] = {0, 0, 0};
volatile unsigned int ADC16_Filtered[NUM_ANALOG] = {0, 0, 0};
volatile unsigned int Last_ADC16_Filtered[NUM_ANALOG] = {0, 0, 0};

unsigned char buff[12] = {[0 ... 12-1] = 0};
cbuff_t adcBuffer;


/*******************************************************************************
 * FUNCTION IMPLEMENTATIONS                                                          *
 ******************************************************************************/
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

    //set up a queue for ADC change events
    cbuff_init(&adcBuffer, buff, 12);

    
}

unsigned int Get_AD(short int channel) {
    if ((channel >= 0) && (channel < NUM_ANALOG * 2)) {
        return ADC_Values[channel];
    } else {
        return -1;
    }
    return -1;
}

unsigned int Get_AD16(short int channel){
    if ((channel >= 0) && (channel < NUM_ANALOG)) {
        return ADC16_Values[channel];
    } else {
        return -1;
    }
    return -1;
}

unsigned int Get_Filtered_AD16(short int channel){
    if ((channel >= 0) && (channel < NUM_ANALOG)) {
        return ADC16_Filtered[channel];
    } else {
        return -1;
    }
    return -1;
}

unsigned int Get_Last_Filtered_AD16(short int channel){
    if ((channel >= 0) && (channel < NUM_ANALOG)) {
        return Last_ADC16_Filtered[channel];
    } else {
        return -1;
    }
    return -1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {

    IFS0bits.T3IF = 0; //TMR3 interrupt flag must be cleared in software
    LEDB = ~LEDB;
    while (!AD1CON1bits.DONE); // wait to complete the conversion
    int ADC16_VAL = 0, ADC_VAL_LSB = 0, ADC_VAL_MSB = 0;
    //load new analog values into memory; take an average of samples
    //Think about implementing the same filter as on pic32 end?
    temp = (ADC1BUF0 + ADC1BUF4 + ADC1BUF8 + ADC1BUFC) >> 2;
    //ADC_Values[6] = (temp & 0x00FF);
        ADC_VAL_LSB = (temp & 0x00FF);

    //ADC_Values[7] = ((temp & 0xFF00) >> 8);
    ADC_VAL_MSB = ((temp & 0xFF00) >> 8);

    ADC16_VAL = ((ADC_VAL_MSB << 8) | ADC_VAL_LSB );

    //Get the values at each of the 4 ADC bufs per channel
    //Take the average, load into ADC_16 value array

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


    filter();
    eventCheck();

    Last_ADC16_Filtered[0] = ADC16_Filtered[0];
    Last_ADC16_Filtered[1] = ADC16_Filtered[1];
    Last_ADC16_Filtered[2] = ADC16_Filtered[2];

    //_SWDTEN = 0;
}

void filter(){
    unsigned int sumAz = 0, sumX = 0, sumY = 0, temp = 0, count = 0, i = 0;
    for (i = 1; i < FILTER_DEPTH; i++) {
        count++;
        shifterAz = &azArray[(FILTER_DEPTH - i - 1)];
        temp = *shifterAz;
        *(shifterAz + 1) = temp;
        sumAz = sumAz + temp;

        shifterX = &xArray[(FILTER_DEPTH - i - 1)];
        temp = *shifterX;
        *(shifterX + 1) = temp;
        sumX = sumX + temp;

        shifterY = &yArray[(FILTER_DEPTH - i - 1)];
        temp = *shifterY;
        *(shifterY + 1) = temp;
        sumY = sumY + temp;

        if (i == (FILTER_DEPTH - 1)) {
            azArray[0] = ADC16_Values[0];
            xArray[0] = ADC16_Values[1];
            yArray[0] = ADC16_Values[2];

            sumAz = sumAz + azArray[0];
            sumX = sumX + xArray[0];
            sumY = sumY + yArray[0];
        }

        //give 87.5% weight to past input(last avg), 12.5% to the current input(current avg)
        ADC16_Filtered[0] = Last_ADC16_Filtered[0] - (Last_ADC16_Filtered[0] >> 3) + ((sumAz >> SHIFT_ITERATION) >> 3);
        ADC16_Filtered[1] = Last_ADC16_Filtered[1] - (Last_ADC16_Filtered[1] >> 3) + ((sumX >> SHIFT_ITERATION) >> 3);
        ADC16_Filtered[2] = Last_ADC16_Filtered[2] - (Last_ADC16_Filtered[2] >> 3) + ((sumY >> SHIFT_ITERATION) >> 3);
    }
}

void eventCheck(void){

    if(Last_ADC16_Filtered[AZIMUTH_INDEX] != ADC16_Filtered[AZIMUTH_INDEX]){
    cbuff_enqueue(&adcBuffer, (unsigned char *) 1 , 1);
    }
    if(Last_ADC16_Filtered[X_INDEX] != ADC16_Filtered[X_INDEX]){
    cbuff_enqueue(&adcBuffer, (unsigned char *) 1 , 1);
    }
    if(Last_ADC16_Filtered[Y_INDEX] != ADC16_Filtered[Y_INDEX]){
    cbuff_enqueue(&adcBuffer, (unsigned char *) 1 , 1);
    }
}




