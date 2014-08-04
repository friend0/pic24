
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



unsigned int count = 0;

unsigned char temp;
static unsigned int dIndex, channel;
volatile char write_byte = 0;
volatile char  dataRead = 0;

unsigned char i2c_i;                            // used for i2c transceiving
unsigned int tx_data[NUM_ANALOG] = {[0 ... NUM_ANALOG-1] = 0};   //Load analog values here
unsigned char rx_data[7];

/////////////////////////////////////////////////////////////////
//
//             I2C Initialization Routine
//             ---------------------------
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////

void i2c_init(void){
    I2C1ADD = I2C_ADDR;                 //set SLAVE address
    IFS1bits.SI2C1IF = 0;               // clears the I2C Slave interrupt flag
    IEC1bits.SI2C1IE = 0;               // Enable Slave interrupt


    IFS1bits.MI2C1IF = 0;               // clears the I2C M interrupt flag
    IEC1bits.MI2C1IE = 1;               // enables I2C Slave Interrupt

    I2C1CONbits.I2CEN = 1;              //enables the I2c module
    I2C1CONbits.GCEN = 1;
    I2C1CONbits.IPMIEN = 0;             //enable slave address masking?
    I2C1CONbits.SCLREL = 0;              //enables clock stretch??
    I2C1CONbits.STREN = 1;              //enables clock stretch??

    OpenI2C1(I2C1_ON, 0);		// BRG is not used for I2C slave
    I2C1ADD = 0x61;
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

void __attribute__((interrupt, no_auto_psv)) _SI2C1Interrupt(void)
{

    // check for MASTER and Bus events and respond accordingly
    if (IFS1bits.MI2C1IF == 1) {
        IFS1bits.MI2C1IF = 0;               //Clear the I2C Master interrupt flag
            return;
    }
    if (I2C1STATbits.BCL == 1) {            //Bus Collision: need to respond accordingly?
        I2C1STATbits.BCL = 1;               // enables I2C Slave Interrupt
            return;
    }


    if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0)) {
            // R/W bit = 0 --> indicates data transfer is input to slave
            // D/A bit = 0 --> indicates last byte was address

            // reset any state variables needed by a message sequence
            // perform a dummy read of the address
            temp = SlaveReadI2C1();


            // release the clock to restart I2C
            I2C1CONbits.SCLREL = 1; // release the clock

    } else if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1)) {
            // R/W bit = 0 --> indicates data transfer is input to slave
            // D/A bit = 1 --> indicates last byte was data


            // writing data to our module, just store it in dataRead for now
            //in the future, these writes could be used to execute scripts on chip, flip bits
            dataRead = SlaveReadI2C1();

            // release the clock to restart I2C
            I2C1CONbits.SCLREL = 1; // release clock stretch bit

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0)) {
            // R/W bit = 1 --> indicates data transfer is output from slave
            // D/A bit = 0 --> indicates last byte was address

            // read of the slave device, read the address
            temp = SlaveReadI2C1();
            dIndex = 0;
            channel = 0;
            SlaveWriteI2C1(dataRead);       //puts written data in register, releases clock

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1)) {
            // R/W bit = 1 --> indicates data transfer is output from slave
            // D/A bit = 1 --> indicates last byte was data


            // output the data until the MASTER terminates the
            // transfer with a NACK, continuing reads return 0
            if (dIndex == 0) {     //we are going to get 8 events after first address bit (1-8)
                dIndex++;

                if(dIndex % 2 == 1){write_byte = (Get_AD(channel) & 0x00FF);}     //get the lower byte of current chennel
                else if(dIndex % 2 == 0){write_byte = (Get_AD(channel) & 0xFF00); channel++;}     //get upper byte, increment channel
                SlaveWriteI2C1(write_byte);        //writes byte

            } else  SlaveWriteI2C1(0);

    }

    IFS1bits.SI2C1IF = 0;               // clears the I2C1 Slave interrupt flag
    //IFS1bits.MI2C1IF = 0;               // clears the I2C M interrupt flag

}


