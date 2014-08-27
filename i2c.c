
/////////////////////////////////////////////////////////////////
//
//                       I2C Module
//             ---------------------------
//
/////////////////////////////////////////////////////////////////
#include "i2c.h"
#include "I2.h"
#include "PIC24.h"
#include "AD.h"
#include "timers.h"

static unsigned int dIndex = 0, channel = 0;
volatile char write_byte = 0;
volatile char dataRead = 0;

unsigned int tx_data[NUM_ANALOG] = {0, 0, 0, 0}; //Load analog values here


unsigned int config1 = 0;
unsigned int config2 = 0;
unsigned int i=0;
/////////////////////////////////////////////////////////////////
//
//             I2C Initialization Routine
//             ---------------------------
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////

void i2c_init(void) {

    CloseI2C2();
    I2C2ADD = I2C_ADDR; //set SLAVE address
    I2C2CONbits.I2CEN = 1; //enables the I2c module

    I2C2CONbits.IPMIEN = 1;
    I2C2CONbits.DISSLW = 1;
    I2C2CONbits.GCEN = 1; //General Call Enable bit (when operating as I2C slave)
    I2C2CONbits.SCLREL = 0;
    I2C2CONbits.STREN = 1; //enables clock stretch??

    //OpenI2C2(I2C2_ON, 0); // BRG is not used for I2C slave
    //I2C2ADD = 0xA2;
    I2C2TRN = 0;
    config1 = (I2C2_ON | I2C2_7BIT_ADD | I2C2_CLK_REL | I2C2_STR_EN  );
    config2 = 0;
    //I2C2TRN = 0;
    I2C2CONbits.GCEN = 1; //General Call Enable bit (when operating as I2C slave)
    IFS3bits.SI2C2IF = 0;               // clears the I2C Slave interrupt flag
    IEC3bits.SI2C2IE = 1;               // Enable Slave interrupt
    //This defines I2C to be slave
    OpenI2C2(config1,config2);
}


/////////////////////////////////////////////////////////////////
//
//             Slave I2C interrupt handler
//             ---------------------------
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////

typedef enum {
    IDLE,
    WAIT_FOR_ADDR,
    WAIT_FOR_WRITE,
    SEND_READ_DATA,
    SEND_READ_LAST_DATA
} STATE;

volatile STATE i2_state = WAIT_FOR_ADDR;
volatile unsigned int idx = -1, count = 0, lastState, currentState, tmp = 0;

void __attribute__((interrupt, no_auto_psv)) _SI2C2Interrupt(void) {
    IFS3bits.SI2C2IF = 0; // clears the I2C S interrupt flag

    switch (i2_state) {

        case WAIT_FOR_ADDR:
            TRISBbits.TRISB10 = 0;              //Set tristate to digital out
            TRISBbits.TRISB11 = 1;              //Set tristate to digital out
            //ClrWdt();
            //if(I2C2STATbits.D_A == 0){
                if (I2C2STATbits.R_W) {

                    idx = -1;
                    tmp = I2C2RCV;
                    while(_RBF);
                    //I2C2TRN = (temp); //get first data byte
                    I2C2TRN = Get_AD(++idx);
                    I2C2CONbits.SCLREL = 1; //release clock line so MASTER can drive it
                    while(_TBF);

                    i2_state = SEND_READ_DATA; //read transaction
                } else i2_state = WAIT_FOR_WRITE;
            //}
            break;


        case WAIT_FOR_WRITE:

            tmp = I2C2RCV;
            while(_RBF);
            idx = -1;
            //character arrived, place in buffer
            //TRISBbits.TRISB10 = ~TRISBbits.TRISB10; //Set tristate to digital out
            i2_state = WAIT_FOR_ADDR;
            break;

        case SEND_READ_DATA:
            _SWDTEN = 1;
            TRISBbits.TRISB11 = 0;              //Set tristate to digital out
            TRISBbits.TRISB10 = 1;              //Set tristate to digital out
            I2C2TRN = Get_AD(++idx); //get first data byte
            I2C2CONbits.SCLREL = 1; //release clock line so MASTER can drive it
            while(_TBF);
            if(idx == 7){i2_state = WAIT_FOR_ADDR; idx = -1; _SWDTEN = 0;}
            
            break;

        case SEND_READ_LAST_DATA:

            idx = -1;
            i2_state = WAIT_FOR_ADDR;
            break;

        default:
            idx = -1;
            i2_state = WAIT_FOR_ADDR;

    }
  
}

