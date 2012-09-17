#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_i2s.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pinsel.h"

#include "terminal.h"


volatile uint32_t SysTickCount;
#define TXFIFO_FULL			(8)

I2S_CFG_Type I2S_TransConf = {
	I2S_WORDWIDTH_16,
	I2S_STEREO,
	I2S_STOP_ENABLE,
	I2S_RESET_ENABLE,
	I2S_MASTER_MODE,
	I2S_MUTE_DISABLE,
};

I2S_MODEConf_Type I2S_ClkConfig;



void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
}

inline static void delay_ms(uint32_t delayTime) {
	uint32_t currentSysTickCount;

	currentSysTickCount = SysTickCount;
	while ((SysTickCount - currentSysTickCount) < delayTime)
		;
}

void I2S_IRQHandler(void);

void I2S_IRQHandler()
{
	uint32_t RXLevel = 0;

	//Check RX interrupt
	if(I2S_GetIRQStatus(LPC_I2S, I2S_TX_MODE))
	{
		int i = LPC_I2S->I2SRXFIFO;
	}
	return;
}


int main() {

	SystemInit();
	SystemCoreClockUpdate();

	SysTick_Config(SystemCoreClock / 1000);

	PINSEL_CFG_Type PinCfg;
	/* Configure pins for I2S
	 * 		P0.4 as I2SRX_CLK
	 * 			- P0.5 as I2SRX_WS
	 * 			- P0.6 as I2SRX_SDA
	 * 			- P0.7 as I2STX_CLK
	 * 			- P0.8 as I2STX_WS
	 * 			- P0.9 as I2STX_SDA
	 */
		PinCfg.Funcnum = 1;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Pinnum = 4;
		PinCfg.Portnum = 0;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 5;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 6;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 7;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 8;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 9;
		PINSEL_ConfigPin(&PinCfg);



	/* Enable I2S interrupts */
	NVIC_EnableIRQ(I2S_IRQn);
	unsigned long LED_PINS = ((uint32_t) 1 << 22);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	/* Uart Init */
	uart1Init();

	/* I2S init */
	I2S_Init(LPC_I2S);
	I2S_Config(LPC_I2S, I2S_TX_MODE, &I2S_TransConf);
	I2S_Stop(LPC_I2S, I2S_TX_MODE);
	I2S_FreqConfig(LPC_I2S, 48000, I2S_TX_MODE);
	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
	I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
	I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
	//I2S_IRQConfig(LPC_I2S,I2S_TX_MODE,8);
	I2S_Start(LPC_I2S);
	/* I2S transmit ---------------------------------------------------*/
	unsigned short test = 1;
	GPIO_SetValue(0, LED_PINS);
		while (1)
		{
			I2S_Send(LPC_I2S,test | (test << 16));
			test++;
			delay_ms(1000);
			term1PutValue(test,TRUE);
		}

	//	I2STXDone = 1;

}
