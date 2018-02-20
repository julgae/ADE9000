/**
 * spi_lib header
 */
#ifndef _SPI_LIB_H_
#define _SPI_LIB_H_

#include <stdint.h>

typedef struct Spi_s {
	char portNum[20];
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	int fd;
} Spi_t;

extern int spi_init(Spi_t *spi);

extern int spi_term(Spi_t *spi);

extern int spi_write(Spi_t *spi, char *txBuf, uint32_t txByte);

extern int spi_read(Spi_t *spi, char *rxBuf, uint32_t rxByte);

extern int spi_simWriteRead(Spi_t *spi, char *txBuf, char *rxBuf, uint32_t txRxByte);

extern int spi_seqWriteRead(Spi_t *spi, char *txBuf, uint32_t txByte, char *rxBuf, uint32_t rxByte);

#endif