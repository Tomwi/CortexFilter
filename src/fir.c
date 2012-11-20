#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define FILTER_LEN (100)
#define N_CHAN (2)
#define BUFFER_SIZE (512*sizeof(int16_t))
#define FILTERED (100)
#define ORIGINAL (0)

int16_t coeffs[FILTER_LEN] = { 11, 10, 8, 6, 4, 1, -2, -6, -12, -19, -27, -36,
                               -46, -57, -68, -80, -92, -102, -112, -119, -124, -125, -121, -113, -99,
                               -79, -52, -17, 24, 73, 130, 195, 267, 344, 428, 516, 608, 701, 796, 890,
                               982, 1070, 1153, 1229, 1297, 1356, 1404, 1441, 1467, 1479, 1479, 1467,
                               1441, 1404, 1356, 1297, 1229, 1153, 1070, 982, 890, 796, 701, 608, 516,
                               428, 344, 267, 195, 130, 73, 24, -17, -52, -79, -99, -113, -121, -125,
                               -124, -119, -112, -102, -92, -80, -68, -57, -46, -36, -27, -19, -12, -6,
                               -2, 1, 4, 6, 8, 10, 11
                             };


int16_t circBuf[FILTER_LEN];
int pos;

void firFilter(int16_t* coeff, int32_t* in, int16_t* out, int len)
{
	int i;
	for(i=0; i<len; i++) {
		circBuf[pos] = in[i]>>16;
		int j, mac = (1<<14);
		int16_t* coeff = coeffs;

		/* Apply filter coefficients */
		for(j=0; j<FILTER_LEN; j++) {
			mac+= (int32_t)(circBuf[(pos+j)%FILTER_LEN])*((int32_t)(*coeff++));
		}
		
		// saturate the result
		if (mac > 0x3fffffff) {
			mac = 0x3fffffff;
		} else if (mac < -0x40000000) {
			mac = -0x40000000;
		}
		/* convert from Q30 to Q15, this mixing with the original signal isn't that
		 * safe */
		out[i] = (in[i] * ORIGINAL) / 100
		         + ((int16_t)(mac >> 15) * FILTERED) / 100;

		pos++;
		pos%=FILTER_LEN;
	}
}


#ifdef TEST_FILTER
int fileSize(FILE* fp)
{
	fseek(fp, 0, SEEK_END);
	int sz = ftell(fp);
	rewind(fp);
	return sz;
}
int main(int argc, char** argv)
{

	if(argc < 3) {
		printf("usage: <file_name_in> <file_name_out>\n");
		return 0;
	}
	FILE* fp = fopen(argv[1], "rb");

	if(fp==NULL) {
		printf("File %s could not be opened!\n", argv[1]);
		return 0;

	}
	int test = fileSize(fp);
	int16_t* buf = malloc(test);
	int16_t* out = malloc(test);
	if(buf==NULL || out==NULL)
		goto end;

	fread(buf, 1, test, fp);
	fclose(fp);

	firFilter(coeffs, buf, out, test/2);

	fp = fopen(argv[2], "wb");
	if(fp==NULL)
		goto end;
	fwrite(out, 1, test, fp);
	
	/* Free resources */
end:
	fclose(fp);
	free(buf);
	free(out);
	return 0;
}
#endif
