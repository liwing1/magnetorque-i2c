/*
 * my_i2c.h
 *
 *  Created on: 14 de mar de 2021
 *      Author: liwka
 */

#ifndef LIBS_MY_I2C_H_
#define LIBS_MY_I2C_H_

#include <msp430.h>
#include <stdint.h>
#include "i2c.h"

void I2C_config_B2(uint8_t isMaster, uint8_t ownAddr, uint32_t baudRate_kHz);
uint8_t I2C_Write(uint8_t slaveAddr, uint8_t *data, uint16_t nBytes);
void i2cSendByte(uint8_t addr, uint8_t data);
void LcdWriteNibble(uint8_t slaveAddr, uint8_t nibble, uint8_t R_S);
void LcdWriteByte(uint8_t slaveAddr, uint8_t byte, uint8_t R_S);
void ConfigLCD( void );


#endif /* LIBS_MY_I2C_H_ */
