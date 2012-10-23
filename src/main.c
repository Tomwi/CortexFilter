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

int16_t LUT[] = { 0, 4663, 9231, 13611, 17715, 21457, 24763, 27565, 29805,
		31439, 32433, 32767, 32433, 31439, 29805, 27565, 24763, 21457, 17715,
		13611, 9231, 4663, 0, -4663, -9231, -13611, -17715, -21457, -24763,
		-27565, -29805, -31439, -32433, -32767, -32433, -31439, -29805, -27565,
		-24763, -21457, -17715, -13611, -9231, -4663 };

volatile int ledvalue = 0;

void DMA_IRQHandler(void);

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
	miliseconds++;

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

			initI2SDMATX((uint32_t) txblock);
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}

	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1) == SET) {
		//term1PutText("DMA channel: RX\r\n");
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);

			initI2SDMARX((uint32_t) rxblock);
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

	//txblock init
	int16_t j, i = 0;
	for (j = 0; j < TRANSFER_SIZE; j++) {
		i = LUT[j];
		txblock[j] = ((i << 16) | (i & 0xffff));
		//term1PutValue(j,0);
		//term1PutValue(i,1);
	}


	initTX(44100, (uint32_t) txblock, (uint32_t) txblock);
	term1PutText("Booted\n\r");

	GPIO_SetValue(1, (1 << 18));
	GPIO_ClearValue(1, (1 << 21));

	term1PutValue(rxblock[0], 0);
int flag = 0;

delay_ms(100);
	while (1) {
		/*uint32_t sample = I2S_Receive(LPC_I2S);
		int16_t right = sample >> 16;
		int16_t left = sample & 0xffff;
		term1PutValue(right, 0);
		term1PutValue(left, 1);
		*/
		while(flag < TRANSFER_SIZE){
			//int left = rxblock[flag] & (0xffff);
			int16_t right = ((int)rxblock[flag]) >> 16;
			//term1PutValue(left, 0);
			term1PutValue(right, 0);
			flag++;
		}

		delay_ms(100);
	}
}
