/*
Engscope
UART
April 16, 2008
Author: JL
*/

//prototypes

//Initiation
extern void UARTInit(int BAUDRATEREG1);

//UART transmit function
extern void  UARTPutChar(char Ch);

//UART receive function
extern char UARTGetChar();

extern void uart_process(void);