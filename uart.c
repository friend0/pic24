
#include "p24EP32GP202.h"	//see tutorial below!
#include "uart.h"
#include "AD.h"
#include "cbuff.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define LED_1 LATBbits.LATB6
#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

/*******************************************************************************
 * PRIVATE VARIABLES                                                            *
 ******************************************************************************/
unsigned int i = 0;
volatile unsigned char hammingWeight = 0, parityBit = 0, LSB = 0, MSB = 0;

void UARTInit(int BAUDRATEREG1) {
    U1MODEbits.STSEL = 0; // 1-stop bit
    U1MODEbits.PDSEL = 0; // No parity, 8-data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disables
    U1MODEbits.BRGH = 1; // high speed mode
    U1BRG = BAUDRATEREG1;
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable UART TX
}

//UART transmit function, parameter Ch is the character to send
void UART1PutChar(char Ch) {
    //transmit ONLY if TX buffer is empty
    while (U1STAbits.UTXBF == 1);
    U1TXREG = Ch;
}

//UART receive function, returns the value received.
char UART1GetChar() {
    char Temp;
    //wait for buffer to fill up, wait for interrupt
    while (IFS0bits.U1RXIF == 0);
    Temp = U1RXREG;
    //reset interrupt
    IFS0bits.U1RXIF = 0;
    //return my received byte
    return Temp;
}

//find the hamming weight of a char
unsigned char count_bits(unsigned char n) {
    unsigned char c; // c accumulates the total bits set in v
    for (c = 0; n; c++)
        n &= n - 1; // clear the least significant bit set
    return c;
}

//get the parity of a char
unsigned char parity_bits(unsigned char n) {
    unsigned char c;
    c = (n & 0x01); //mask parity
    return c;
}

void uart_process(void) {
    //TODO: software watchdog
    while (U1STAbits.UTXBF == 1);
    UART1PutChar('z');
    UART1PutChar('e');

    //ClrWdt(); //just service interrupts
    for (i = 0; i < (NUM_ANALOG); i++) {

        /*TODO
         * Change this to work with new filtered_adc16 values
         *
        */

        //LSB = (Get_AD16(i) & 0x00FF);
        //MSB = ((Get_AD16(i) & 0x0F00) >> 8);

        LSB = (Get_Filtered_AD16(i) & 0x00FF);
        MSB = ((Get_Filtered_AD16(i) & 0x0F00) >> 8);

        MSB |= (0x10 << i); //add 'address' tag to MSB

        hammingWeight = count_bits(MSB) + count_bits(LSB);
        parityBit = parity_bits(LSB) + parity_bits(MSB);

        UART1PutChar(MSB);
        UART1PutChar(LSB);
        UART1PutChar(hammingWeight);
        UART1PutChar(parityBit);
    }
}


void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    static char newData = FALSE;
    IFS0bits.T1IF = 0; //TMR3 interrupt flag must be cleared in software
    LED_1 = ~LED_1;
    /*TODO
     consider placing this function in main while loop?
     * consider turning this into 'eventChecker' function where the various buffers are checked
     */

    if(cbuff_isnonempty(&adcBuffer)){
        cbuff_dequeue1(&adcBuffer);
        uart_process();
    }
    newData = FALSE;
}