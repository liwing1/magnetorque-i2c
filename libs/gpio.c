#include <msp430.h> 
#include <stdint.h>
#include "gpio.h"

void gpioInit()
{
    /*
     * Pin configuration - Configure all unused pins
     */
    PADIR = 0; PAOUT = 0;     PBDIR = 0; PBOUT = 0;
    P1DIR = 0; P1OUT = 0;     P3DIR = 0; P3OUT = 0;
    P2DIR = 0; P2OUT = 0;     P4DIR = 0; P4OUT = 0;

    PCDIR = 0; PCOUT = 0;     PDDIR = 0; PDOUT = 0;
    P5DIR = 0; P5OUT = 0;     P7DIR = 0; P7OUT = 0;
    P6DIR = 0; P6OUT = 0;     P8DIR = 0; P8OUT = 0;

    P1OUT &= ~(BIT1 | BIT2);                // P1.1 (X CTL2)
    P1DIR |=   BIT1 | BIT2;                 // P1.2 (X CTL1)

    P3OUT &= ~BIT3;                         // P3.3 (Y CTL1)
    P3DIR |=  BIT3;                         //
    
    P4OUT &= ~BIT7;                         // P4.7 (Y CTL2)
    P4DIR |=  BIT7;                         // 

    PJOUT &= ~BIT0;                         // PJ.0 (Z CTL1)
    PJDIR |=  BIT0;                         // 

    P1OUT &= ~BIT5;                         // P1.5 (Z CTL2)
    P1DIR |=  BIT5;                         // 

    P3OUT |=  BIT0;                         // P3.0 (Stand by X,Y)
    P3DIR |=  BIT0;                         // Active low

    PJOUT |=  BIT1;                         // PJ.1 (Stand by Z)
    PJDIR |=  BIT1;                         // Active low

    /* Remove High impedance state of pins */
    PM5CTL0 &= ~LOCKLPM5;
}

void setCoil(uint8_t mask, uint8_t mode)
{
    if(mask & BIT4)                         /* X Axis */
    {
        if(mode == COIL_OFF)                // Stop
        {                                   // 
            P1OUT &= ~BIT2;                 // X CTL1 = 0
            P1OUT &= ~BIT1;                 // X CTL2 = 0
        }
        if(mode == COIL_CW)                 // ClockWise
        {                                   //
            P1OUT |=  BIT2;                 // X CTL1 = 1
            P1OUT &= ~BIT1;                 // X CTL2 = 0
        }
        if(mode == COIL_CCW)                // Counter Clockwise
        {                                   //
            P1OUT &= ~BIT2;                 // X CTL1 = 0
            P1OUT |=  BIT1;                 // X CTL2 = 1
        }
    }


    if(mask & BIT3)                         /* Y Axis */
    {
        if(mode == COIL_OFF)                // Stop
        {                                   //
            P3OUT &= ~BIT3;                 // Y CTL1 = 0
            P4OUT &= ~BIT7;                 // Y CTL2 = 0
        }
        if(mode == COIL_CW)                 // ClockWise
        {                                   //
            P3OUT &= ~BIT3;                 // Y CTL1 = 0
            P4OUT |=  BIT7;                 // Y CTL2 = 1
        }
        if(mode == COIL_CCW)                // Counter Clockwise
        {                                   //
            P3OUT |=  BIT3;                 // Y CTL1 = 1
            P4OUT &= ~BIT7;                 // Y CTL2 = 0
        }
    }

    if(mask & BIT2)                         /* Z Axis */
    {
        if(mode == COIL_OFF)                // Stop
        {                                   //
            PJOUT &= ~BIT0;                 // Z CTL1 = 0
            P1OUT &= ~BIT5;                 // Z CTL2 = 0
        }
        if(mode == COIL_CW)                 // ClockWise
        {                                   //
            PJOUT &= ~BIT0;                 // Z CTL1 = 0
            P1OUT |=  BIT5;                 // Z CTL2 = 1
        }
        if(mode == COIL_CCW)                // Counter Clockwise
        {                                   //
            PJOUT |=  BIT0;                 // Z CTL1 = 1
            P1OUT &= ~BIT5;                 // Z CTL2 = 0
        }  
    }

}

