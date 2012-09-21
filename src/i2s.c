#include "i2s.h"


I2S_CFG_Type I2S_TransConf = {
	I2S_WORDWIDTH_16,
	I2S_STEREO,
	I2S_STOP_ENABLE,
	I2S_RESET_ENABLE,
	I2S_MASTER_MODE,
	I2S_MUTE_DISABLE,
};

I2S_MODEConf_Type I2S_ClkConfig;

/* Inits I2S and stops the transmitter and receiver */
void initI2S(void){
	I2S_Init(LPC_I2S);
	I2S_Stop(LPC_I2S, I2S_TX_MODE);
	I2S_Stop(LPC_I2S, I2S_RX_MODE);
	I2S_Start(LPC_I2S);
}

void initTX(unsigned int freq){
	/* Configure pins for I2S Transmitter */
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 7; // P0.7 as I2STX_CLK
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8; // P0.8 as I2STX_WS
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9; // P0.9 as I2STX_SDA
	PINSEL_ConfigPin(&PinCfg);

	I2S_Config(LPC_I2S, I2S_TX_MODE, &I2S_TransConf);
	I2S_FreqConfig(LPC_I2S, freq, I2S_TX_MODE);
	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
	I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
	I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
}

void TransmitValue(unsigned int val){
	I2S_Send(LPC_I2S, val);
}
