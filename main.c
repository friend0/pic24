// /////////////////////////////////////////////////////////
//     Filename:       main.c                             //
////////////////////////////////////////////////////////////


#define FCY 8000000UL   // Almost
#define USE_AND_OR	// To enable AND_OR mask setting for I2C. 

#ifndef SUCCESS
#define SUCCESS 1
#define TRUE 1
#define FAILURE 0
#define FALSE 0
#endif

#include <xc.h>
#include <i2c.h>
#include "ports.h"
#include "libpic30.h"
#include "timer.h"
#include <stdio.h>
#include "AD.h"
#include "I2.h"
#include "PIC24.h"
#include "uart.h"

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to ASDA1/ASCL1 pins)
#pragma config ALTI2C2 = ON            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS128        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR32           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF              // Watchdog Timer Enable bit (Watchdog timer always enabled)


// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF             // Peripheral pin select configuration (Allow only one reconfiguration)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Source Selection (Internal Fast RC (FRC))
#pragma config IESO = OFF                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)







int main(void) {
    //Hardware Inits
    //Board_Init();
    AD_Init();
    //i2c_init();
    UARTInit(1);



    RCONbits.SWDTEN = 0; /*Diable watchdog*/
    CORCON |= 0x0001; /* Enable integer arithmetic */
    //TRISB = 1;


    //RPINR18bits.U1RXR = 2; //UART1 receive set to RB2
    RPOR1bits.RP37R = 1; //UART1 transmit set to Rp37 //pin14


    ODCBbits.ODCB10 = 0; // disable open-drain mode
    TRISBbits.TRISB10 = 1; //Set tristate to digital out

    ODCBbits.ODCB11 = 0; // disable open-drain mode
    TRISBbits.TRISB11 = 1; //Set tristate to digital out

    ODCBbits.ODCB7 = 0; // disable open-drain mode
    TRISBbits.TRISB7 = 0; //Set tristate to digital out

    ODCBbits.ODCB6 = 0; // disable open-drain mode
    TRISBbits.TRISB6 = 1; //Set tristate to digital in

    ODCBbits.ODCB5 = 0; // disable open-drain mode
    TRISBbits.TRISB5 = 1; //Set tristate to digital in


    ConfigIntTimer3(T3_INT_PRIOR_4 & T3_INT_ON);
    OpenTimer3(T3_ON & //Timer1 ON
            T3_IDLE_CON &
            T3_GATE_OFF & //Gated mode OFF
            T3_PS_1_64 & //Prescale 256
            T3_SOURCE_INT, 0x000f);


    while (1) {
        uart_process();
    }

}


