#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "colortypes.h"

typedef int dev_handle_t;
typedef ssize_t dev_size_t;

dev_handle_t serialInit(const char *device);
void writeChannels(color_config_t *cfg, dev_handle_t devfd);
void serialClose(dev_handle_t handle);
