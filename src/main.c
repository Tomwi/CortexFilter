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

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
	miliseconds++;

	/*
	 if (miliseconds >= 500) {
	 miliseconds = 0;

	 if (ledvalue) {
	 GPIO_SetValue(0, (1 << 22));
	 ledvalue = 0;
	 } else {
	 GPIO_ClearValue(0, (1 << 22));
	 ledvalue = 1;
	 }
	 }
	 */
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

			for (i = 0; i < TRANSFER_SIZE; i++) {
				txActive[i] = rxActive[i];
			}

			//term1PutValue( (uint16_t)( rxActive[0] & 0xffff), 1);

			needsProcessing = 0;
			GPIO_ClearValue(0, (1 << 22));
		}
	}
}
