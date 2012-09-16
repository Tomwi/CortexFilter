#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"

int main(void) {
	SystemInit();

	int i, j;
	unsigned long LED_PINS = ((uint32_t) 1 << 22);

	/* Enable GPIO Clock */
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	GPIO_SetDir(0, LED_PINS, 1);

	while (1) {
		/* Delay some time */
		for (i = 500; i > 0; i--)
			for (j = 10000; j > 0; j--) {
			}
		/* Output low level  */
		GPIO_ClearValue(0, LED_PINS);
		/* Delay some time */
		for (i = 500; i > 0; i--)
			for (j = 10000; j > 0; j--) {
			}
		/* Output high level */
		GPIO_SetValue(0, LED_PINS);
	}
}
