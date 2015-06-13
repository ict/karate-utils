#pragma once

#include "options.h"

extern volatile int run;

void startHTTP(karateoptions_t *options);
void stopHTTP(karateoptions_t *options);
