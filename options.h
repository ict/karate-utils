#pragma once

#include "colortypes.h"
#include "serial.h"

typedef enum {
	ONESHOT,
	ONECOLOR,
	GRADIENT,
	TEST,
	DAEMON
} color_mode_t;

typedef struct {
	color_mode_t mode;
	int wakeupTime;
	int speed;
	double colorRange;
	double colorStart;
	char *device;
	dev_handle_t devfd;
	rgb_t color;
	double brightness;
	int httpPort;
} karateoptions_t;

void getOptions(int argc, char **argv, karateoptions_t *result);
