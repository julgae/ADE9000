/**
 * ADE9000 기능
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ade9000.h"
#include "ade9000_reg.h"
#include "spi_lib.h"
#include "csv_file.h"

#define MODE (0) /* ADE9000 supports MODE 0 and 3 */
#define BITS (8)
#define SPEED (8000000) /* ADE9000 데이터시트상에서 최대 20Mhz까지 지원하고 있지만
						9.2Mhz 이상의 속도에서 rx data가 1bit씩 shift 되는 문제 있음 */

/* ADE9000 포트 번호 */
#define PORTA (0)
#define PORTB (1)
#define PORTC (2)

/* 실제 전류,전압,전력,전력량 계산에 사용 */
#define RB (10) /* 부담저항(burden resistor)값*/
#define R1 (990) /* 전압분배 회로 중 R1에 해당하는 저항값 */
#define R2 (1) /* 전압분배 회로 중 R2에 해당하는 저항값 */
#define CT_RATIO (1000) /* CT센서 비율 */
#define FULL_SCALE_RMS (52702092) /* xIRMS, xVRMS의 full-scale ADC값 */
#define FULL_SCALE_PWR (20694066) /* WATT, VAR, VA의 full-scale ADC값 */
#define EGY_SAMPLE (8000) /* 측정샘플 수 설정, Energy update rate가 8ksps이므로 측정 샘플이 8000이면 Energy update rate는 1초 */
#define MAX_CURRENT (0.7071 * CT_RATIO / RB)
#define MAX_VOLTAGE (0.7071 * (R1 + R2) / R2)
#define MAX_POWER (MAX_CURRENT * MAX_VOLTAGE)

static Spi_t spiADE9000 = {"/dev/spidev0.0", MODE, BITS, SPEED, -1};

static int initADE9000();
static int writeADE9000(int addr, int txData);
static int readADE9000(int addr);

static int current[] = {AIRMS, BIRMS, CIRMS};
static int voltage[] = {AVRMS, BVRMS, CVRMS};
static int activePower[] = {AWATT, BWATT, CWATT};
static int reactivePower[] = {AVAR, BVAR, CVAR};
static int apparentPower[] = {AVA, BVA, CVA};
static int activeEnergy[] = {AWATTHR_HI, BWATTHR_HI, CWATTHR_HI};

static float aWattHrRes = 0;
static float bWattHrRes = 0;
static float cWattHrRes = 0;

static float readCurrent(int channelIndex);
static float readVoltage(int channelIndex);
static float readActivePower(int channelIndex);
static float readReactivePower(int channelIndex);
static float readApparentPower(int channelIndex);
static float readActiveEnergy(int channelIndex);

int main(int argc, char **argv) {
	int i;
	char *setName;
	// 얼마의 시간 동안 데이터를 측정할지 설정 (단위 : 초)
	int sec = 20;

	if (argc == 1) {
		setName = NULL;
	} else {
		setName = argv[1];
	}

	RealValue_t portA, portB, portC;
	memset(&portA, 0, sizeof(RealValue_t));
	memset(&portB, 0, sizeof(RealValue_t));
	memset(&portC, 0, sizeof(RealValue_t));

	time_t getTime;

	initADE9000();
	File_open(setName);

	time(&getTime);
	printf("Starting time : %s\n", ctime(&getTime));

	while(sec > 0) {
		// STATUS0 레지스터의 EGYRDY(bits 0) 비트를 확인하여 인터럽트를 감지
		if ((readADE9000(STATUS0) & 0x1) == 0x1) {
			portA.current = readCurrent(PORTA);
			portA.voltage =	readVoltage(PORTA);
			portA.activePower = readActivePower(PORTA);
			portA.reactivePower = readReactivePower(PORTA);
			portA.apparentPower = readApparentPower(PORTA);
			portA.activeEnergy = readActiveEnergy(PORTA);

			portB.current = readCurrent(PORTB);
			portB.voltage =	readVoltage(PORTB);
			portB.activePower = readActivePower(PORTB);
			portB.reactivePower = readReactivePower(PORTB);
			portB.apparentPower = readApparentPower(PORTB);
			portB.activeEnergy = readActiveEnergy(PORTB);

			portC.current = readCurrent(PORTC);
			portC.voltage =	readVoltage(PORTC);
			portC.activePower = readActivePower(PORTC);
			portC.reactivePower = readReactivePower(PORTC);
			portC.apparentPower = readApparentPower(PORTC);
			portC.activeEnergy = readActiveEnergy(PORTC);

			File_append(&portA, &portB, &portC);

			// WATTHR 레지스터를 읽은 후 인터럽트 clear
			writeADE9000(STATUS0, 0x1);
			sec--;
		}
	}

	printf("Finished!\n");

	File_close();
	spi_term(&spiADE9000);

	return 0;
}

/**
 * ADE9000 초기화
 */
int initADE9000() {
	char versionBuf[BYTE(VERSION)];

	spi_init(&spiADE9000);

	// 버전 확인
	readADE9000(VERSION);
	// RUN register ON
	writeADE9000(RUN, 0x1);
	// ACCMODE 레지스터의 8번째 비트(SELFREQ) 설정 -> 0 : 50Hz 시스템 사용 시 / 1 : 60Hz 시스템 사용 시
	writeADE9000(ACCMODE, 0x0100);
	// 인터럽트 기능을 사용하기 위한 설정, nIRQ0핀에서 확인 가능
	writeADE9000(MASK0, 0x1);
	// 전력량 측정을 위한 설정 레지스터
	writeADE9000(EP_CFG, 0x31);
	// 전력량을 한번 측정할 때 얼마나 많은 샘플을 사용할 것인지 설정
	writeADE9000(EGY_TIME, EGY_SAMPLE);

	return 0;
}

/**
 * ADE9000 Write
 */
int writeADE9000(int addr, int txData) {
	int i;
	char txBuf[BYTE(addr)];
	memset(txBuf, 0, sizeof(txBuf));

	// CMD_HDR[15:4] : addr || CMD_HDR[3] : 0 (write) || CMD_HDR[2:0] : 000
	int writeAddr = addr << 4;

	// register 주소값 + 쓰기 모드
	txBuf[0] = (writeAddr >> 8) & 0xFF;
	txBuf[1] = writeAddr & 0xFF;

	// 원하는 데이터 쓰기
	for (i = BYTE(addr) - 1; i > 1; i--) {
		txBuf[i] = txData & 0xFF;
		txData >>= 8;
	}

	spi_write(&spiADE9000, txBuf, BYTE(addr));

	return 0;
}

/**
 * ADE9000 Read
 */
int readADE9000(int addr) {
	int i;
	int res;
	int rxValue = 0;
	char txBuf[BYTE(addr)];
	char rxBuf[BYTE(addr)];
	memset(txBuf, 0, sizeof(txBuf));
	memset(rxBuf, 0, sizeof(rxBuf));

	// CMD_HDR[15:4] : addr || CMD_HDR[3] : 1 (read) || CMD_HDR[2:0] : 000
	int readAddr = (addr << 4) | 0x8;

	// register 주소값 + 읽기 모드
	txBuf[0] = (readAddr >> 8) & 0xFF;
	txBuf[1] = readAddr & 0xFF;

	spi_simWriteRead(&spiADE9000, txBuf, rxBuf, BYTE(addr));

	// rxBuf[2]부터 유효한 데이터값
	for (i = BYTE(addr) - 1; i > 1; i--) {
		res = rxBuf[i] << (8 * (BYTE(addr) - 1 - i));
		rxValue |= res;
	}

	return rxValue;
}

/**
 * Read real current value
 */
float readCurrent(int channelIndex) {
	int res = readADE9000(current[channelIndex]);

	return (MAX_CURRENT * res / FULL_SCALE_RMS);
}

/**
 * Read real voltage value
 */
float readVoltage(int channelIndex) {
	int res = readADE9000(voltage[channelIndex]);

	return (MAX_VOLTAGE * res / FULL_SCALE_RMS);
}

/**
 * Read real active power value
 */
float readActivePower(int channelIndex) {
	int res = readADE9000(activePower[channelIndex]);

	return (MAX_POWER * res / FULL_SCALE_PWR);
}

/**
 * Read real reactive power value
 */
float readReactivePower(int channelIndex) {
	int res = readADE9000(reactivePower[channelIndex]);

	return (MAX_POWER * res / FULL_SCALE_PWR);
}

/**
 * Read real apparent power value
 */
float readApparentPower(int channelIndex) {
	int res = readADE9000(apparentPower[channelIndex]);

	return (MAX_POWER * res / FULL_SCALE_PWR);
}

/**
 * Read real active energy value
 */
float readActiveEnergy(int channelIndex) {
	int res = readADE9000(activeEnergy[channelIndex]);
	float realValue = (MAX_POWER * res) / (FULL_SCALE_PWR * 3600.0);
	float wattHrRes;

	switch (channelIndex) {
		case PORTA:
			aWattHrRes += realValue;
			wattHrRes = aWattHrRes;
			break;
		case PORTB:
			bWattHrRes += realValue;
			wattHrRes = bWattHrRes;
			break;
		case PORTC:
			cWattHrRes += realValue;
			wattHrRes = cWattHrRes;
			break;
	}

	return wattHrRes;
}