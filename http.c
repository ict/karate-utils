
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#include "http.h"
#include "colortypes.h"
#include "color.h"
#include "serial.h"

static struct MHD_Daemon *server;

typedef enum {
	ERROR,
	ONE,
	ALL
} request_type_t;

typedef struct {
	request_type_t type;
	rgb_t color;
	color_config_t channels;
} request_t;

static int
handleConnection(void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
	const char *OKpage = "<html><body>OK</body></html>";
	const char *ERRpage = "<html><body>Error</body></html>";
	struct MHD_Response *response;
	int ret;

	printf ("New %s request for %s using version %s\n", method, url, version);

	request_t req;
	req.type = ERROR;

	if (strcmp(url, "/off") == 0) {
		req.color = (rgb_t) {0,0,0};
		req.type = ONE;
	}
	else if (strcmp(url, "/orange") == 0) {
		req.color = (rgb_t) {255,214,0};
		req.type = ONE;
	}

	else if (strcmp(url, "/orange") == 0) {
		req.color = (rgb_t) {255,214,0};
		req.type = ONE;
	}

	else if (strstr(url, "/rgb") == url && strlen(url) >= 10) {
		char buf[7];
		buf[6] = '\0';
		strncpy(buf, url+4, 6);
		parseRGB(buf, &req.color);
		req.type = ONE;
	}

	if (req.type == ERROR) {
		response = MHD_create_response_from_buffer (strlen (ERRpage), (void *) ERRpage, MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);

	
	} else {
		response = MHD_create_response_from_buffer (strlen (OKpage), (void *) OKpage, MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response (connection, MHD_HTTP_OK, response);

		karateoptions_t *options = (karateoptions_t *) cls;
		if (options->devfd < 0) {
			options->devfd = serialInit(options->device);
			if (options->devfd < 0) {
				MHD_destroy_response (response);
				return ret;
			}
		}

		switch (req.type) {
			
			case ONECOLOR:
				writeSingleRGB(&req.color, options->devfd);
				break;

			case ALL:
				;
			case ERROR:
				;
		
		}

		serialClose(options->devfd);
		options->devfd = -1;
	}

	MHD_destroy_response (response);
	return ret;
}

void startHTTP(karateoptions_t *options) 
{
	server = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, options->httpPort, NULL, NULL,
			&handleConnection, options, MHD_OPTION_END);
	if (NULL == server) {
		fprintf(stderr, "Error starting server");
		run = 0;
	}

	fprintf(stderr, "Started HTTP server on port %d\n", options->httpPort);
}


void stopHTTP(karateoptions_t *options) 
{
	MHD_stop_daemon(server);
}



