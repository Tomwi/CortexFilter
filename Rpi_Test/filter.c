/***
  This file is part of PulseAudio.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 1024

#define FILTER_LEN 513
#define N_CHAN (2)
#define BUFFER_SIZE (512*sizeof(int16_t))
#define FILTERED (70)
#define ORIGINAL (1)

/* inBuf, workBuf, outBuf
int16_t buffers[BUFFER_SIZE*3];
*/

int16_t circBuf[FILTER_LEN];
int pos;


//int16_t coeffs[ FILTER_LEN ] = {8,9,8,9,9,10,10,12,12,15,15,19,19,23,23,28,27,32,31,35,33,38,34,40,36,43,38,48,43,55,51,66,62,78,72,90,81,98,84,100,79,95,68,83,52,70,36,62,26,64,29,81,47,117,79,170,120,234,157,306,170,400,79,919,919,79,400,170,306,157,234,120,170,79,117,47,81,29,64,26,62,36,70,52,83,68,95,79,100,84,98,81,90,72,78,62,66,51,55,43,48,38,43,36,40,34,38,33,35,31,32,27,28,23,23,19,19,15,15,12,12,10,10,9,9,8,9,8};

int16_t coeffs[ FILTER_LEN ] =  {
       -4,     -4,     -4,     -4,     -4,     -4,     -5,     -5,     -5,
       -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,
       -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,
       -4,     -4,     -4,     -4,     -3,     -3,     -3,     -2,     -2,
       -2,     -1,     -1,      0,      0,      1,      1,      2,      2,
        3,      4,      4,      5,      5,      6,      7,      8,      8,
        9,     10,     10,     11,     12,     13,     13,     14,     15,
       15,     16,     17,     17,     18,     19,     19,     20,     20,
       21,     21,     22,     22,     23,     23,     23,     23,     24,
       24,     24,     24,     24,     24,     24,     24,     23,     23,
       23,     22,     22,     21,     21,     20,     19,     18,     18,
       17,     16,     14,     13,     12,     11,      9,      8,      7,
        5,      3,      2,      0,     -2,     -4,     -6,     -7,     -9,
      -11,    -13,    -16,    -18,    -20,    -22,    -24,    -26,    -29,
      -31,    -33,    -35,    -37,    -40,    -42,    -44,    -46,    -48,
      -50,    -52,    -54,    -56,    -58,    -60,    -61,    -63,    -64,
      -66,    -67,    -68,    -70,    -71,    -72,    -72,    -73,    -74,
      -74,    -74,    -74,    -74,    -74,    -73,    -73,    -72,    -71,
      -70,    -69,    -67,    -66,    -64,    -62,    -60,    -57,    -55,
      -52,    -49,    -46,    -42,    -38,    -35,    -31,    -26,    -22,
      -17,    -12,     -7,     -2,      3,      9,     15,     21,     27,
       33,     40,     47,     53,     60,     68,     75,     82,     90,
       98,    105,    113,    121,    129,    137,    146,    154,    162,
      171,    179,    188,    196,    205,    213,    222,    230,    239,
      247,    255,    264,    272,    280,    288,    296,    304,    312,
      320,    327,    334,    342,    349,    356,    362,    369,    375,
      381,    387,    393,    398,    403,    408,    413,    418,    422,
      426,    429,    433,    436,    439,    441,    444,    446,    447,
      449,    450,    450,    451,    451,    451,    450,    450,    449,
      447,    446,    444,    441,    439,    436,    433,    429,    426,
      422,    418,    413,    408,    403,    398,    393,    387,    381,
      375,    369,    362,    356,    349,    342,    334,    327,    320,
      312,    304,    296,    288,    280,    272,    264,    255,    247,
      239,    230,    222,    213,    205,    196,    188,    179,    171,
      162,    154,    146,    137,    129,    121,    113,    105,     98,
       90,     82,     75,     68,     60,     53,     47,     40,     33,
       27,     21,     15,      9,      3,     -2,     -7,    -12,    -17,
      -22,    -26,    -31,    -35,    -38,    -42,    -46,    -49,    -52,
      -55,    -57,    -60,    -62,    -64,    -66,    -67,    -69,    -70,
      -71,    -72,    -73,    -73,    -74,    -74,    -74,    -74,    -74,
      -74,    -73,    -72,    -72,    -71,    -70,    -68,    -67,    -66,
      -64,    -63,    -61,    -60,    -58,    -56,    -54,    -52,    -50,
      -48,    -46,    -44,    -42,    -40,    -37,    -35,    -33,    -31,
      -29,    -26,    -24,    -22,    -20,    -18,    -16,    -13,    -11,
       -9,     -7,     -6,     -4,     -2,      0,      2,      3,      5,
        7,      8,      9,     11,     12,     13,     14,     16,     17,
       18,     18,     19,     20,     21,     21,     22,     22,     23,
       23,     23,     24,     24,     24,     24,     24,     24,     24,
       24,     23,     23,     23,     23,     22,     22,     21,     21,
       20,     20,     19,     19,     18,     17,     17,     16,     15,
       15,     14,     13,     13,     12,     11,     10,     10,      9,
        8,      8,      7,      6,      5,      5,      4,      4,      3,
        2,      2,      1,      1,      0,      0,     -1,     -1,     -2,
       -2,     -2,     -3,     -3,     -3,     -4,     -4,     -4,     -4,
       -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,
       -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,     -5,
       -5,     -5,     -5,     -4,     -4,     -4,     -4,     -4,     -4
};

//Array to 

void firFilter(int16_t* coeff, int16_t* in, int16_t* out, int len)
{
	int i;
	for(i=0; i<len; i++) {
		circBuf[pos] = in[i];
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
		if(pos>FILTER_LEN)
			printf("error"); //line of the year!

	}
}

int main(int argc, char*argv[]) {

    /* The Sample format to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 1
    };

    pa_simple *s = NULL;
    int ret = 1;
    int error;

    /* replace STDIN with the specified file if needed */
    if (argc > 1) {
        int fd;

        if ((fd = open(argv[1], O_RDONLY)) < 0) {
            fprintf(stderr, __FILE__": open() failed: %s\n", strerror(errno));
            goto finish;
        }

        if (dup2(fd, STDIN_FILENO) < 0) {
            fprintf(stderr, __FILE__": dup2() failed: %s\n", strerror(errno));
            goto finish;
        }

        close(fd);
    }

    /* Create a new playback stream */
    if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    for (;;) {
        uint16_t buf[BUFSIZE];
        uint16_t out[BUFSIZE];
        ssize_t r;


        pa_usec_t latency;

        if ((latency = pa_simple_get_latency(s, &error)) == (pa_usec_t) -1) {
            fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
            goto finish;
        }

        fprintf(stderr, "%0.0f usec    \r", (float)latency);


        /* Read some data ... */
        if ((r = read(STDIN_FILENO, buf, sizeof(buf))) <= 0) {
            if (r == 0) /* EOF */
                break;

            fprintf(stderr, __FILE__": read() failed: %s\n", strerror(errno));
            goto finish;
        }
		firFilter(coeffs, buf, out, r/2);
        /* ... and play it */
        if (pa_simple_write(s, out, (size_t) r, &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            goto finish;
        }
    }

    /* Make sure that every single sample was played */
    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    ret = 0;

finish:

    if (s)
        pa_simple_free(s);

    return ret;
}
