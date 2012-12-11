#include "fir.h"

#define FILTER_LEN 128

static int16_t coeffs[FILTER_LEN] =
		{ -2, 0, 0, 2, 4, 7, 9, 12, 15, 19, 22, 25, 28, 31, 33, 34, 34, 33, 30,
				25, 19, 11, 0, -11, -26, -42, -60, -79, -99, -119, -138, -156,
				-172, -185, -195, -200, -199, -192, -178, -156, -126, -88, -40,
				15, 79, 152, 233, 321, 414, 513, 615, 719, 824, 928, 1029, 1127,
				1218, 1301, 1376, 1441, 1494, 1534, 1562, 1576, 1576, 1562,
				1534, 1494, 1441, 1376, 1301, 1218, 1127, 1029, 928, 824, 719,
				615, 513, 414, 321, 233, 152, 79, 15, -40, -88, -126, -156,
				-178, -192, -199, -200, -195, -185, -172, -156, -138, -119, -99,
				-79, -60, -42, -26, -11, 0, 11, 19, 25, 30, 33, 34, 34, 33, 31,
				28, 25, 22, 19, 15, 12, 9, 7, 4, 2, 0, 0, -2 };

static int16_t circBuf[FILTER_LEN];
static int pos;

void firFixed(uint32_t* in, int len) {
	int i;
	for (i = 0; i < len; i++) {
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
