#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

#define X_COIL 0
#define Y_COIL 1
#define Z_COIL 2

typedef struct {
    uint8_t  timescale;  					// 0: sec,  1: min, 2: hour, 3: day
    uint8_t  direction[3];					// 0: none, 1: clockwise, 2: counter cw
    uint16_t intensity[3];					// PWM duty cycle [0:100]
    uint32_t timeOn[3];						// time on
    uint32_t period[3];						// time on + time off
} mag_t;

void pwmInit();


#endif  // __TIMER_H
