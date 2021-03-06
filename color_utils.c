#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "colortypes.h"

void HSL2RGB(double h, double sl, double l, rgb_t *rgb)
{
    double v;
    double r,g,b;

    r = l;   // default to gray
    g = l;
    b = l;

    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);

    if (v > 0)
    {
        double m;
        double sv;
        int sextant;
        double fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m ) / v;
        h *= 6.0;
        sextant = (int)h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;

        switch (sextant)
        {
            case 0:
                r = v;
                g = mid1;
                b = m;
                break;

            case 1:
                r = mid2;
                g = v;
                b = m;
                break;

            case 2:
                r = m;
                g = v;
                b = mid1;
                break;

            case 3:
                r = m;
                g = mid2;
                b = v;
                break;

            case 4:
                r = mid1;
                g = m;
                b = v;
                break;

            case 5:
                r = v;
                g = m;
                b = mid2;
                break;
        }
    }

    rgb->R = (uint8_t)(int)(r * 255.0f);
    rgb->G = (uint8_t)(int)(g * 255.0f);
    rgb->B = (uint8_t)(int)(b * 255.0f);
}

void getRandColor(struct hsl *col, double colorRange, double colorStart, double brightness)
{
    col->H = (rand() / (double)RAND_MAX) * colorRange + colorStart;
    col->S = (rand() / (double)RAND_MAX) * 0.3 + 0.7;
	col->L = brightness;
}


static inline uint8_t parseHex(char *hex) { 
	char *end;
	int r  = strtol(hex, &end, 16);
	if (*end != '\0' || r < 0 || r > 255) {
		fprintf(stderr, "Invalid value: %s: Using 0 instead\n", hex);
		return 0;
	}
	return (uint8_t) r;
}

void parseRGB(const char *str, rgb_t *res) {
	int i = 0;

	char col[3];
	col[2] = '\0';

	while (i < 6) {
		col[i%2] = str[i]; i++;
		col[i%2] = str[i]; i++;

		uint8_t num = parseHex(col);
		switch (i) {
			case 2:
				res->R = num; break;
			case 4:
				res->G = num; break;
			case 6:
				res->B = num; break;
		}
	}
}

static inline double max(double a, double b) {
	return a > b ? a : b;
}
static inline double min(double a, double b) {
	return a < b ? a : b;
}

void RGB2HSL (rgb_t *rgb, hsl_t *hsl) {

	double r = rgb->R/255.0;
	double g = rgb->G/255.0;
	double b = rgb->B/255.0;
	double v;
	double m;
	double vm;
	double r2, g2, b2;

	hsl->H = 0; // default to black
	hsl->S = 0;
	hsl->L = 0;
	v = max(r,g);
	v = max(v,b);
	m = min(r,g);
	m = min(m,b);

	hsl->L = (m + v) / 2.0;
	if (hsl->L <= 0.0) {
		return;
	}

	vm = v - m;
	hsl->S = vm;

	if (hsl->S > 0.0) {
		hsl->S /= (hsl->L <= 0.5) ? (v + m ) : (2.0 - v - m) ;
	} else {
		return;
	}

	r2 = (v - r) / vm;
	g2 = (v - g) / vm;
	b2 = (v - b) / vm;

	if (r == v) {
		hsl->H = (g == m ? 5.0 + b2 : 1.0 - g2);
	}

	else if (g == v) {
		hsl->H = (b == m ? 1.0 + r2 : 3.0 - b2);
	}

	else {
		hsl->H = (r == m ? 3.0 + g2 : 5.0 - r2);
	}

	hsl->H /= 6.0;
}
