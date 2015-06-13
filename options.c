/* Based on the example at http://crasseux.com/books/ctutorial/argp-example.html */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include "options.h"
#include "colortypes.h"

const char *argp_program_version = "color 0.1";

const char *argp_program_bug_address = "this github issue tracker";

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
   */
static struct argp_option options[] =
{
	{"device", 'd', "DEVICE", 0,
		"The serial device to be used for output. Default: /dev/ttyACM0", 0},
	{"mode",   'm', "MODESTRING", 0,
		"Select operation mode: daemon, oneshot, onecolor, gradient, test", 0},
	{"preset",   'p', "PRESET", 0,
		"Choose color gradient PRESET: warm, cold, full", 0},
	{"speed",   's', "SPEED", 0,
		"For gradient mode: The speed of color changes (higher is slower)", 0},
	{"wakeup",   'w', "SECONDS", 0,
		"Activate wakup mode: Slowly increase brightness in specified time (SECONDS)", 0},
	{"color",   'c', "RGB", 0,
		"For oneshot and onecolor modes: A Hex-RGB-Value in the form RRGGBB", 0},
	{"httpport",   'o', "PORT", 0,
		"For daemon mode, the port to listen on", 0},
	{0, 0, 0, 0, 0, 0}
};


/*
   PARSER. Field 2 in ARGP.
   Order of parameters: KEY, ARG, STATE.
   */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	karateoptions_t *arguments = state->input;

	switch (key)
	{
		case 'd':
			arguments->device = arg;
			break;

		case 's':
			arguments->speed = atoi(arg);
			break;

		case 'p':
			if (!strcmp(arg, "warm"))
			{
				arguments->colorRange = 0.2;
				arguments->colorStart = 0;
				printf("Using warm colors\n");
			}
			else if (!strcmp(arg, "cold"))
			{
				arguments->colorRange = 0.5;
				arguments->colorStart = 0.22;
				printf("Using cold colors\n");
			}
			else if (!strcmp(arg, "full"))
			{
				arguments->colorRange = 1.0;
				arguments->colorStart = 0.0;
				printf("Using full colors\n");
			}
			else 
			{
				fprintf(stderr, "Unknown preset mode: %s. Try --help\n", arg);
				exit(EXIT_FAILURE);
			}
			break;

		case 'w':
			arguments->wakeupTime = atoi(arg) * 1000;
			printf("Using wakeup mode with %d milliseconds delay\n", arguments->wakeupTime);
			break;

		case 'm':
			if (!strcmp(arg, "oneshot"))
			{
				arguments->mode = ONESHOT;
			}
			else if (!strcmp(arg, "onecolor"))
			{
				arguments->mode = ONECOLOR;
			}
			else if (!strcmp(arg, "gradient"))
			{
				arguments->mode = GRADIENT;
			}
			else if (!strcmp(arg, "test"))
			{
				arguments->mode = TEST;
			}
			else if (!strcmp(arg, "daemon"))
			{
				arguments->mode = DAEMON;
			}
			else 
			{
				fprintf(stderr, "Unknown mode: %s. Try --help\n", arg);
				exit(EXIT_FAILURE);
			}

			break;

		case 'c':
			parseRGB(arg, &arguments->color);
			break;

		case 'o':
			arguments->httpPort = atoi(arg);
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/*
   DOC.  Field 4 in ARGP.
   Program documentation.
   */
static char doc[] = "color -- A program to easily use a karatelight for mood lighting";

/*
   The ARGP structure itself.
   */
static struct argp argp = {options, parse_opt, 0, doc, 0, 0, 0}; // First 0 is ARG_DOCS (normal arguments) which we dont have

/*
   The main function.
   Notice how now the only function call needed to process
   all command-line options and arguments nicely
   is argp_parse.
   */

void getOptions(int argc, char **argv, karateoptions_t *result)
{
	// defaults
	result->device = "/dev/ttyACM0";
	result->devfd = -1;
	result->mode = ONECOLOR;
	result->wakeupTime = 0;
	result->colorRange = 1.0;
	result->colorStart = 0.0;
	result->httpPort = 1338;

	argp_parse (&argp, argc, argv, 0, 0, result);


	/* fprintf (stdout, "mode = %d\nwakeup = %d\n\n", arguments.mode, arguments.wakeupTime); */
	/* fprintf (stdout, "colorStart = %f\ncolorRange = %f\n\n", arguments.colorStart, arguments.colorRange); */
}
