/*
 * convenience functions
 */

#include <string.h>
#include <stdio.h>

#include "color.h"
#include "serial.h"


static void writeColor(uint8_t r, uint8_t g, uint8_t b, dev_handle_t devfd)
{
	color_config_t cc;
	for (int i = 0; i < NUM_CHANNELS; ++i)
	{
		cc.channel[i] = (rgb_t) {r,g,b};
	}

	writeChannels(&cc, devfd);
}

void writeSingleChannel(uint8_t r, uint8_t g, uint8_t b, uint8_t channel, dev_handle_t devfd)
{

	if (channel > 7) {
		fprintf(stderr, "Channel must be between 0 and 7. Got %d\n", channel);
		return;
	}

	color_config_t cc;
	memset(&cc, 0, sizeof(cc));
	cc.channel[channel] = (rgb_t){r, g, b};

	writeChannels(&cc, devfd);
}

void writeSingleHSL(hsl_t *c, dev_handle_t devfd)
{
	rgb_t rc;
	HSL2RGB(c->H, c->S, c->L, &rc);
	writeColor(rc.R, rc.G, rc.B, devfd);
}

void writeSingleRGB(rgb_t *c, dev_handle_t devfd) 
{
	writeColor(c->R, c->G, c->B, devfd);
}

