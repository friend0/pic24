// /////////////////////////////////////////////////////////
//     Filename:       main.c                             //
////////////////////////////////////////////////////////////

#include "xc.h"
#include "timer.h"
#include "ports.h"
#include "timer.h"
#include <stdio.h>
#include "AD.h"
#include "I2.h"
#include "PIC24.h"
#include "uart.h"

#define FCY 70000000UL   // Almost
#define FP FCY
#define USE_AND_OR	// To enable AND_OR mask setting for I2C.


#ifndef SUCCESS
#define SUCCESS 1
#define TRUE 1
#define FAILURE 0
#define FALSE 0
#endif


// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

_FOSCSEL(FNOSC_FRC);
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT);

int main(void) {
    //Hardware Inits
    Board_Init();   //configure ADC and UART
    Timer_Init();   //configure Timer Interrupts

    while (1) {
    }

}
