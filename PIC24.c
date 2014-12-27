//Board Init

#include "PIC24.h"
#include "xc.h"
#include "timer.h"
#include "uart.h"
#include "AD.h"

void Board_Init(void){
    //Configure PLL prescaler, PLL postscaler, PLL divisor (primary OSC )
    PLLFBD = 68;   // M = 70
    CLKDIVbits.PLLPOST = 0;     // N2 = 2
    CLKDIVbits.PLLPRE=0;     // N1 = 2
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(0x01);
    while (OSCCONbits.COSC != 0b011);
    while (OSCCONbits.LOCK != 1);

    RCONbits.SWDTEN = 0; /*Disable watchdog*/
    CORCON |= 0x0001; /* Enable integer arithmetic */

    RPOR1bits.RP37R = 1; //UART1 transmit set to Rp37 //pin14
    RPINR18bits.U1RXR = 39; //UART1 RX set RP39 //pin16
    _TRISB6 = 0;   // RB6 as output

    AD_Init();
    UARTInit(150);
}

void Timer_Init(void){

    ConfigIntTimer3(T3_INT_PRIOR_4 & T3_INT_ON);
    OpenTimer3(T3_ON & //Timer1 ON
            T3_IDLE_CON &
            T3_GATE_OFF & //Gated mode OFF
            T3_PS_1_64 & //Prescale 256
            T3_SOURCE_INT, 1094);


    ConfigIntTimer1(T1_INT_PRIOR_1 & T1_INT_ON);
    OpenTimer1(T1_ON & //Timer1 ON
            T1_IDLE_CON &
            T1_GATE_OFF & //Gated mode OFF
            T1_PS_1_64 & //Prescale 256
            T1_SOURCE_INT, 1094*2);
}