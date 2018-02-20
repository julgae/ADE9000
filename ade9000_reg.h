/**
 * ade9000 register header
 */
#ifndef _ADE9000_REG_H_
#define _ADE9000_REG_H_

/* 데이터 크기 : 4 or 6byte (2byte(주소값+쓰기/읽기모드) + 2 or 4byte(쓸/읽을 데이터)) */
#define BYTE(ADDR) ((ADDR >= 0x480) && (ADDR <= 0x4FE) ? (4) : (6))

/* Register address */
/* 공통 */
#define VERSION 0x4FE
#define RUN 0x480 /* Write this register to 1 to start the measurements */
#define ACCMODE 0x492 /* Accumulation mode register. Bit8(SELFREQ) -> 0 : 50Hz / 1 : 60Hz */
#define NIRMS 0x266 /* Neutral current filter-based rms value */
#define EP_CFG 0x4B0 /* Energy and power accumulation configuration */
#define EGY_TIME 0x4B2 /* Energy accumulation update time configuration */
#define STATUS0 0x402 /* Status Register 0 */
#define MASK0 0x405 /* Interrupt Enable Register 0 */
/* A포트 */
#define AIRMS 0x20C /* 전류RMS */
#define AVRMS 0x20D /* 전압RMS */
#define AWATT 0x210 /* Active power 유효전력 */
#define AVAR 0x211 /* Reactive power 무효전력 */
#define AVA 0x212 /* Apparent power 피상전력 */
#define APF 0x216 /* Power factor 역률 */
#define AWATTHR_HI 0x2E7 /* Active energy 유효전력량 */
/* B포트 */
#define BIRMS 0x22C
#define BVRMS 0x22D
#define BWATT 0x230
#define BVAR 0x231
#define BVA 0x232
#define BPF 0x236
#define BWATTHR_HI 0x323
/* C포트 */
#define CIRMS 0x24C
#define CVRMS 0x24D
#define CWATT 0x250
#define CVAR 0x251
#define CVA 0x252
#define CPF 0x256
#define CWATTHR_HI 0x35F

#endif