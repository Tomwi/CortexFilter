#ifndef T_I2S_H
#define T_I2S_H

#include "lpc17xx_i2s.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc_types.h"
#include "lpc17xx_gpdma.h"

#define TRANSFER_SIZE (1024)



#define I2S_STATE (*(volatile uint32_t*)0x400A8010)

void initI2SDMATX(uint32_t);
void initI2SDMARX(uint32_t);
void initI2S(void);
void initTX(unsigned int, uint32_t, uint32_t);
void TransmitValue(unsigned int);

#endif
