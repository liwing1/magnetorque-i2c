#ifndef __SPI_H
#define __SPI_H

#define ECHO_CMD                0x00
#define STATS_CMD               0x20
#define COIL_CMD                0x60
#define TIMED_CMD               0x80
#define PERIODIC_CMD            0xC0

void spiInit();

#endif  // __SPI_H
