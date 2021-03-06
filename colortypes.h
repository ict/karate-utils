#pragma once

#include <stdint.h>
#include <inttypes.h>


#define NUM_CHANNELS (8)

typedef struct rgb
{
    uint8_t R;
    uint8_t G;
    uint8_t B;

} rgb_t;

typedef struct hsl
{
    double H;
    double S;
    double L;
} hsl_t;

typedef struct colorcfg
{
	rgb_t channel[NUM_CHANNELS];
} color_config_t;

void HSL2RGB(double h, double sl, double l, rgb_t *rgb);
void RGB2HSL(rgb_t *rgb, hsl_t *hsl);
void getRandColor(struct hsl *col, double colorRange, double colorStart, double brightness);
void parseRGB(const char *str, rgb_t *res);
