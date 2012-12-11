#ifndef T_I2S_H
#define T_I2S_H

#include "lpc_types.h"
#include "lpc17xx_i2s.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

#define TRANSFER_SIZE (256)

void initI2SDMA(uint32_t, uint32_t);
void initTX(unsigned int, uint32_t, uint32_t);

#endif
