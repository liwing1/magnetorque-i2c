#include <libs/i2c.h>
#include <msp430.h> 
#include <stdint.h>
#include "pwm.h"
#include "gpio.h"

uint8_t cmd[6];
extern mag_t mag;

/*
 *  I2C Configuration
 */
void i2cInit( void )
{
    UCB1CTLW0 =  UCSWRST;                  // Reset interface

    UCB1CTLW0 |=  UCSYNC | UCMODE_3 | UCMST__SLAVE;

    UCB1BRW = BAUD_100;

    UCB1I2COA0 = UCOAEN | SLV_ADDR;

    /* I2C Pins */
    P5DIR  &= ~(BIT1 | BIT0);
    P5SEL1 &= ~(BIT1 | BIT0);            // P5.0 (SDA)
    P5SEL0 |= BIT1 | BIT0;               // P5.1 (SCK)
    P5REN  |= BIT1 | BIT0;
    P5OUT  |= BIT1 | BIT0;


    UCB1CTLW0 &= ~UCSWRST;                  // Release reset

    UCB1IE     =  UCRXIE;                   // Enable RX interruptions
}

uint32_t timeOn, period;
uint16_t pwmValue;
uint8_t  direction;

enum {byte1, byte2, byte3, byte4, byte5, byte6} rxState = byte1;

#pragma vector = EUSCI_B1_VECTOR
__interrupt void I2C_ISR()
{
    if(rxState == byte1)
    {
        cmd[0] = UCB1RXBUF;

        /* Idle byte */
        if(cmd[0] == 0xFF)
        {
            //while(!(UCB1IFG & UCTXIFG));
            UCB1TXBUF = 0xFF;
            return;
        }

        /* Echo */
        if( (cmd[0] & 0xE0) == ECHO_CMD)
        {
            //while(!(UCB1IFG & UCTXIFG));
            UCB1TXBUF = cmd[0];
            return;
        }

        mag.timeOn[X_COIL] = 0;
        mag.timeOn[Y_COIL] = 0;
        mag.timeOn[Z_COIL] = 0;

        mag.period[X_COIL] = 0;
        mag.period[Y_COIL] = 0;
        mag.period[Z_COIL] = 0;


        /* Coil Control, Timed and Periodic Actuation */
        if( (cmd[0] & 0xE0) >= COIL_CMD)
        {
            rxState = byte2;
            return;
        }

    }


    if(rxState == byte2)
    {
        cmd[1] = UCB1RXBUF;
        int8_t intensity = (int8_t) UCB1RXBUF;

        /* Coil Control, Timed and Periodic Actuation */

        if(intensity < 0)                   // If negative
        {                                   //
            pwmValue = ( ~(0xFF00 | cmd[1]) + 1);       // Get two's complement
            direction = COIL_CCW;           // Set direction: counter clockwise
        }
        else if (intensity == 0)            // If zero
        {                                   //
            pwmValue = 0;                   // Turn off the coils
            direction = COIL_OFF;           //
        }
        else                                // if positive
        {                                   //
            pwmValue = cmd[1];              // Set direction: clockwise
            direction = COIL_CW;            //
        }

        setCoil(cmd[0], direction);          // Set coil direction

        // Setup configured direction
        if(cmd[0] & BIT4) mag.direction[X_COIL] = direction;
        if(cmd[0] & BIT3) mag.direction[Y_COIL] = direction;
        if(cmd[0] & BIT2) mag.direction[Z_COIL] = direction;


        if(pwmValue >= 100)                 // Adjust pwmValue
            pwmValue = 100;                 // Saturate in 100
        if(pwmValue != 0)
        {
            pwmValue *= 300;                // Scale to timer CCR0
            pwmValue -= 1;                  // Subtract 1 to match clock cycles
        }

        // Setup configured intensity
        if(cmd[0] & BIT4) mag.intensity[X_COIL] = pwmValue;
        if(cmd[0] & BIT3) mag.intensity[Y_COIL] = pwmValue;
        if(cmd[0] & BIT2) mag.intensity[Z_COIL] = pwmValue;

        if(pwmValue == 0) // If zero
        {
            if(cmd[0] & BIT4) {P1OUT &= ~BIT0; P1SEL0 &= ~BIT0; }
            if(cmd[0] & BIT3) {P1OUT &= ~BIT3; P1SEL0 &= ~BIT3; }
            if(cmd[0] & BIT2) {P1OUT &= ~BIT4; P1SEL0 &= ~BIT4; }

        }
        else if (pwmValue == 29999)// If maxed out
        {
            if(cmd[0] & BIT4) {P1OUT |= BIT0; P1SEL0 &= ~BIT0;}
            if(cmd[0] & BIT3) {P1OUT |= BIT3; P1SEL0 &= ~BIT3;}
            if(cmd[0] & BIT2) {P1OUT |= BIT4; P1SEL0 &= ~BIT4;}
        }
        else
        {
            if(cmd[0] & BIT4) {P1SEL0 |= BIT0; TA0CCR1 = pwmValue;}
            if(cmd[0] & BIT3) {P1SEL0 |= BIT3; TA1CCR2 = pwmValue;}
            if(cmd[0] & BIT2) {P1SEL0 |= BIT4; TB0CCR1 = pwmValue;}
        }

        /* Timed and Periodic Actuation */
        if ( (cmd[0] & 0xE0) >= TIMED_CMD )
        {
            mag.timescale = cmd[1] & 0x3;
            rxState = byte3;
        }
        else /* Coil Control only */
        {
            rxState = byte1;
        }

        return;

    } // end rxState: byte2

    if(rxState == byte3)
    {
        cmd[2] = UCB1RXBUF;
        rxState = byte4;
        return;
    }

    if(rxState == byte4)
    {
        cmd[3] = UCB1RXBUF;

        /* Convert time to seconds */
        timeOn = (cmd[2] << 8) | cmd[3];            // Seconds
        if(mag.timescale == 1) timeOn *= 60;        // Minutes
        if(mag.timescale == 2) timeOn *= 3600;      // Hours
        if(mag.timescale == 3) timeOn *= 86400;     // Days

        if(cmd[0] & BIT4) {mag.timeOn[X_COIL] = timeOn;}  // Setup timer
        if(cmd[0] & BIT3) {mag.timeOn[Y_COIL] = timeOn;}
        if(cmd[0] & BIT2) {mag.timeOn[Z_COIL] = timeOn;}

        if ( (cmd[0] & 0xE0) == PERIODIC_CMD )
            rxState = byte5;
        else
            rxState = byte1;

        return;
    }

    if(rxState == byte5)
    {
        cmd[4] = UCB1RXBUF;
        rxState = byte6;
        return;
    }


    if(rxState == byte6)
    {
        cmd[5] = UCB1RXBUF;

        /* Convert time to seconds */
        period = (cmd[4] << 8) | cmd[5];                // Seconds
        if(mag.timescale == 1) period *= 60;            // Minutes
        if(mag.timescale == 2) period *= 3600;          // Hours
        if(mag.timescale == 3) period *= 86400;         // Days

        if(period <= timeOn)
        {
            rxState = byte1;
            return;
        }

        if(cmd[0] & BIT4) {mag.period[X_COIL] = period;}      // Setup timer
        if(cmd[0] & BIT3) {mag.period[Y_COIL] = period;}
        if(cmd[0] & BIT2) {mag.period[Z_COIL] = period;}

        rxState = byte1;
        return;
    }
}
