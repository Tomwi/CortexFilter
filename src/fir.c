#include "fir.h"


//Array with filter coefficients
#define FILTER_LEN 128
int16_t coeffs[ FILTER_LEN ] = {8,9,8,9,9,10,10,12,12,15,15,19,19,23,23,28,27,32,31,35,33,38,34,40,36,43,38,48,43,55,51,66,62,78,72,90,81,98,84,100,79,95,68,83,52,70,36,62,26,64,29,81,47,117,79,170,120,234,157,306,170,400,79,919,919,79,400,170,306,157,234,120,170,79,117,47,81,29,64,26,62,36,70,52,83,68,95,79,100,84,98,81,90,72,78,62,66,51,55,43,48,38,43,36,40,34,38,33,35,31,32,27,28,23,23,19,19,15,15,12,12,10,10,9,9,8,9,8};

//Array to store last input values
static int16_t circBuf[FILTER_LEN];
//Variable to remember position in the circular buffer.
static int pos;

void firFixed(uint32_t* in, int len) {
	int i;
	for (i = 0; i < len; i++) {

		//Take one sample and put it in the circular buffer
		circBuf[pos] = (int16_t)(in[i] & 0xffff);
		int j, mac = (1 << 14);
		int16_t* coeff = coeffs;

		/* Apply filter coefficients */
		for (j = 0; j < FILTER_LEN; j++) {
			mac += (int32_t)(circBuf[(pos + j) % FILTER_LEN])
					* ((int32_t)(*coeff++));
		}
		// saturate the result
		if (mac > 0x3fffffff) {
			mac = 0x3fffffff;
		} else if (mac < -0x40000000) {
			mac = -0x40000000;
		}
		/* convert from Q30 to Q15, this mixing with the original signal isn't that
		 * safe */
		in[i] = (int16_t)(mac >> 15);

		pos++;
		if (pos >= FILTER_LEN)
			pos = 0;

	}
}
