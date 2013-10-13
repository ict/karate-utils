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

typedef int dev_handle_t;
typedef ssize_t dev_size_t;
typedef speed_t dev_speed_t;

#define OPEN_DEVICE(name) open(name, O_WRONLY|O_NOCTTY)
#define CLOSE_DEVICE(fd) close(fd)
#define WRITE_DATA(fd, buf, len, written) ((written = write(fd, buf, (size_t)len)) != (ssize_t)-1)
#define FLUSH_BUFFER(fd) (tcdrain(fd) != -1)
#define INVALID_DEV_HANDLE -1
#define GET_SYS_ERR_MSG(buf) strerror_r(errno, buf, sizeof(buf))

#define DEFAULT_WAKEUP_SPEED (60*1000)

#include "hsl.c"
#include "util.c"

void writeColor(uint8_t r, uint8_t g, uint8_t b, int devfd);
void showGradient(struct hsl *col1, struct hsl *col2, int devfd);
int showStaticWakeup(struct hsl *col, int devfd);
void getRandColor(struct hsl *res);
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

    int devfd = OPEN_DEVICE(devname);
    if (devfd == INVALID_DEV_HANDLE) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "could not open serial port device '%s': %s", devname, buf);
        return -1;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = (CS8 | CSTOPB | CLOCAL);
    cfsetospeed(&tio, 38400);
    int ok = (tcsetattr(devfd, TCSANOW, &tio) == 0);
    if (ok)
        tcflush(devfd, TCIOFLUSH);
    if (!ok) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "configuration of serial port device '%s' failed: %s", devname, buf);
        CLOSE_DEVICE(devfd);
        return -1;
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

void writeColor(uint8_t r, uint8_t g, uint8_t b, int devfd) 
{
    // xAA|x12|CX|24|Gt1|Bt1|Rt1|Gt2|Bt2|Rt2|Gr1|Br1|Rr1|Gr2|Br2|Rr2|Gl1|Bl1|Rl1|Gl2|Bl2|Rl2|Gb|Bb|Rb|Gb|Bb|Rb

    char buf[128];

    uint8_t data[28];
    uint8_t *d = data;
    uint8_t *crc_pos = data+2;

    *d++ = 0xAA;
    *d++ = 0x12;
    d++; //data[2] is CRC
    *d++ = 24;
    for(int i = 0; i < 8; ++i)
    {
        *d++ = g;
        *d++ = b;
        *d++ = r;
    }

    uint8_t *v = data;
    uint8_t crc = 0;
    while (v < d)
    {
        if (v != crc_pos)
            crc ^= *v;
        ++v;
    }
    *crc_pos = crc;

    int len = d - data;
    if (len != 28)
        fprintf(stderr, "len = %d\n", len);
    int written = 0;
    if (!WRITE_DATA(devfd, data, len, written) || !FLUSH_BUFFER(devfd)) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "writing data to serial port failed: %s", buf);
    }
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

void getRandColor(struct hsl *col)
{
    col->H = (rand() / (double)RAND_MAX) * colorRange + colorStart;
    col->S = (rand() / (double)RAND_MAX) * 0.3 + 0.7;
	col->L = brightness;
}

void sigint_handler(int signum)
{
    run = 0;
}

