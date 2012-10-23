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
volatile uint32_t txblock[TRANSFER_SIZE];
volatile uint32_t rxblock[TRANSFER_SIZE];
volatile int16_t i = 0;

int16_t LUT[] = { 0, 4771, 9440, 13908, 18080, 21866, 25187, 27970, 30157, 31702,
		32571, 32746, 32222, 31012, 29141, 26649, 23589, 20026, 16037, 11705,
		7124, 2391, -2391, -7124, -11705, -16037, -20026, -23589, -26649,
		-29141, -31012, -32222, -32746, -32571, -31702, -30157, -27970, -25187,
		-21866, -18080, -13908, -9440, -4771};
volatile int ledvalue = 0;
unsigned long LED_PINS = ((uint32_t) 1 << 22);

void DMA_IRQHandler(void);

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
	miliseconds++;

	if (miliseconds >= 500) {
		miliseconds = 0;

		if (ledvalue) {
			GPIO_SetValue(0, LED_PINS);
			ledvalue = 0;
		} else {
			GPIO_ClearValue(0, LED_PINS);
			ledvalue = 1;
		}

	}
}

inline static void delay_ms(uint32_t delayTime) {
	uint32_t currentSysTickCount;

	currentSysTickCount = SysTickCount;
	while ((SysTickCount - currentSysTickCount) < delayTime)
		;
}

void DMA_IRQHandler(void) {

	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) == SET) {
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);

			initI2SDMATX((uint32_t) txblock);
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}

	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1) == SET) {
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);

			//initI2SDMARX((uint32_t) txblock);
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 1);
		}
	}

}

int main() {

	SystemInit();
	SystemCoreClockUpdate();

	//txblock init
	int j;
	for (j = 0; j < TRANSFER_SIZE; j++) {
		i = LUT[j];
		txblock[j] = ((i << 16) | i);
	}

	SysTick_Config(SystemCoreClock / 1000);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	uart1Init();

	initTX(44100, (uint32_t) txblock, (uint32_t) txblock);

	term1PutText("Booted, PCLK:\n\r");
	term1PutValue(CLKPWR_GetPCLK(CLKPWR_PCLKSEL_I2S), 1);

	while (1) {
		;
	}
}
