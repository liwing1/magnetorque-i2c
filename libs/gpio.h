#ifndef __GPIO_H
#define __GPIO_H

#define COIL_OFF	0
#define	COIL_CW 	1
#define	COIL_CCW 	2

void gpioInit();
void setCoil(uint8_t mask, uint8_t mode);

#endif  // __GPIO_H
