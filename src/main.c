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
volatile uint32_t txdata = 0;
volatile uint32_t txblock[4];
volatile int ledvalue = 0;
unsigned long LED_PINS = ((uint32_t) 1 << 22);

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
	miliseconds++;

	if (miliseconds >= 500) {
		miliseconds = 0;

		txdata++;

		if (txdata >= 65535) {
			txdata = 0;
		}
		txblock[0] = txblock[1] = txblock[2] = txblock[3] = txdata;

		term1PutValue(txdata, TRUE);

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

void DMA_IRQHandler(void)
{
	if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) == SET)
	{
		if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0) == SET)
		{
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
		}
		if(GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET)
		{
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}
}

int main() {

	SystemInit();
	SystemCoreClockUpdate();

	SysTick_Config(SystemCoreClock / 1000);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	uart1Init();
	initTX(44100);

	GPDMA_Channel_CFG_Type GPDMACfg;

	/* GPDMA block section -------------------------------------------- */
	/* Disable GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));

	/* Initialize GPDMA controller */
	GPDMA_Init();

	/* Setup GPDMA channel --------------------------------*/
	/* channel 0 */
	GPDMACfg.ChannelNum = 0;
	/* Source memory */
	GPDMACfg.SrcMemAddr = (uint32_t) txblock;
	/* Destination memory */
	GPDMACfg.DstMemAddr = 0;
	/* Transfer size */
	GPDMACfg.TransferSize = 4;
	/* Transfer width */
	GPDMACfg.TransferWidth = GPDMA_WIDTH_WORD;
	/* Transfer type */
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	/* Source connection - unused */
	GPDMACfg.SrcConn = 0;
	/* Destination connection - I2S */
	GPDMACfg.DstConn = GPDMA_CONN_I2S_Channel_0;
	/* Linker List Item - unused */
	GPDMACfg.DMALLI = 0;
	/* Setup channel with given parameter */
	GPDMA_Setup(&GPDMACfg);
	/* Enable GPDMA channel 0 */
	GPDMA_ChannelCmd(0, ENABLE);
	/* Enable GPDMA interrupt */
	NVIC_EnableIRQ(DMA_IRQn);

	while (1) {
		//TransmitValue((txdata)|(txdata<<16));
		;

	}

}
