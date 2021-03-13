#include <libs/pwm.h>
#include <msp430.h> 
#include <stdint.h>
#include "libs/gpio.h"

mag_t mag;
uint32_t timeLeftOn[3], timeLeftOff[3];
/*
 * Timer configuration
 */
void pwmInit()
{

    TA0CTL  = TASSEL__ACLK | MC__UP;        // Select slow clock @32768Hz
    TA1CTL  = TASSEL__ACLK | MC__UP;        //
    TB0CTL  = TBSSEL__ACLK | MC__UP;        //

    TA0CCR0 = 30000 - 1;                    // Period < 1 sec
    TA1CCR0 = 30000 - 1;                    // steps of N = 300
    TB0CCR0 = 30000 - 1;                    //

    TA0CCR1 = 0;                            // X axis duty cycle
    TA1CCR2 = 0;                            // Y axis duty cycle
    TB0CCR1 = 0;                            // Z axis duty cycle

    TA0CCTL1 = OUTMOD_7;                    // PWM mode
    TA1CCTL2 = OUTMOD_7;                    //
    TB0CCTL1 = OUTMOD_7;                    //

    /* Timer pins */
    P1DIR  |=  (BIT0 | BIT3 | BIT4);        // P1.0 (X) - PWM mode
    P1SEL0 |=  (BIT0 | BIT3 | BIT4);        // P1.3 (Y) - PWM mode
    P1SEL1 &= ~(BIT0 | BIT3 | BIT4);        // P1.4 (Z) - PWM mode

    TA4CTL   = TASSEL__ACLK | MC__UP;       // Timer to count 1 sec
    TA4CCR0  = 0x8000 - 1;                  // Period = 1 sec
    TA4CCTL0 = CCIE;

}

/*
 * Timer Interruptions (once every second)
 */
#pragma vector = TIMER4_A0_VECTOR
__interrupt void TIMER_ISR()
{

    uint8_t coil = 3;
    uint8_t coilMask = BIT1;
    while(coil--)
    {
        coilMask <<= 1;

        // If running continuously, don't mess up with timers
        if(!mag.period[coil] && !mag.timeOn[coil])
            continue;

        // If mode is periodic and there is no more time left, reload timeLeft
        if(mag.period[coil] && !timeLeftOn[coil] && !timeLeftOff[coil])
        {
            timeLeftOn[coil]  = mag.timeOn[coil];
            timeLeftOff[coil] = mag.period[coil] - mag.timeOn[coil];
            // don't return here, the next decrement will adjust time
        }

        // Timed and periodic mode
        if(timeLeftOn[coil]) 
        {
            timeLeftOn[coil]--;
            setCoil(coilMask, mag.direction[coil]);
        }
        else
        {
            // Periodic mode
            if(timeLeftOff[coil])
                timeLeftOff[coil]--;
            setCoil(coilMask, COIL_OFF);
        }

    }


}
