/*
 * File:   roach.c
 * Author: mdunne
 *
 * Created on December 15, 2011, 12:59 PM
 */

#include <roach.h>
#include <xc.h>


#include <pwm.h>
#include <serial.h>
#include <AD.h>
#include <BOARD.h>


#ifndef IM_A_ROACH
#error "This library is unavailable if this is not a roach"
#endif

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define LEFT_DIR LATFbits.LATF1
#define LEFT_DIR_INV LATEbits.LATE4
#define RIGHT_DIR LATDbits.LATD7
#define RIGHT_DIR_INV LATDbits.LATD6

#define HALL_FRONT_LEFT _RG6
#define HALL_FRONT_RIGHT _RB0
#define HALL_REAR_RIGHT _RB1
#define HALL_REAR_LEFT _RF5

#define LEFT_DIR_TRIS _TRISF1
#define LEFT_DIR_INV_TRIS _TRISE4
#define RIGHT_DIR_TRIS _TRISD7
#define RIGHT_DIR_INV_TRIS _TRISD6

#define HALL_FRONT_LEFT_TRIS _TRISG6
#define HALL_FRONT_RIGHT_TRIS _TRISB0
#define HALL_REAR_RIGHT_TRIS _TRISB1
#define HALL_REAR_LEFT_TRIS _TRISF5

#define LEFT_PWM PWM_PORTZ06
#define RIGHT_PWM PWM_PORTY12

#define LIGHT_SENSOR AD_PORTV4
#define ROACH_BAT_VOLTAGE AD_PORTV3


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function Roach_Init(void)
 * @param None.
 * @return None.
 * @brief  Performs all the initialization necessary for the roach. this includes initializing 
 * the PWM module, the A/D converter, the data directions on some pins, and 
 * setting the initial motor directions.
 * @note  None.
 * @author Max Dunne, 2012.01.06 */
void Roach_Init(void)
{

    //set the control pins for the motors
    PWM_Init();
    PWM_SetFrequency(1000);
    PWM_AddPins(LEFT_PWM | RIGHT_PWM);
    LEFT_DIR_TRIS = 0;
    LEFT_DIR_INV_TRIS = 0;
    RIGHT_DIR_TRIS = 0;
    RIGHT_DIR_INV_TRIS = 0;
    LEFT_DIR = 0;
    LEFT_DIR_INV = ~LEFT_DIR;
    RIGHT_DIR = 0;
    RIGHT_DIR_INV = ~RIGHT_DIR;

    //set up the hall effect sensors
    HALL_FRONT_LEFT_TRIS = 1;
    HALL_FRONT_RIGHT_TRIS = 1;
    HALL_REAR_RIGHT_TRIS = 1;
    HALL_REAR_LEFT_TRIS = 1;


    //Initialize the light sensor
    AD_Init();
    AD_AddPins(LIGHT_SENSOR);

    //enable interrupts
}

/**
 * @Function Roach_LeftMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief  This function is used to set the speed and direction of the left motor.
 * @author Max Dunne, 2012.01.06 */
char Roach_LeftMtrSpeed(char newSpeed)
{
    if ((newSpeed < -10) || (newSpeed > 10)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        LEFT_DIR = 0;
        newSpeed = newSpeed * (-1); // set speed to a positive value
    } else {
        LEFT_DIR = 1;
    }
    LEFT_DIR_INV = ~(LEFT_DIR);
    if (PWM_SetDutyCycle(LEFT_PWM, newSpeed * 100) == ERROR) {
        //printf("ERROR: setting channel 1 speed!\n");
        return (ERROR);
    }
    return (SUCCESS);
}

/**
 * @Function Roach_RightMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief  This function is used to set the speed and direction of the left motor.
 * @author Max Dunne, 2012.01.06 */
char Roach_RightMtrSpeed(char newSpeed)
{
    if ((newSpeed < -10) || (newSpeed > 10)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        RIGHT_DIR = 0;
        newSpeed = newSpeed * (-1); // set speed to a positive value
    } else {
        RIGHT_DIR = 1;
    }
    RIGHT_DIR_INV = ~(RIGHT_DIR);
    if (PWM_SetDutyCycle(RIGHT_PWM, newSpeed * 100) == ERROR) {
        //puts("\aERROR: setting channel 1 speed!\n");
        return (ERROR);
    }
    return (SUCCESS);
}

/**
 * @Function Roach_LightLevel(void)
 * @param None.
 * @return a 10-bit value corresponding to the amount of light received.
 * @brief  Returns the current light level. A higher value means less light is detected.
 * @author Max Dunne, 2012.01.06 */
unsigned int Roach_LightLevel(void)
{
    return AD_ReadADPin(LIGHT_SENSOR);
}

/**
 * @Function Roach_BatteryVoltage(void)
 * @param None.
 * @return a 10-bit value corresponding to the current voltage of the roach
 * @brief  returns a 10:1 scaled value of the roach battery level
 * @author Max Dunne, 2013.07.12 */
unsigned int Roach_BatteryVoltage(void)
{
    return AD_ReadADPin(ROACH_BAT_VOLTAGE);
}

/**
 * @Function Roach_ReadFrontLeftBumper(void)
 * @param None.
 * @return BUMPER_TRIPPED or BUMPER_NOT_TRIPPED
 * @brief  Returns the state of the front left bumper
 * @author Max Dunne, 2012.01.06 */
unsigned char Roach_ReadFrontLeftBumper(void)
{
    return !HALL_FRONT_LEFT;
}

/**
 * @Function Roach_ReadFrontRightBumper(void)
 * @param None.
 * @return BUMPER_TRIPPED or BUMPER_NOT_TRIPPED
 * @brief  Returns the state of the front right bumper
 * @author Max Dunne, 2012.01.06 */
unsigned char Roach_ReadFrontRightBumper(void)
{
    return !HALL_FRONT_RIGHT;
}

/**
 * @Function Roach_ReadRearLeftBumper(void)
 * @param None.
 * @return BUMPER_TRIPPED or BUMPER_NOT_TRIPPED
 * @brief  Returns the state of the rear left bumper
 * @author Max Dunne, 2012.01.06 */
unsigned char Roach_ReadRearLeftBumper(void)
{
    return !HALL_REAR_LEFT;
}

/**
 * @Function Roach_ReadRearRightBumper(void)
 * @param None.
 * @return BUMPER_TRIPPED or BUMPER_NOT_TRIPPED
 * @brief  Returns the state of the rear right bumper
 * @author Max Dunne, 2012.01.06 */
unsigned char Roach_ReadRearRightBumper(void)
{
    return !HALL_REAR_RIGHT;
}

/**
 * @Function Roach_ReadBumpers(void)
 * @param None.
 * @return 4-bit value representing all four bumpers in following order: front left,front right, rear left, rear right
 * @brief  Returns the state of all 4 bumpers
 * @author Max Dunne, 2012.01.06 */
unsigned char Roach_ReadBumpers(void)
{
    unsigned char bump_state;
    bump_state = (!HALL_FRONT_LEFT + ((!HALL_FRONT_RIGHT) << 1)+((!HALL_REAR_LEFT) << 2)+((!HALL_REAR_RIGHT) << 3));
    return (!HALL_FRONT_LEFT + ((!HALL_FRONT_RIGHT) << 1)+((!HALL_REAR_LEFT) << 2)+((!HALL_REAR_RIGHT) << 3));
}








//#define ROACH_TEST
#ifdef ROACH_TEST
#pragma config FPLLIDIV 	= DIV_2		//PLL Input Divider
#pragma config FPLLMUL 		= MUL_20	//PLL Multiplier
#pragma config FPLLODIV 	= DIV_1 	//System PLL Output Clock Divid
#pragma config FNOSC 		= PRIPLL	//Oscillator Selection Bits
#pragma config FSOSCEN 		= OFF		//Secondary Oscillator Enable
#pragma config IESO 		= OFF		//Internal/External Switch O
#pragma config POSCMOD 		= XT		//Primary Oscillator Configuration
#pragma config OSCIOFNC 	= OFF		//CLKO Output Signal Active on the OSCO Pin
#pragma config FPBDIV 		= DIV_2		//Peripheral Clock Divisor
#pragma config FCKSM 		= CSECMD	//Clock Switching and Monitor Selection
#pragma config WDTPS 		= PS1		//Watchdog Timer Postscaler
#pragma config FWDTEN		= OFF		//Watchdog Timer Enable
#pragma config ICESEL		= ICS_PGx2	//ICE/ICD Comm Channel Select
#pragma config PWP 			= OFF		//Program Flash Write Protect
#pragma config BWP 			= OFF		//Boot Flash Write Protect bit
#pragma config CP 			= OFF		//Code Protect

//#define WANDER_MODE
#define WANDER_SAMPLE_RATE 100

#define FLEFT_BUMP_MASK (1)
#define FRIGHT_BUMP_MASK (1<<1)
#define RLEFT_BUMP_MASK (1<<2)
#define RRIGHT_BUMP_MASK (1<<3)


#include <timers.h>
#include <roach.h>

int main(void)
{
    int lightLevel = 0;
    BOARD_Init();
    //        for(lightLevel=0;lightLevel<2<<20;lightLevel++)
    //        {
    //            Nop();
    //        }
    PutChar('a');
    //while(1);
    Roach_Init();
    TIMERS_Init();
#ifndef WANDER_MODE
    printf("Welcome to the Roach Test, battery voltage is %d\r\n",Roach_BatteryVoltage());
    printf("Bump states, light levels and battery voltage will be printed to the terminal every .1 seconds while also controlling the motor speed\r\n");
    char motorspeed = -10;
    Roach_LeftMtrSpeed(motorspeed);
    Roach_RightMtrSpeed(motorspeed);

    char directionflag = 0;
    char direction = 1;
    //while (1);
    char BumpState = Roach_ReadBumpers();
    char oldBumpState = 0xff;
    int LightLevels=0,oldLightLevels=0;
    InitTimer(0, 3000);
    while (1) {
        BumpState = Roach_ReadBumpers();
        if (BumpState != oldBumpState) {
            printf("Bump: %X\r\n", BumpState);
            oldBumpState = BumpState;
        }
        LightLevels=Roach_LightLevel();
        if(abs(LightLevels-oldLightLevels)>25)
        {
            printf("Light: %d\r\n",LightLevels);
            oldLightLevels=LightLevels;
        }
        if (Roach_LightLevel() > 400) {
            if (Roach_ReadFrontLeftBumper())
                Roach_LeftMtrSpeed(-10);
            if (Roach_ReadFrontRightBumper())
                Roach_RightMtrSpeed(-10);
            if (Roach_ReadRearLeftBumper())
                Roach_LeftMtrSpeed(10);
            if (Roach_ReadRearRightBumper())
                Roach_RightMtrSpeed(10);
        } else {
            if(IsTransmitEmpty())
            {
               // printf("%d\r\n",Roach_LightLevel());
            }
            Roach_LeftMtrSpeed(0);
            Roach_RightMtrSpeed(0);
        }

    }
    //    while(1)
    //    {
    //        if(IsTimerExpired(0))
    //        {
    //            direction*=-1;
    //            Roach_LeftMtrSpeed(direction*motorspeed);
    //            Roach_RightMtrSpeed(direction*motorspeed);
    //            InitTimer(0,3000);
    //            printf("going %d\r\n",direction);
    //        }
    //    }
    //    while (1) {
    //        if ((Roach_ReadFrontLeftBumper() & Roach_ReadFrontRightBumper())&&(directionflag==0)) {
    //
    //            printf("going reverse: %d\r\n",motorspeed);
    //            Roach_LeftMtrSpeed(motorspeed);
    //            Roach_RightMtrSpeed(motorspeed);
    //            directionflag=1;
    //        }
    //        else if ((Roach_ReadRearLeftBumper() & Roach_ReadRearRightBumper())&&(directionflag==1)) {
    //            printf("going forward: %d\r\n",motorspeed);
    //            Roach_LeftMtrSpeed(direction*motorspeed);
    //            Roach_RightMtrSpeed(direction*motorspeed);
    //            motorspeed--;
    //            directionflag=0;
    //        }
    //    }
    InitTimer(0, 2000);
    while (!IsTimerExpired(0));
    Roach_RightMtrSpeed(10);
    Roach_LeftMtrSpeed(10);
    //char direction = 0;
    InitTimer(1, 5000);
    while (1) {
        lightLevel = Roach_LightLevel() / 100;
        if (Roach_ReadFrontLeftBumper()) {
            Roach_LeftMtrSpeed(lightLevel);
        }
        if (Roach_ReadFrontRightBumper()) {
            Roach_RightMtrSpeed(lightLevel);
        }
        if (Roach_ReadRearLeftBumper()) {
            Roach_LeftMtrSpeed(lightLevel*-1);
        }
        if (Roach_ReadRearRightBumper()) {
            Roach_RightMtrSpeed(lightLevel*-1);
        }
        if (IsTimerExpired(1) == TIMER_EXPIRED) {

            if (direction) {
                Roach_RightMtrSpeed(10);
                Roach_LeftMtrSpeed(10);

            } else {
                Roach_RightMtrSpeed(-10);
                Roach_LeftMtrSpeed(-10);
            }
            printf("Switching Directions: %d\r\n", direction);
            direction ^= 1;
            InitTimer(1, 5000);
        }
        if (IsTransmitEmpty()) {
            printf("%d\t%d\t%d\t%d\t%d\t%d\r\n", Roach_ReadFrontLeftBumper(), Roach_ReadFrontRightBumper(), Roach_ReadRearLeftBumper(), Roach_ReadRearRightBumper(), Roach_LightLevel(), Roach_BatteryVoltage());
            InitTimer(0, 100);
        }
    }
#else
    char Basespeed = 5;
    char SpeedOffset = 2;
    char BumpState = Roach_ReadBumpers();
    char leftMotorState = 5;
    char rightMotorState = 5;
    SetTimer(0, WANDER_SAMPLE_RATE);
    while (1) {
        if (IsTimerExpired(0)) {
            lightLevel = Roach_LightLevel() / 300;
            if (leftMotorState < 0) {
                Roach_LeftMtrSpeed(leftMotorState - lightLevel);
            } else {
                Roach_LeftMtrSpeed(leftMotorState + lightLevel);
            }
            if (rightMotorState < 0) {
                Roach_RightMtrSpeed(rightMotorState - lightLevel);
            } else {
                Roach_RightMtrSpeed(rightMotorState + lightLevel);
                ;
            }

            InitTimer(0, WANDER_SAMPLE_RATE);
        }
        if (BumpState != Roach_ReadBumpers()) {
            BumpState = Roach_ReadBumpers();
            switch (BumpState) {
            case FLEFT_BUMP_MASK:
                leftMotorState = -(Basespeed + SpeedOffset);
                rightMotorState = -(Basespeed);
                break;
            case FRIGHT_BUMP_MASK:
                rightMotorState = -(Basespeed + SpeedOffset);
                leftMotorState = -(Basespeed);
                break;
            case (FRIGHT_BUMP_MASK | FLEFT_BUMP_MASK):
                rightMotorState = -Basespeed;
                leftMotorState = -Basespeed;
                break;
            case RLEFT_BUMP_MASK:
                leftMotorState = -(Basespeed + SpeedOffset);
                rightMotorState = -(Basespeed);
                break;
            case RRIGHT_BUMP_MASK:
                rightMotorState = -(Basespeed + SpeedOffset);
                leftMotorState = -(Basespeed);
                break;
            case (RRIGHT_BUMP_MASK | RLEFT_BUMP_MASK):
                rightMotorState = -Basespeed;
                leftMotorState = -Basespeed;
                break;

            }
            Roach_RightMtrSpeed(rightMotorState);
            Roach_LeftMtrSpeed(leftMotorState);
        }
    }
#endif
    return 0;
}

#endif