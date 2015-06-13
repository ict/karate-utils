#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "colortypes.h"
#include "serial.h"

void writeSingleChannel(uint8_t r, uint8_t g, uint8_t b, uint8_t channel, dev_handle_t devfd);
void writeSingleHSL(hsl_t *c, dev_handle_t devfd);
void writeSingleRGB(rgb_t *c, dev_handle_t devfd);
