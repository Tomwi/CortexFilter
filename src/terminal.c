#include "terminal.h"

LPC_UART_TypeDef *UARTx = (LPC_UART_TypeDef *) LPC_UART1;

char *strrev(char *str) {
	char *p1, *p2;
	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}
char *itoa(int n, char *s, int b) {
	static char digits[] = "0123456789";
	int i = 0, sign;

	if ((sign = n) < 0)
		n = -n;
	do {
		s[i++] = digits[n % b];
	} while ((n /= b) > 0);
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	return strrev(s);
}

void uart1Init() {

	/* UART Configuration structure variable */
	UART_CFG_Type UARTConfigStruct;
	/* UART FIFO configuration Struct variable */
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	/* Pin configuration for UART0 */
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize UART1 pin connect
	 */
	PinCfg.Portnum = PINSEL_PORT_0;
	PinCfg.Pinnum = PINSEL_PIN_15;
	PinCfg.Funcnum = PINSEL_FUNC_1;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Portnum = PINSEL_PORT_0;
	PinCfg.Pinnum = PINSEL_PIN_16;
	PinCfg.Funcnum = PINSEL_FUNC_1;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&PinCfg);

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	/* Set Baudrate to 115200 */
	UARTConfigStruct.Baud_rate = 115200;

	/* Initialize UART0 peripheral with given to corresponding parameter */
	UART_Init(UARTx, &UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	/* Initialize FIFO for UART0 peripheral */
	UART_FIFOConfig(UARTx, &UARTFIFOConfigStruct);

	/*  Enable UART Transmit */
	UART_TxCmd(UARTx, ENABLE);

}

void term1PutText(char *s) {
	UART_Send(UARTx, (uint8_t *) s, strlen(s), BLOCKING);//NONE_BLOCKING);
}

void term1PutValue(int32_t value, uint8_t endOfLine) {

	char buffer[128];
	itoa(value, buffer, 10);

	term1PutText(buffer);

	if (endOfLine)
		term1PutText("\r\n");
	else
		term1PutText(",");

}
