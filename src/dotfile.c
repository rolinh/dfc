/*
 * Copyright (c) 2012, Robin Hahling
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1 Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  4 Neither the name of the author nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * dotfile.c
 *
 * Handle configuration file
 */
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "dotfile.h"
#include "extern.h"

/*
 * Finds the configuration file and returns it.
 * NULL is returned when no configuration file is found.
 * Configuration file follows XDG Base Directory Specification
 * http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 */
char *
getconf(void)
{
	struct stat buf;
	char *home;
	char *xdg_c_h;
	char *conf;

	if ((xdg_c_h = getenv("XDG_CONFIG_HOME")) != NULL) {
		conf = strcat(xdg_c_h, "/dfc/dfcrc");
		if (stat(conf, &buf) == 0)
			return conf;
			/* NOTREACHED */
		else /* no configuration file exists or it cannot be accessed */
			return NULL;
			/* NOTREACHED */
	} else { /* maybe XDG_CONFIG_HOME is just not exported */
		/* lets assume that XDG_CONFIG_HOME is simply $HOME/.config */
		if ((home = getenv("HOME")) != NULL) {
			conf = strcat(home, "/.config/dfc/dfcrc");
			if (stat(conf, &buf) == 0)
				return conf;
				/* NOTREACHED */
			else { /* support $HOME/.dfcrc */
				conf = strcat(home, "/.dfcrc");
				if (stat(conf, &buf) == 0)
					return conf;
					/* NOTREACHED */
				else
					return NULL;
					/* NOTREACHED */
			}
		} else /* sorry, there is nothing I can do... */
			return NULL;
			/* NOTREACHED */
	}
}

int
parse_conf(char *conf)
{
	FILE *fd;
	int tmpcol;
	char line[255];
	char *key, *val;

	if ((fd = fopen(conf, "r")) == NULL) {
		(void)fprintf(stderr, "Cannot read file %s", conf);
		perror(" ");
		return -1;
		/* NOTREACHED */
	}

	while ((fgets(line, sizeof(line), fd)) != NULL) {

		/* skip empty lines and lines beginning with # */
		if (!strlen(strtrim(line)) || line[0] == '#')
			continue;
			/* NOTREACHED */

		key = strtok(line, "=");
		val = strtok(NULL, "");

		key = strtrim(key);
		val = strtrim(val);

		if (strcmp(key, "color_header") == 0) {
			if ((tmpcol = cvrt_color(val)) == -1)
				(void)fprintf(stderr, "Unknown color value: "
						"%s\n", val);
			else
				cnf.chead = tmpcol;
		} else if (strcmp(key, "color_low") == 0) {
			if ((tmpcol = cvrt_color(val)) == -1)
				(void)fprintf(stderr, "Unknown color value: "
						"%s\n", val);
			else
				cnf.clow = tmpcol;
		} else if (strcmp(key, "color_medium") == 0) {
			if ((tmpcol = cvrt_color(val)) == -1)
				(void)fprintf(stderr, "Unknown color value: "
						"%s\n", val);
			else
				cnf.cmedium = tmpcol;
		} else if (strcmp(key, "color_high") == 0) {
			if ((tmpcol = cvrt_color(val)) == -1)
				(void)fprintf(stderr, "Unknown color value: "
						"%s\n", val);
			else
				cnf.chigh = tmpcol;
		} else if (strcmp(key, "graph_symbol") == 0) {
			if (strlen(val) == 1)
				cnf.gsymbol = val[0];
			else
				(void)fprintf(stderr, "Wrong symbol value: "
						"%s\n", val);
		} else
			(void)fprintf(stderr, "Unknown option in configuration "
					"file: %s\n", key);
	}

	if (fclose(fd) == EOF)
		perror("Could not close configuration file ");

	return 0;
	/* NOTREACHED */
}

char *
strtrim(char *str)
{
	char *end;

	if (!str)
		return NULL;
		/* NOTREACHED */

	while (isspace(*str))
		str++;

	if (*str == '\0')
		return str;
		/* NOTREACHED */

	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;

	*(end + 1) = '\0';

	return str;
	/* NOTREACHED */
}

int
cvrt_color(char *col)
{
	if (strcmp(col, "black") == 0)
		return BLACK;
		/* NOTREACHED */
	else if (strcmp(col, "red") == 0)
		return RED;
		/* NOTREACHED */
	else if (strcmp(col, "green") == 0)
		return GREEN;
		/* NOTREACHED */
	else if (strcmp(col, "yellow") == 0)
		return YELLOW;
		/* NOTREACHED */
	else if (strcmp(col, "blue") == 0)
		return BLUE;
		/* NOTREACHED */
	else if (strcmp(col, "magenta") == 0)
		return MAGENTA;
		/* NOTREACHED */
	else if (strcmp(col, "cyan") == 0)
		return CYAN;
		/* NOTREACHED */
	else if (strcmp(col, "white") == 0)
		return WHITE;
		/* NOTREACHED */
	else
		return -1;
		/* NOTREACHED */
}

void
init_conf(struct conf *cnf)
{
	cnf->clow =	GREEN;
	cnf->cmedium =	YELLOW;
	cnf->chigh =	RED;
	cnf->chead =	BLUE;

	cnf->gsymbol =	'=';
}
