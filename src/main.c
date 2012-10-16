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
volatile uint16_t i = 0;

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
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}
	
	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1) == SET) {
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 1);
		}
	}

	//txblock init
	int j;
	for (j = 0; j < TRANSFER_SIZE; j++) {
		txblock[j] = ((i << 16) | i);
		i++;
	}

	initI2SDMA((uint32_t) txblock, (uint32_t) rxblock);

}

int main() {

	SystemInit();
	SystemCoreClockUpdate();

	//txblock init
	int j;
	for (j = 0; j < TRANSFER_SIZE; j++) {
		txblock[i] = ((i << 16) | i);
		i++;
	}

	SysTick_Config(SystemCoreClock / 1000);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	uart1Init();

	initTX(44100, (uint32_t) txblock, (uint32_t) txblock);

	term1PutText("Booted\n\r");
	while (1) {

		//TransmitValue((123)|((123)<<16));

		if (I2S_STATE & (I2S_STATE_DMA1 | I2S_STATE_DMA2)) {
			term1PutText("I2S DMA Request occurred\n\r");
		}
	}
}
