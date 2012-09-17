#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pinsel.h"

#include "terminal.h"
#include "i2s.h"

volatile uint32_t SysTickCount;

void SysTick_Handler(void) {
	SysTickCount++; // increment the SysTick counter
}

inline static void delay_ms(uint32_t delayTime) {
	uint32_t currentSysTickCount;

	currentSysTickCount = SysTickCount;
	while ((SysTickCount - currentSysTickCount) < delayTime)
		;
}

int main() {

	SystemInit();
	SystemCoreClockUpdate();

	SysTick_Config(SystemCoreClock / 1000);
	unsigned long LED_PINS = ((uint32_t) 1 << 22);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	/* Uart Init */
	uart1Init();
	initI2S();
	initTX(44100);

	int val = 0;
	for(;;val++){
		delay_ms(500);
		GPIO_ClearValue(0, LED_PINS);
		delay_ms(500);
		GPIO_SetValue(0, LED_PINS);
		TransmitValue((val)|(val<<16));
	}

}
