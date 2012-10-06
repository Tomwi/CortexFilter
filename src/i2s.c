#include "i2s.h"

I2S_MODEConf_Type I2S_ClkConfig;

void initTX(unsigned int freq) {
	/* Configure pins for I2S Transmitter */

	/* Configure pins for I2S
	 * 		P0.4 as I2SRX_CLK
	 * 			- P0.5 as I2SRX_WS
	 * 			- P0.6 as I2SRX_SDA
	 * 			- P0.7 as I2STX_CLK
	 * 			- P0.8 as I2STX_WS
	 * 			- P0.9 as I2STX_SDA
	 */
	PINSEL_CFG_Type PinCfg;

	PinCfg.Portnum = PINSEL_PORT_0;
	PinCfg.Funcnum = PINSEL_FUNC_1;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;

	PinCfg.Pinnum = PINSEL_PIN_4; // P0.4 as I2SRX_CLK
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = PINSEL_PIN_5; // P0.5 as I2SRX_WS
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = PINSEL_PIN_6; // P0.6 as I2SRX_SDA
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = PINSEL_PIN_7; // P0.7 as I2STX_CLK
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = PINSEL_PIN_8; // P0.8 as I2STX_WS
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = PINSEL_PIN_9; // P0.9 as I2STX_SDA
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Portnum = PINSEL_PORT_4;
	PinCfg.Funcnum = PINSEL_FUNC_1;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = PINSEL_PIN_29;
	PINSEL_ConfigPin(&PinCfg);

	I2S_CFG_Type I2S_TransConf = { I2S_WORDWIDTH_16, I2S_STEREO,
			I2S_STOP_ENABLE, I2S_RESET_ENABLE, I2S_MASTER_MODE,
			I2S_MUTE_DISABLE, };

	/* I2S init */
	I2S_Init(LPC_I2S);
	I2S_Config(LPC_I2S, I2S_TX_MODE, &I2S_TransConf);
	I2S_Stop(LPC_I2S, I2S_TX_MODE);
	I2S_FreqConfig(LPC_I2S, freq, I2S_TX_MODE);

	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
	I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
	I2S_ModeConfig(LPC_I2S, &I2S_ClkConfig, I2S_TX_MODE);
	//I2S_IRQConfig(LPC_I2S,I2S_TX_MODE,8);

	I2S_DMAConf_Type I2SDMACfg;
	I2SDMACfg.DMAIndex = I2S_DMA_1;
	I2SDMACfg.depth = 4;
	I2S_DMAConfig(LPC_I2S, &I2SDMACfg, I2S_TX_MODE);

	I2S_Start(LPC_I2S);

	I2S_DMACmd(LPC_I2S, I2S_DMA_1, I2S_TX_MODE, ENABLE);

}

void TransmitValue(unsigned int val) {
	I2S_Send(LPC_I2S, val);
}
