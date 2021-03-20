#ifndef __I2C_H
#define __I2C_H

#define ECHO_CMD                0x00
#define STATS_CMD               0x20
#define COIL_CMD                0x60
#define TIMED_CMD               0x80
#define PERIODIC_CMD            0xC0

#define SLV_ADDR                0x12
#define MST_ADDR                0x14
#define BAUD_100                100

void i2cInit( void );

#endif  // __I2C_H
