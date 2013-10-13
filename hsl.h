#pragma once

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

void HSL2RGB(double h, double sl, double l, rgb_t *rgb);
void getRandColor(struct hsl *col, double colorRange, double colorStart, double brightness);
