#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

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

	if (miliseconds >= 500){
		miliseconds = 0;


		txdata++;

		if (txdata >= 65535){
			txdata = 0;
		}
		txblock[0] = txblock[1] = txblock[2] = txblock[3] = txdata;

		term1PutValue(txdata,TRUE);

		if (ledvalue){
			GPIO_SetValue(0, LED_PINS);
			ledvalue = 0;
		}else{
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

void DMA_IRQHandler(void){

	;
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




	GPDMA_Init();



	GPDMA_Channel_CFG_Type DMACfg;
	DMACfg.ChannelNum = 0;
	DMACfg.TransferSize = 4;
	DMACfg.TransferWidth = GPDMA_WIDTH_WORD;
	DMACfg.SrcMemAddr = txblock;
	DMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	DMACfg.SrcConn = GPDMA_CONN_I2S_Channel_0;
	DMACfg.DMALLI = 0;


	//GPDMA_Setup(&DMACfg);
	//GPDMA_ChannelCmd(0,ENABLE);



	while(1){
		//TransmitValue((txdata)|(txdata<<16));
		;


	}

}
