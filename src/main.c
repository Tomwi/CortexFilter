#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_libcfg_default.h"
#include "lpc17xx_nvic.h"

#include "terminal.h"
#include "i2s.h"

// maximum number of inputs that can be handled
// in one function call
#define MAX_INPUT_LEN   TRANSFER_SIZE
// maximum length of filter than can be handled
#define MAX_FLT_LEN     128
// buffer to hold all of the input samples
#define BUFFER_LEN      (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)
#define FILTER_LEN  100

#define FILTERED 100
#define ORIGINAL 50

// array to hold input samples
int16_t insamp[BUFFER_LEN];
int16_t coeffs[FILTER_LEN] = { 11, 10, 8, 6, 4, 1, -2, -6, -12, -19, -27, -36,
		-46, -57, -68, -80, -92, -102, -112, -119, -124, -125, -121, -113, -99,
		-79, -52, -17, 24, 73, 130, 195, 267, 344, 428, 516, 608, 701, 796, 890,
		982, 1070, 1153, 1229, 1297, 1356, 1404, 1441, 1467, 1479, 1479, 1467,
		1441, 1404, 1356, 1297, 1229, 1153, 1070, 982, 890, 796, 701, 608, 516,
		428, 344, 267, 195, 130, 73, 24, -17, -52, -79, -99, -113, -121, -125,
		-124, -119, -112, -102, -92, -80, -68, -57, -46, -36, -27, -19, -12, -6,
		-2, 1, 4, 6, 8, 10, 11 };

volatile uint32_t SysTickCount;
volatile uint32_t miliseconds = 0;

volatile uint32_t buffer1[TRANSFER_SIZE];
volatile uint32_t buffer2[TRANSFER_SIZE];
volatile uint32_t buffer3[TRANSFER_SIZE];

volatile uint8_t needsProcessing = 0;

volatile uint8_t txReady = 0, rxReady = 0;
volatile uint32_t *txActive, *rxActive, *processActive;

volatile int ledvalue = 0;

void DMA_IRQHandler(void);

// FIR init
void firFixedInit(void) {
	memset(insamp, 0, sizeof(insamp));
}

// the FIR filter function
void firFixed(int16_t *coeffs, int16_t *input, int16_t *output, int length,
		int filterLength) {
	int32_t acc; // accumulator for MACs
	int16_t *coeffp; // pointer to coefficients
	int16_t *inputp; // pointer to input samples
	int n;
	int k;

	// put the new samples at the high end of the buffer
	memcpy(&insamp[filterLength - 1], input, length * sizeof(int16_t));

	// apply the filter to each input sample
	for (n = 0; n < length; n++) {
		// calculate output n
		coeffp = coeffs;
		inputp = &insamp[filterLength - 1 + n];
		// load rounding constant
		acc = 1 << 14;
		// perform the multiply-accumulate
		for (k = 0; k < filterLength; k++) {
			acc += (int32_t)(*coeffp++) * (int32_t)(*inputp--);
		}
		// saturate the result
		if (acc > 0x3fffffff) {
			acc = 0x3fffffff;
		} else if (acc < -0x40000000) {
			acc = -0x40000000;
		}
		// convert from Q30 to Q15
		output[n] = (output[n] * ORIGINAL) / 100
				+ ((int16_t)(acc >> 15) * FILTERED) / 100;
	}

	// shift input samples back in time for next time
	memmove(&insamp[0], &insamp[length], (filterLength - 1) * sizeof(int16_t));
}

void DMA_IRQHandler(void) {

	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) == SET) {
		//term1PutText("DMA channel: TX\r\n");
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);

			txReady = 1;
			rxReady = 1;

			if (txReady && rxReady) {
				txReady = 0;
				rxReady = 0;
				initI2SDMA((uint32_t) txActive, (uint32_t) rxActive);

				if (needsProcessing)
					term1PutText("Too late :(\r\n");
				needsProcessing = 1;

			}
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}

	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1) == SET) {
		//term1PutText("DMA channel: RX\r\n");
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);

			rxReady = 1;

			if (txReady && rxReady) {
				txReady = 0;
				rxReady = 0;
				initI2SDMA((uint32_t) txActive, (uint32_t) rxActive);

				if (needsProcessing)
					term1PutText("Too late :(\r\n");

				needsProcessing = 1;
			}
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 1);
		}
	}

}

int main() {
	SystemInit();
	SystemCoreClockUpdate();

	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, ENABLE);

	GPIO_SetDir(0, (1 << 22), 1);
	GPIO_SetDir(1, (1 << 18) | (1 << 21), 1);

	uart1Init();

	firFixedInit();

	processActive = buffer3;
	rxActive = buffer1;
	txActive = buffer2;

	initTX(44100, (uint32_t) txActive, (uint32_t) rxActive);

	term1PutText("Booted\n\r");

	GPIO_SetValue(1, (1 << 18));
	GPIO_ClearValue(1, (1 << 21));

	while (1) {
		if (needsProcessing) {
			GPIO_SetValue(0, (1 << 22));

			uint32_t i;

			int16_t samples[TRANSFER_SIZE];


			for (i = 0; i < TRANSFER_SIZE; i++) {
				samples[i] = (processActive[i] >> 16);

			}

			firFixed(coeffs, samples, samples, TRANSFER_SIZE, FILTER_LEN);


			for (i = 0; i < TRANSFER_SIZE; i++) {
				processActive[i] = samples[i];
			}

			//rotate buffers
			volatile uint32_t *tmp = processActive;
			processActive = rxActive;
			rxActive = txActive;
			txActive = tmp;

			needsProcessing = 0;
			GPIO_ClearValue(0, (1 << 22));
		}
	}
}
