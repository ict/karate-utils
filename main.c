#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <assert.h>

#include "util.h"
#include "color.h"
#include "serial.h"
#include "options.h"
#include "http.h"

#define DEFAULT_WAKEUP_SPEED (60*1000)

void showGradient(struct hsl *col1, struct hsl *col2, int devfd);
void showStaticWakeup(struct hsl *col, int devfd);
void sigint_handler(int signum);

static long startMillis;
static karateoptions_t options;

volatile int run = 1;


int main(int argc, char** argv)
{
	// read options
	getOptions(argc, argv, &options);
	options.brightness = 0.5; //default for HSL, goes from 0 (black) to 1 (white)

	// set up Ctrl-C handler
    struct sigaction newact;
    newact.sa_handler = sigint_handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, NULL);

	// init the output
	if (options.mode != DAEMON) {
		options.devfd = serialInit(options.device);
		if (options.devfd < 0) {
			exit(EXIT_FAILURE);
		}
	}

	startMillis = currentTimeMillis();

	// what are we supposed to do?
	switch (options.mode) {
		case ONESHOT:
			{
				writeSingleRGB(&options.color, options.devfd);
				exit(EXIT_SUCCESS);
			}
		case ONECOLOR:
			{
				hsl_t col;
				RGB2HSL(&options.color, &col);
				if (options.wakeupTime)  {
					fprintf(stderr, "Starting wakeup. Color: %f %f %f\n", col.H, col.S, col.L);
					showStaticWakeup(&col, options.devfd);
				} else {
					writeSingleRGB(&options.color, options.devfd);
				}

				while (run) {
					usleep(100 * 1000);
				}

				break;
			}

		case GRADIENT:
			{
				//TODO
				break;
			}
		case TEST:
			{
				fprintf(stderr, "Testing all channels with color (%d, %d, %d)\n", options.color.R, options.color.G, options.color.B);
				for (uint8_t i = 0; i < 8; ++i) {
					fprintf(stderr, "Channel %" PRIu8 "\n", i);
					writeSingleChannel(options.color.R, options.color.G, options.color.B, i, options.devfd);
					usleep(5 * 1000 * 1000);
				}
			}

		case DAEMON:
			{

				startHTTP(&options);

				while (run) {
					usleep(100 * 1000);
				}

				stopHTTP(&options);
				break;
			}
	}
	
	/*
    struct hsl c1;
    struct hsl c2;
    struct hsl *curr = &c1;
    struct hsl *next = &c2;
    struct hsl *tmp;
	struct hsl wakeupOrange;


	wakeupOrange.H = 32 / 255.0;
	wakeupOrange.S = 0.8;
	wakeupOrange.L = 0;

    srand(42);
    getRandColor(curr, options.colorRange, options.colorStart, options.brightness);

	if (options.wakeupTime)
	{
		options.wakeupTime *= 1000;
		while(run && !showStaticWakeup(&wakeupOrange, devfd));
		curr = &wakeupOrange;
	}
	while(run)
	{
		getRandColor(next, options.colorRange, options.colorStart, options.brightness);
		showGradient(curr, next, devfd);

		tmp = curr;
		curr = next;
		next = tmp;
	}
	*/

	// be sure to switch off the LEDs (if not ONESHOT-mode)
	rgb_t off = {0,0,0};
    writeSingleRGB(&off, options.devfd);
    return EXIT_SUCCESS;
}


void showStaticWakeup(hsl_t *col, int devfd) 
{
	double ratio;
	options.brightness = col->S;
	do {
		long elapsed = currentTimeMillis() - startMillis;
		if (elapsed > options.wakeupTime)
			elapsed = options.wakeupTime;
		ratio = 1.0 - ((double)options.wakeupTime - elapsed) / options.wakeupTime;
		col->L = options.brightness * ratio;

		writeSingleHSL(col, devfd);
		/* fprintf(stderr, "Wrote %f %f %f\n", col->H, col->S, col->L); */
		usleep(50 * 1000);
	} while (run && ratio < 1.0);

}

void showGradient(struct hsl *col1, struct hsl *col2, int devfd)
{
    const int steps = (int) (fabs(col2->H - col1->H) * options.speed);

    struct rgb rgb = {0, 0, 0};
    double h = col1->H;
    double s = col1->S;
    double l = col1->L;
    HSL2RGB(h, s, l, &rgb);
	writeSingleRGB(&rgb, devfd);

    double hStep = (col2->H - col1->H) / steps;
    double sStep = (col2->S - col1->S) / steps;
    double lStep = (col2->L - col1->L) / steps;

    for(int i = 0; run && i < steps; ++i)
    {
        h += hStep;
        s += sStep;
        l += lStep;
        //printf("%1.2f %1.2f %1.2f -> %u %u %u\n", h, s, l, rgb.R, rgb.G, rgb.B);
        HSL2RGB(h, s, l, &rgb);
		writeSingleRGB(&rgb, devfd);
        usleep(30 * 1000);
    }
}

void sigint_handler(int signum)
{
	// silence compiler
	assert(signum == SIGINT);
    run = 0;
}

