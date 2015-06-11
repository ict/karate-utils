#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "hsl.h"

typedef int dev_handle_t;
typedef ssize_t dev_size_t;

void writeColor(uint8_t r, uint8_t g, uint8_t b, dev_handle_t devfd);
void writeChannel(uint8_t r, uint8_t g, uint8_t b, uint8_t channel, dev_handle_t devfd);
void writeHSL(hsl_t *c, dev_handle_t devfd);
void writeRGB(rgb_t *c, dev_handle_t devfd);
dev_handle_t serialInit(const char *device);
void serialClose(dev_handle_t handle);
