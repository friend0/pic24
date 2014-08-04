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


unsigned int Get_AD(int channel){
    if((channel > 0) & (channel < NUM_ANALOG))
    {
        return ADC_Values[channel];
    }
    else{
        return -1;
    }

}



