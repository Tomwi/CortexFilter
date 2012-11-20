#ifndef FIR_H
#define FIR_H

#include "lpc_types.h"
#define FILTER_LEN 128
void firFilter(int16_t* in, int16_t* out, int len);

#endif
