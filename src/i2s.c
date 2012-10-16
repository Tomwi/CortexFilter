#include "i2s.h"

void initI2SDMA(uint32_t txblock) {
	I2S_DMAConf_Type I2SDMACfg;
	GPDMA_Channel_CFG_Type GPDMACfg;
	/* Initialize GPDMA controller */
	GPDMA_Init();
	LPC_GPDMA->DMACConfig = 0x01;

	/* Setting GPDMA interrupt */
	// Disable interrupt for DMA
	NVIC_DisableIRQ(DMA_IRQn);
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));

	/* Setup GPDMA channel --------------------------------*/
	/* channel 0 */
	GPDMACfg.ChannelNum = 0;
	/* Source memory */
	GPDMACfg.SrcMemAddr = txblock;
	/* Destination memory */
	GPDMACfg.DstMemAddr = 0;
	/* Transfer size */
	GPDMACfg.TransferSize = TRANSFER_SIZE;
	/* Transfer width */
	GPDMACfg.TransferWidth = 0;
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

	I2SDMACfg.DMAIndex = I2S_DMA_1;
	I2SDMACfg.depth = 4;
	I2S_DMAConfig(LPC_I2S, &I2SDMACfg, I2S_TX_MODE);

	I2S_Start(LPC_I2S);
	I2S_DMACmd(LPC_I2S, I2S_DMA_1, I2S_TX_MODE, ENABLE);

}
void initTX(unsigned int freq, uint32_t txblock) {

	PINSEL_CFG_Type PinCfg;
	I2S_CFG_Type I2S_ConfigStruct;
	I2S_MODEConf_Type I2S_ClkConfig;

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
	PinCfg.Pinnum = PINSEL_PIN_29;
	PINSEL_ConfigPin(&PinCfg);

	/* I2S init */
	I2S_Init(LPC_I2S);

	I2S_ConfigStruct.wordwidth = I2S_WORDWIDTH_16;
	I2S_ConfigStruct.mono = I2S_STEREO;
	I2S_ConfigStruct.stop = I2S_STOP_ENABLE;
	I2S_ConfigStruct.reset = I2S_RESET_ENABLE;
	I2S_ConfigStruct.ws_sel = I2S_MASTER_MODE;
	I2S_ConfigStruct.mute = I2S_MUTE_DISABLE;
	I2S_Config(LPC_I2S, I2S_TX_MODE, &I2S_ConfigStruct);

	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
	I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
	I2S_ModeConfig(LPC_I2S, &I2S_ClkConfig, I2S_TX_MODE);

	I2S_FreqConfig(LPC_I2S, freq, I2S_TX_MODE);

	initI2SDMA(txblock);

}

void TransmitValue(unsigned int val) {
	I2S_Send(LPC_I2S, val);
}
