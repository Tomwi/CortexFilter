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
#include "fir.h"

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

volatile uint32_t SysTickCount;
volatile uint32_t miliseconds = 0;

volatile uint32_t buffer1[TRANSFER_SIZE];
volatile uint32_t buffer2[TRANSFER_SIZE];
volatile uint32_t buffer3[TRANSFER_SIZE];

volatile uint8_t needsProcessing = 0;

volatile uint8_t txReady = 0, rxReady = 0;
volatile uint32_t *txActive, *rxActive, *processActive;

volatile int ledvalue = 0;


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

			firFilter(processActive, processActive, TRANSFER_SIZE);

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
