#include <time.h>
#include <math.h>

#include "util.h"

long currentTimeMillis() {
	long ms;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	ms = (spec.tv_sec * 1000) + round(spec.tv_nsec / 1.0e6);

	return ms;
}
