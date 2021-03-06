
#define OPEN_DEVICE(name) open(name, O_WRONLY|O_NOCTTY)
#define CLOSE_DEVICE(fd) close(fd)
#define WRITE_DATA(fd, buf, len, written) ((written = write(fd, buf, (size_t)len)) != (ssize_t)-1)
#define FLUSH_BUFFER(fd) (tcdrain(fd) != -1)
#define GET_SYS_ERR_MSG(buf) strerror_r(errno, buf, sizeof(buf))

#define INVALID_DEV_HANDLE -1

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#include "serial.h"
#include "colortypes.h"


// for various error messages
static char buf[128];

void writeChannels(color_config_t *cfg, dev_handle_t devfd) 
{
    // xAA|x12|CX|24|Gt1|Bt1|Rt1|Gt2|Bt2|Rt2|Gr1|Br1|Rr1|Gr2|Br2|Rr2|Gl1|Bl1|Rl1|Gl2|Bl2|Rl2|Gb|Bb|Rb|Gb|Bb|Rb
	
	int numChannels = NUM_CHANNELS > 8 ? 8 : NUM_CHANNELS;
	if (NUM_CHANNELS > 8) {
		fprintf(stderr, "Warning: Only first 8 (of %d) channels will be output!\n", NUM_CHANNELS);
	}

    char buf[128];

    uint8_t data[28] = {0};
    uint8_t *d = data;
    uint8_t *crc_pos = data+2;

    *d++ = 0xAA;
    *d++ = 0x12;
    d++; //data[2] is CRC
    *d++ = 24;
    for(int i = 0; i < numChannels; ++i)
    {
        *d++ = cfg->channel[i].G;
        *d++ = cfg->channel[i].B;
        *d++ = cfg->channel[i].R;
    }

	// calculate CRC
    uint8_t *v = data;
    uint8_t crc = 0;
    while (v < d)
    {
        if (v != crc_pos)
            crc ^= *v;
        ++v;
    }
    *crc_pos = crc;

    int len = d - data;
    if (len != 28)
        fprintf(stderr, "len = %d\n", len);
    int written = 0;
    if (!WRITE_DATA(devfd, data, len, written) || !FLUSH_BUFFER(devfd)) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "writing data to serial port failed: %s", buf);
    }
}


dev_handle_t serialInit(const char *device) {
    int devfd = OPEN_DEVICE(device);
    if (devfd == INVALID_DEV_HANDLE) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "could not open serial port device '%s': %s", device, buf);
        return -1;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = (CS8 | CSTOPB | CLOCAL);
    cfsetospeed(&tio, 38400);
    int ok = (tcsetattr(devfd, TCSANOW, &tio) == 0);
    if (ok)
        tcflush(devfd, TCIOFLUSH);
    if (!ok) {
        GET_SYS_ERR_MSG(buf);
        fprintf(stderr, "configuration of serial port device '%s' failed: %s", device, buf);
        CLOSE_DEVICE(devfd);
        return -1;
    }

	return devfd;
}

void serialClose(dev_handle_t handle) 
{
	CLOSE_DEVICE(handle);
}
