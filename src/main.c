#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"

#include "i2s.h"
#include "fir.h"

//Transfer buffer
uint32_t buffer1[TRANSFER_SIZE];
uint32_t buffer2[TRANSFER_SIZE];
uint32_t buffer3[TRANSFER_SIZE];

//Flag to indicate that processing is necessary
volatile uint8_t needsProcessing = 0;

//Flags to indicate that transmitting or receiving is finished
volatile uint8_t txReady = 0, rxReady = 0;

//Pointers to active buffers
uint32_t *txActive, *rxActive, *processActive;

//Interrupt that is triggered when the the DMA is finished
void DMA_IRQHandler(void) {
	//Check which channel triggered the interrupt

	//Transmit
	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) == SET) {
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0) == SET) {
			//Clear interrupt flag
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);

			//Set transit ready flag
			txReady = 1;

			//If both transmit and receive are finished
			if (txReady && rxReady) {
				//Clear flags
				txReady = 0;
				rxReady = 0;
				//Restart DMA
				initI2SDMA((uint32_t) txActive, (uint32_t) rxActive);

				//Set flag for processing in main loop
				needsProcessing = 1;
			}
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
		}
	}
	//Receive
	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1) == SET) {
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);

			//Set receive flag
			rxReady = 1;

			//If both transmit and receive are finished
			if (txReady && rxReady) {
				//Clear flags
				txReady = 0;
				rxReady = 0;
				//Restart DMA
				initI2SDMA((uint32_t) txActive, (uint32_t) rxActive);

				//Set flag for processing in main loop
				needsProcessing = 1;
			}
		}
		if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1) == SET) {
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 1);
		}
	}

}

int main() {
	//Initialize system and clocks
	SystemInit();
	SystemCoreClockUpdate();

	//Turn on peripheral clocks for GPIO and I2S
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, ENABLE);

	//Set direction for LED pin
	GPIO_SetDir(0, (1 << 22), 1);
	//Set direction for ADC control pins
	GPIO_SetDir(1, (1 << 18) | (1 << 21), 1);

	//Initialize buffer pointers to default value
	processActive = buffer3;
	rxActive = buffer1;
	txActive = buffer2;

	//Init the I2S hardware
	initTX(44100, (uint32_t) txActive, (uint32_t) rxActive);

	//Set and Clear control pins for ADC
	GPIO_SetValue(1, (1 << 18));
	GPIO_ClearValue(1, (1 << 21));

	//infinite loop
	while (1) {
		//If the interrupt has set the flag
		if (needsProcessing) {
			//Turn led on to indicate CPU usage
			GPIO_SetValue(0, (1 << 22));

			//Run filter on current buffers
			firFixed(processActive, TRANSFER_SIZE);

			//Rotate buffers
			uint32_t *tmp = processActive;
			processActive = rxActive;
			rxActive = txActive;
			txActive = tmp;

			//Clear flag
			needsProcessing = 0;

			//Turn led off, if the processing takes longer the LED becomes brighter
			GPIO_ClearValue(0, (1 << 22));
		}
	}
}
