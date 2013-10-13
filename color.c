#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <unistd.h>
#include <termios.h>
#include <regex.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "util.h"
#include "hsl.h"
#include "serial.h"

#define DEFAULT_WAKEUP_SPEED (60*1000)

void showGradient(struct hsl *col1, struct hsl *col2, int devfd);
int showStaticWakeup(struct hsl *col, int devfd);
void sigint_handler(int signum);

static int run = 1;
static char wakeupMode = 0;
static double colorRange = 1.0;
static double colorStart = 0.0;
static double brightness = 0.5;
static int speed = 1000;
static int wakeupTime = DEFAULT_WAKEUP_SPEED; //ms
static long startMillis;

int main(int argc, char** argv)
{
    char buf[128];
    const char *devname = "/dev/ttyACM0";

	//FIXME: parse arguments sanely
    if (argc == 1)
    {
		printf("Using full range.\nHint: You can change the settings with %s\
				[brightness] [speed] [mode] [wakeuptime-to-full-brightness]\n",
        argv[0]);
    }

    if (argc >= 2)
    {
        brightness = atoi(argv[1]) / 200.0;
    }

    if (argc >= 3)
    {
        speed = atoi(argv[2]);
    }

    if (argc >= 4)
    {
        if (!strcmp(argv[3], "warm"))
        {
            colorRange = 0.2;
            colorStart = 0;
            printf("Using warm colors\n");
        }
        else if (!strcmp(argv[3], "cold"))
        {
            colorRange = 0.5;
            colorStart = 0.22;
            printf("Using cold colors\n");
        }
        else if (!strcmp(argv[3], "wakeup"))
        {
			wakeupMode = 1;
            colorRange = 0.2;
            colorStart = 0;
			startMillis = currentTimeMillis();
            printf("Wakeup-mode!\n");
        }
        else
        {
            printf("Color keyword not defined, \"warm\", \"cold\" and \"wakeup\" can be used\n");
        }
    }

	if (argc >=  5)
	{
		if (strcmp(argv[3], "wakeup") != 0) 
			printf("Warning: Ignoring wakeup-time parameter in non-wakeup mode");

		wakeupTime = atoi(argv[4]) * 1000;
		if (wakeupTime <= 0)
			wakeupTime = DEFAULT_WAKEUP_SPEED;
	}

    printf("Settings used:\nSpeed: %u\nBrightness: %.2f\n", speed, (brightness*2));

    struct sigaction newact;
    newact.sa_handler = sigint_handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, NULL);

	dev_handle_t devfd = serialInit(devname);
	if (devfd < 0) {
		exit(EXIT_FAILURE);
	}

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
    getRandColor(curr);

	if (wakeupMode)
	{
		while(run && !showStaticWakeup(&wakeupOrange, devfd));
		curr = &wakeupOrange;
	}
	while(run)
	{
		getRandColor(next);
		showGradient(curr, next, devfd);

		tmp = curr;
		curr = next;
		next = tmp;
	}

    writeColor(0, 0, 0, devfd);

    CLOSE_DEVICE(devfd);

    return EXIT_SUCCESS;
}


inline int showStaticWakeup(struct hsl *col, int devfd) 
{
	long elapsed = currentTimeMillis() - startMillis;
	if (elapsed > wakeupTime)
		elapsed = wakeupTime;
	double ratio = 1.0 - ((double)wakeupTime - elapsed) / wakeupTime;
	col->L = brightness * ratio;

	struct rgb rgb = {0};
	HSL2RGB(col->H, col->S, col->L, &rgb);
	writeColor(rgb.R, rgb.G, rgb.B, devfd);
	usleep(50 * 1000);

	return ratio >= 1.0;
}

void showGradient(struct hsl *col1, struct hsl *col2, int devfd)
{
    const int steps = (int) (fabs(col2->H - col1->H) * speed);

    struct rgb rgb = {0};
    double h = col1->H;
    double s = col1->S;
    double l = col1->L;
    HSL2RGB(h, s, l, &rgb);
    writeColor(rgb.R, rgb.G, rgb.B, devfd);

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
        writeColor(rgb.R, rgb.G, rgb.B, devfd);
        usleep(30 * 1000);
    }
}

void sigint_handler(int signum)
{
	// silence compiler
	assert(signum == SIGINT);
    run = 0;
}

