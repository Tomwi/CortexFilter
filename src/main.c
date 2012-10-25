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
int16_t insampL[BUFFER_LEN];
int16_t insampR[BUFFER_LEN];
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

volatile uint32_t txBlock1[TRANSFER_SIZE];
volatile uint32_t rxBlock1[TRANSFER_SIZE];
volatile uint32_t txBlock2[TRANSFER_SIZE];
volatile uint32_t rxBlock2[TRANSFER_SIZE];

volatile uint8_t currentBuffer = 1;
volatile uint8_t needsProcessing = 0;

volatile uint8_t txReady, rxReady;

volatile uint32_t *txActive, *rxActive;

volatile int ledvalue = 0;

void DMA_IRQHandler(void);

// FIR init
void firFixedLInit(void) {
	memset(insampL, 0, sizeof(insampL));
}

// the FIR filter function
void firFixedL(int16_t *coeffs, int16_t *input, int16_t *output, int length,
		int filterLength) {
	int32_t acc; // accumulator for MACs
	int16_t *coeffp; // pointer to coefficients
	int16_t *inputp; // pointer to input samples
	int n;
	int k;

	// put the new samples at the high end of the buffer
	memcpy(&insampL[filterLength - 1], input, length * sizeof(int16_t));

	// apply the filter to each input sample
	for (n = 0; n < length; n++) {
		// calculate output n
		coeffp = coeffs;
		inputp = &insampL[filterLength - 1 + n];
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
	memmove(&insampL[0], &insampL[length],
			(filterLength - 1) * sizeof(int16_t));
}

// FIR init
void firFixedRInit(void) {
	memset(insampR, 0, sizeof(insampR));
}

// the FIR filter function
void firFixedR(int16_t *coeffs, int16_t *input, int16_t *output, int length,
		int filterLength) {
	int32_t acc; // accumulator for MACs
	int16_t *coeffp; // pointer to coefficients
	int16_t *inputp; // pointer to input samples
	int n;
	int k;

	// put the new samples at the high end of the buffer
	memcpy(&insampR[filterLength - 1], input, length * sizeof(int16_t));

	// apply the filter to each input sample
	for (n = 0; n < length; n++) {
		// calculate output n
		coeffp = coeffs;
		inputp = &insampR[filterLength - 1 + n];
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
	memmove(&insampR[0], &insampR[length],
			(filterLength - 1) * sizeof(int16_t));
}

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
	miliseconds++;
}

inline static void delay_ms(uint32_t delayTime) {
	uint32_t currentSysTickCount;

	currentSysTickCount = SysTickCount;
	while ((SysTickCount - currentSysTickCount) < delayTime)
		;
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

void deInterleave(void* in, void* out, int len);

int main() {

	SystemInit();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, ENABLE);

	GPIO_SetDir(0, (1 << 22), 1);
	GPIO_SetDir(1, (1 << 18) | (1 << 21), 1);

	uart1Init();

	firFixedLInit();
	firFixedRInit();

	rxReady = 0;
	rxReady = 0;
	txActive = txBlock1;
	rxActive = rxBlock1;
	initTX(44100, (uint32_t) txActive, (uint32_t) rxActive);

	term1PutText("Booted\n\r");

	GPIO_SetValue(1, (1 << 18));
	GPIO_ClearValue(1, (1 << 21));

	while (1) {
		if (needsProcessing) {
			GPIO_SetValue(0, (1 << 22));

			if (currentBuffer == 1) {
				currentBuffer = 2;

				txActive = txBlock2;
				rxActive = rxBlock2;
			} else {
				currentBuffer = 1;

				txActive = txBlock1;
				rxActive = rxBlock1;
			}

			uint32_t i;

			int16_t left[TRANSFER_SIZE];
			int16_t right[TRANSFER_SIZE];

			for (i = 0; i < TRANSFER_SIZE; i++) {
				left[i] = rxActive[i] & (0xffff);
				right[i] = ((int) rxActive[i]) >> 16;
			}

			firFixedL(coeffs, left, left, TRANSFER_SIZE, FILTER_LEN);
			firFixedR(coeffs, right, right, TRANSFER_SIZE, FILTER_LEN);

			for (i = 0; i < TRANSFER_SIZE; i++) {
				txActive[i] = ((right[i] << 16) | (left[i] & 0xffff));
			}

			needsProcessing = 0;
			GPIO_ClearValue(0, (1 << 22));
		}
	}
}
