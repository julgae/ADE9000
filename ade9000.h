/**
 * ade9000 header
 */
#ifndef _ADE9000_H_
#define _ADE9000_H_

typedef struct RealValue_s {
	float current;
	float voltage;
	float activePower;
	float reactivePower;
	float apparentPower;
	float activeEnergy;
} RealValue_t;

#endif