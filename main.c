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

    // FICD
    #pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
    #pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

    // FPOR
    #pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
    #pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
    #pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

    // FWDT
    #pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
    #pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
    #pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
    #pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
    #pragma config FWDTEN = OFF              // Watchdog Timer Enable bit (Watchdog timer always enabled)

    // FOSC
    #pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
    #pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
    #pragma config IOL1WAY = ON             // Peripheral pin select configuration (Allow only one reconfiguration)
    #pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

    // FOSCSEL
    #pragma config FNOSC = FRC              // Oscillator Source Selection (Internal Fast RC (FRC))
    #pragma config IESO = OFF                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

    // FGS
    #pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
    #pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

int main ( void )
{
    //Hardware Inits
    Board_Init();
    //AD_Init();
    //i2c_init();

    RCONbits.SWDTEN = 0;    /*Diable watchdog*/
    CORCON |= 0x0001;     /* Enable integer arithmetic */

    TRISB = 0;
    ODCBbits.ODCB10 = 0;		// disable open-drain mode
    //TRISBbits.TRISB10 = 0;              //Set tristate to digital out

    ODCBbits.ODCB11 = 0;		// disable open-drain mode
    //TRISBbits.TRISB11 = 0;              //Set tristate to digital out

    ODCBbits.ODCB12 = 0;		// disable open-drain mode
    //TRISBbits.TRISB12 = 0;              //Set tristate to digital out

    ConfigIntTimer3(T1_INT_PRIOR_3 & T3_INT_ON);
    OpenTimer3(T3_ON &      //Timer1 ON
            T3_IDLE_CON&
            T3_GATE_OFF &   //Gated mode OFF
            T3_PS_1_8 &   //Prescale 8
            T3_SOURCE_INT, 0x0fff);

    while (1)           
    {               //just service interrupts
    }

}
void AD_Init(void){

    AD1CON3 = 0x1F02;           // max sample time = 31Tad, Tad = 2 x Tcy = 125ns >75ns
    AD1CON2 = 0;                // use MUXA, AVss and AVdd are used as Vref+/-
    AD1CON1bits.SSRC = 2;       // set interrupt source to tmr3
    AD1CON1bits.SIMSAM = 1;     //simultaneous sampling on
    AD1CON1bits.FORM = 0;       //output form is unsigned fraction
    AD1CON1bits.ASAM = 1;       //Auto Sampling On
    AD1CON2bits.CHPS = 2;       //Multi-Channel (0-3)
    AD1CON2bits.SMPI = 3;       //Samples per interrupt;
    AD1CON2bits.BUFM = 0;       //Set buffer load mode to use all 16 bufs
    AD1CON3bits.ADRC = 0;       //ADC clock is system clock;
    AD1CON1bits.ADON = 1;       // turn on the ADC;

    AD1CHS0bits.CH0SA = 3;      // Select AN3 for CH0 +ve input
    AD1CHS0bits.CH0NA = 0;      // Select AVss for CH0 -ve input

    AD1CHS123bits.CH123SA=0;    // Select AN0 for CH1 +ve input
                                // Select AN1 for CH2 +ve input
                                // Select AN2 for CH3 +ve input
    AD1CHS123bits.CH123NA=0;    // Select AVss for CH1/CH2/CH3 -ve inputs
    TRISA = 0;                  //set unused pins as 0 outputs
    PORTA = 0;
    ANSELA = 0x0000;            //Disable all analog pins PortA/B
    ANSELB = 0x0000;
    ANSELAbits.ANSA0 = 1;       //Set An0, An1, An2, An3 as inputs
    ANSELAbits.ANSA1 = 1;
    ANSELBbits.ANSB0 = 1;
    ANSELBbits.ANSB1 = 1;
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;

}



/////////////////////////////////////////////////////////////////
//
//                  ADC interrupt handler
//                  ---------------------
// This handler is called on an interval set by Timer 3
// ADC unit is configured to respond to interrupts at this vector
// Take an average of respective sample values in buffer
// Load average into I2C data transmission array tx_data[]
//
///////////////////////////////////////////////////////////////////

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
        while (!AD1CON1bits.DONE);   // wait to complete the conversion
        TRISB = ~TRISB;                   //Toggle LED

        //load new analog values into memory; take an average of samples
        //Think about implementing the same filter as on pic32 end?
        ADC_Values[3] = (ADC1BUF0 + ADC1BUF4 + ADC1BUF8 + ADC1BUFC) >> 2;
        ADC_Values[0] = (ADC1BUF1 + ADC1BUF5 + ADC1BUF9 + ADC1BUFD) >> 2;
        ADC_Values[1] = (ADC1BUF2 + ADC1BUF6 + ADC1BUFA + ADC1BUFE) >> 2;
        ADC_Values[2] = (ADC1BUF3 + ADC1BUF7 + ADC1BUFB + ADC1BUFF) >> 2;

        IFS0bits.T3IF = 0; //TMR3 interrupt flag must be cleared in software
}
