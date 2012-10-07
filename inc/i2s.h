#ifndef T_I2S_H
#define T_I2S_H

#include "lpc17xx_i2s.h"
#include "lpc17xx_pinsel.h"
#include "lpc_types.h"
#include "lpc17xx_gpdma.h"



#define I2S_STATE (*(volatile uint32_t*)0x400A8010)

void initI2S(void);
void initTX(unsigned int freq, uint32_t txblock, volatile uint32_t * TC, volatile uint32_t * Err);
void TransmitValue(unsigned int val);

#endif
