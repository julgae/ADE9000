/**
 * 전력량 관련 데이터를 csv 형식의 파일에 저장하는 기능
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <linux/limits.h>
#include "csv_file.h"

static int count = 0;
static FILE *fp = NULL;

int File_open(char *fileName) {
	char csvName[PATH_MAX];

	if (fileName == NULL) {
		time_t now;
		struct tm *date;
		now = time(NULL);
		date = localtime(&now);

		sprintf(csvName, "ADE9000_%.4d%.2d%.2d-%.2d%.2d%.2d.csv",
			date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
			date->tm_hour, date->tm_min, date->tm_sec);
	} else {
		strcpy(csvName, fileName);
		strcat(csvName, ".csv");
	}

	fp = fopen(csvName, "w");

	if (fp == NULL) {
		printf("File open failed\n");
		return -1;
	}

	fprintf(fp, "second,A-Voltage(V),A-Current(A),A-Power(W),A-VAR,A-VA,A-Energy(Wh),\
		B-Voltage(V),B-Current(A),B-Power(W),B-VAR,B-VA,B-Energy(Wh),\
		C-Voltage(V),C-Current(A),C-Power(W),C-VAR,C-VA,C-Energy(Wh)\n");

	return 0;
}

int File_close() {
	time_t getTime;

	time(&getTime);
	fprintf(fp, "Finish : %s\n", ctime(&getTime));

	fclose(fp);
	fp = NULL;

	count = 0;

	return 0;
}

int File_append(RealValue_t *portA, RealValue_t *portB, RealValue_t *portC) {
	if (fp == NULL) {
		printf("File append failed\n");
		return -1;
	}

	fprintf(fp, "%d,%.2f,%.4f,%.2f,%.2f,%.2f,%.3f,\
		%.2f,%.4f,%.2f,%.2f,%.2f,%.3f,\
		%.2f,%.4f,%.2f,%.2f,%.2f,%.3f\n",
		count, portA->voltage, portA->current, portA->activePower,
		portA->reactivePower, portA->apparentPower, portA->activeEnergy,
		portB->voltage, portB->current, portB->activePower,
		portB->reactivePower, portB->apparentPower, portB->activeEnergy,
		portC->voltage, portC->current, portC->activePower,
		portC->reactivePower, portC->apparentPower, portC->activeEnergy);

	count++;

	return 0;
}