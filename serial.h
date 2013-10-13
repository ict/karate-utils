#pragma once

typedef int dev_handle_t;
typedef ssize_t dev_size_t;
typedef speed_t dev_speed_t;

void writeColor(uint8_t r, uint8_t g, uint8_t b, dev_handle_t devfd);
dev_handle_t serialInit(const char *device);
