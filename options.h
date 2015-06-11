#pragma once

#include "hsl.h"

typedef enum {
	ONESHOT,
	ONECOLOR,
	GRADIENT,
	TEST
} color_mode_t;

typedef struct {
	color_mode_t mode;
	int wakeupTime;
	int speed;
	double colorRange;
	double colorStart;
	char *device;
	rgb_t color;

	double brightness;
} karateoptions_t;

void getOptions(int argc, char **argv, karateoptions_t *result);
