/**
 * SPI 공통 라이브러리
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "spi_lib.h"

static int spi_open(Spi_t *spi);
static int spi_close(Spi_t *spi);
static int spi_mode(Spi_t *spi);
static int spi_bits(Spi_t *spi);
static int spi_speed(Spi_t *spi);

/**
 * SPI Init
 */
int spi_init(Spi_t *spi) {
	spi_open(spi);
	spi_mode(spi);
	spi_bits(spi);
	spi_speed(spi);

	return 0;
}

/**
 * SPI Terminate
 */
int spi_term(Spi_t *spi) {
	spi_close(spi);

	return 0;
}

/**
 * SPI Open
 */
int spi_open(Spi_t *spi) {
	char buf[40];

	sprintf(buf, "%s", spi->portNum);
	if ((spi->fd = open(buf, O_RDWR)) < 0) {
		printf("SPI Open Error!\n");
		return -1;
	}

	return 0;
}

/**
 * SPI Close
 */
int spi_close(Spi_t *spi) {
	if (close(spi->fd) < 0) {
		printf("SPI Close Error!\n");
		return -1;
	}

	spi->fd = -1;

	return 0;
}

/**
 * SPI Write
 * @writeByte : 쓰기 할 바이트 수
 */
int spi_write(Spi_t *spi, char *txBuf, uint32_t txByte) {
	struct spi_ioc_transfer wr = {
			.tx_buf = (unsigned long) txBuf,
			.len = txByte,
			.speed_hz = spi->speed,
	};

	if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &wr) < 0) {
		printf("Failed to write\n");
		return -1;
	}

	return 0;
}

/**
 * SPI Read
 * @readByte : 읽을 바이트 수
 */
int spi_read(Spi_t *spi, char *rxBuf, uint32_t rxByte) {
	struct spi_ioc_transfer rd = {
			.rx_buf = (unsigned long) rxBuf,
			.len = rxByte,
			.speed_hz = spi->speed,
	};

	if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &rd) < 0) {
		printf("Failed to read\n");
		return -1;
	}

	return 0;
}

/**
 * SPI Write & Read Simultaneously
 * @rdWrByte : 쓰기,읽기 더한 바이트 수
 */
int spi_simWriteRead(Spi_t *spi, char *txBuf, char *rxBuf, uint32_t txRxByte) {
	struct spi_ioc_transfer wrRd = {
			.tx_buf = (unsigned long) txBuf,
			.rx_buf = (unsigned long) rxBuf,
			.len = txRxByte,
			.speed_hz = spi->speed,
	};

	if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &wrRd) < 0) {
		printf("Failed to simultaneous write and read\n");
		return -1;
	}

	return 0;
}

/**
 * SPI Write & Read Sequentially
 * @writeByte : 쓰기 할 바이트 수
 * @readByte : 읽을 바이트 수
 */
int spi_seqWriteRead(Spi_t *spi, char *txBuf, uint32_t txByte, char *rxBuf, uint32_t rxByte) {
	struct spi_ioc_transfer wrRd[2];
	memset(wrRd, 0x0, sizeof(struct spi_ioc_transfer) * 2);

	wrRd[0].tx_buf = (unsigned long) txBuf;
	wrRd[0].len = txByte;
	wrRd[1].rx_buf = (unsigned long) rxBuf;
	wrRd[1].len = rxByte;

	if (ioctl(spi->fd, SPI_IOC_MESSAGE(2), wrRd) < 0) {
		printf("Failed to sequential write and read\n");
		return -1;
	}

	return 0;
}

/**
 * SPI Mode
 */
int spi_mode(Spi_t *spi) {
	int ret = -1;
	uint8_t getMode;

	ret = ioctl(spi->fd, SPI_IOC_WR_MODE, &(spi->mode));
	if (ret < 0) {
		perror("Can't set spi mode");
		goto FINISH;
	}

	ret = ioctl(spi->fd, SPI_IOC_RD_MODE, &getMode);
	if (ret < 0) {
		perror("Can't get spi mode");
		goto FINISH;
	}

	if (getMode != (spi->mode)) {
		perror("Set mode is not the same as get mode\n");
		ret = -1;
	}
FINISH:
	return ret;
}

/**
 * SPI Bits
 */
int spi_bits(Spi_t *spi) {
	int ret = -1;
	uint8_t getBits;

	ret = ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &(spi->bits));
	if (ret < 0) {
		perror("Can't set bits per word");
		goto FINISH;
	}

	ret = ioctl(spi->fd, SPI_IOC_RD_BITS_PER_WORD, &getBits);
	if (ret < 0) {
		perror("Can't get bits per word");
		goto FINISH;
	}

	if (getBits != (spi->bits)) {
		perror("Set bit is not the same as get bit\n");
		ret = -1;
	}
FINISH:
	return ret;
}

/**
 * SPI max speed
 */
int spi_speed(Spi_t *spi) {
	int ret = -1;
	uint32_t getSpeed;

	ret = ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi->speed));
	if (ret < 0) {
		perror("Can't set spi max speed hz");
		goto FINISH;
	}

	ret = ioctl(spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &getSpeed);
	if (ret < 0) {
		perror("Can't get spi max speed hz");
		goto FINISH;
	}

	if (getSpeed != (spi->speed)) {
		perror("Set speed is not the same as get speed\n");
		ret = -1;
	}
FINISH:
	return ret;
}
