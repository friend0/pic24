/*
 * File:   AD.h
 * Author: rrodri
 *
 * Software module to enable the Analog to Digital converter of the Uno32 boards.
 * All analog pins are are Port V and Port W. BatVolt removed.
 *
 * NOTE: Analog pins automatically take over digital I/O regardless of which TRIS
 *       it is in. There remains an error in the ADC code such that if all 12
 *       pins are enabled, one of them does not respond.
 *
 * Created on November 22, 2011, 8:57 AM
 */

#ifndef AD_H
#define AD_H

#include "PIC24.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

void AD_Init(void);
unsigned int Get_AD(short int channel);
unsigned int Get_AD16(short int channel);
unsigned int Get_Filtered_AD16(short int channel);
unsigned int Get_Last_Filtered_AD16(short int channel);
void filter(void);
void eventCheck(void);

#endif
