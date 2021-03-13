#include <msp430.h> 
#include <stdint.h>
#include "libs/spi.h"
#include "libs/clock.h"
#include "libs/gpio.h"
#include "libs/pwm.h"

/*
 * One time init function
 */
void boot()
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    gpioInit();
    clockInit();                            // Initialize Clock System
    pwmInit();
    spiInit();                              // Initialize SPI
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

}

/*
 * Background task
 */

uint16_t timeOut;
uint8_t autotest;

void main()
{

    boot();

    timeOut  = 10 * 60; // 10min
    TA2CTL   = TASSEL__ACLK | MC__UP;
    TA2CCR0  = 0x8000 - 1;
    TA2CCTL0 = CCIE;

    UCB0IE  &= ~UCRXIE;
    autotest = 1;

    while(timeOut)    
    {
        if(UCB0IFG & UCRXIFG)
        {
            if(UCB0RXBUF == 0xFF)
            {
                autotest = 0;
                break;
            }
        }
    }

    if(autotest)
    {
        setCoil(BIT4 , COIL_CW);
        P1OUT  |=  BIT0;            // X: 100% Clockwise 
        P1SEL0 &= ~BIT0;
        
        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT4 , COIL_CCW);   // X: 100% Counter Clockwise

        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT4 , COIL_OFF);   // Turn off X
        P1OUT  &= ~BIT0;            // X: Off  

        // -------------------------------------------------------
        setCoil(BIT3 , COIL_CW);
        P1OUT |= BIT3;              // Y: 100% Clockwise
        P1SEL0 &= ~BIT3;

        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT3 , COIL_CCW);   // Y: 100% Counter Clockwise

        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT3 , COIL_OFF);   // Turn off Y
        P1OUT  &= ~BIT3;            // Y: Off  

        // -------------------------------------------------------
        setCoil(BIT2 , COIL_CW);
        P1OUT  |=  BIT4;            // Z: 100% Clockwise
        P1SEL0 &= ~BIT4;        
        
        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT2 , COIL_CCW);   // Z: 100% Counter Clockwise

        timeOut = 5;                // Wait 5 sec
        while(timeOut);

        setCoil(BIT2 , COIL_OFF);   // Turn off Z
        P1OUT  &= ~BIT4;

        // -------------------------------------------------------
        // -------------------------------------------------------
        // Test 2 - Measure actuation for long period of time

        setCoil(BIT4 , COIL_CW);
        P1OUT  |=  BIT0;            // X: 100% Clockwise 
        P1SEL0 &= ~BIT0;
        
        timeOut = 5 * 60 * 60;      // Wait 5 hours
        while(timeOut);

        setCoil(BIT4 , COIL_OFF);   // Turn off X
        P1OUT  &= ~BIT0;            // X: Off  

        // -------------------------------------------------------
        setCoil(BIT3 , COIL_CW);
        P1OUT |= BIT3;              // Y: 100% Clockwise
        P1SEL0 &= ~BIT3;

        timeOut = 5 * 60 * 60;      // Wait 5 hours
        while(timeOut);

        setCoil(BIT3 , COIL_OFF);   // Turn off Y
        P1OUT  &= ~BIT3;            // Y: Off  

        // -------------------------------------------------------
        setCoil(BIT2 , COIL_CW);
        P1OUT  |=  BIT4;            // Z: 100% Clockwise
        P1SEL0 &= ~BIT4;        
        
        timeOut = 5 * 60 * 60;      // Wait 5 hours
        while(timeOut);
        
        setCoil(BIT2 , COIL_OFF);   // Turn off Z
        P1OUT  &= ~BIT4;

        // -------------------------------------------------------

    }

    TA2CTL    =  MC__STOP;
    TA2CCTL0 &= ~CCIE;

    UCB0IE   |=  UCRXIE;

    gpioInit();
    while(1);

    //__low_power_mode_0();

}


#pragma vector = TIMER2_A0_VECTOR
__interrupt void TIMEOUT_ISR()
{
    if(timeOut)
        timeOut--;
}
