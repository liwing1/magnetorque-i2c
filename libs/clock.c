#include <msp430.h> 
#include "clock.h"

void clockInit()
{
    FRCTL0 = FRCTLPW | NWAITS_1;            // FRAM needs 2 clocks when operating at 16MHz

    /* Clock pins */                        
    PJSEL0 |=  (BIT4 | BIT5 |               // Connect XTAL to oscillator
                BIT6 | BIT7);               // LFXT @ 32768 Hz
    PJSEL1 &= ~(BIT4 | BIT5 |               // HFXT @ 16 MHz
                BIT6 | BIT7);               //

    CSCTL0 = CSKEY;                         // Open Clock System registers

    CSCTL1 = DCORSEL | DCOFSEL_4;           // DCO   -> 16MHz

    CSCTL3 = DIVS__1;                       // SMCLK divide by 1

    CSCTL4 = HFXTDRIVE_0   |                // Disable HFXT
//           HFXTBYPASS    |
             HFFREQ_0      |
             HFXTOFF       |
//           LFXTDRIVE_3   |                // Set LFXT drive to max
//           LFXTBYPASS    |                // for fast start-up
//           VLOOFF        |
//           SMCLKOFF      |
//           LFXTOFF       |
             0;

    CSCTL5 = ENSTFCNT2     |                // Enable fault counters and
             ENSTFCNT1     |
//           HFXTOFFG      |                // reset fault flags
//           LFXTOFFG      |
             0;

    CSCTL6 =
//           MODCLKREQEN   |                // Enable conditional requests for
              SMCLKREQEN   |                // all system clocks. In case one
               MCLKREQEN   |                // fails, another takes over
               ACLKREQEN;

    while (SFRIFG1 & OFIFG){                // Check if all clocks are oscillating
        CSCTL5  &= ~LFXTOFFG;               // Try to clear XTAL fault flags,
        SFRIFG1 &= ~OFIFG;                  // and system fault flag
    }

//    CSCTL4 &= ~(LFXTDRIVE_3);             // Lower LFXT drive to save power

    CSCTL2 = SELA__LFXTCLK |                // ACLK  -> 32.768 Hz (LFXT)
             SELS__DCOCLK  |                // MCLK ->     16 MHz (DCO)
             SELM__DCOCLK;                  // SMCLK ->    16 MHz (DCO)

    CSCTL0_H = 0;                           // Lock CS registers for write

}


