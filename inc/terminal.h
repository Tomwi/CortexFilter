#ifndef TERMINAL_H
#define TERMINAL_H

#include "lpc_types.h"
#include "system_LPC17xx.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pinsel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strrev(char*);
char *itoa(int,char*,int);
void uart1Init();
void term1PutText(char*);
void term1PutValue(int32_t,uint8_t);

#endif
