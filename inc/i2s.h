#ifndef T_I2S_H
#define T_I2S_H

#include "lpc17xx_i2s.h"
#include "lpc17xx_pinsel.h"

void initI2S(void);
void initTX(unsigned int freq);
void TransmitValue(unsigned int val);

#endif
