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
#include <errno.h>
#include <limits.h>

#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <libintl.h>

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

/*
 * parse the configuration file and update options
 * return -1 in case of error, otherwise, 0 is returned
 * @conf: path to the configuration file
 */
int
parse_conf(char *conf)
{
	FILE *fd;
	int tmp;
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
		if ((val = strtrim(val)) == NULL) {
			(void)fprintf(stderr, _("Error: no value for %s in configuration"
				" file\n"), key);
			return -1;
			/* NOTREACHED */
		}

		if (strcmp(key, "color_header") == 0) {
			if ((tmp = cvrt_color(val)) == -1)
				(void)fprintf(stderr, _("Unknown color value: "
						"%s\n"), val);
			else
				cnf.chead = tmp;
		} else if (strcmp(key, "color_low") == 0) {
			if ((tmp = cvrt_color(val)) == -1)
				(void)fprintf(stderr, _("Unknown color value: "
						"%s\n"), val);
			else
				cnf.clow = tmp;
		} else if (strcmp(key, "color_medium") == 0) {
			if ((tmp = cvrt_color(val)) == -1)
				(void)fprintf(stderr, _("Unknown color value: "
						"%s\n"), val);
			else
				cnf.cmedium = tmp;
		} else if (strcmp(key, "color_high") == 0) {
			if ((tmp = cvrt_color(val)) == -1)
				(void)fprintf(stderr, _("Unknown color value: "
						"%s\n"), val);
			else
				cnf.chigh = tmp;
		} else if (strcmp(key, "graph_medium") == 0) {
			tmp = (int)strtol(val, (char **) NULL, 10);
			if ((tmp == LONG_MIN || tmp == LONG_MAX) && errno == ERANGE)
				(void)fprintf(stderr, _("Value conversion failed"
					" for graph_medium: %s. What were you "
					"expecting with such a thing anyway?\n"),
					val);
			else if (tmp < 0)
				(void)fprintf(stderr, _("Medium value cannot be"
					" set below 0: %s\n"), val);
			else if (tmp > 100)
				(void)fprintf(stderr, _("Medium value cannot be"
					" set above 100: %s\n"), val);
			else
				cnf.gmedium = tmp;
		} else if (strcmp(key, "graph_high") == 0) {
			tmp = (int)strtol(val, (char **) NULL, 10);
			if (tmp < 0) {
				(void)fprintf(stderr, _("High value cannot be"
					" set below 0: %s\n"), val);
			} else if (tmp > 100) {
				(void)fprintf(stderr, _("High value cannot be"
					" set above 100: %s\n"), val);
			} else
				cnf.ghigh = tmp;
		} else if (strcmp(key, "graph_symbol") == 0) {
			if (strlen(val) == 1)
				cnf.gsymbol = val[0];
			else
				(void)fprintf(stderr, _("Wrong symbol value: "
						"%s\n"), val);
		} else
			(void)fprintf(stderr, _("Error: unknown option in configuration "
					"file: %s\n"), key);
	}

	if (fclose(fd) == EOF)
		perror("Could not close configuration file ");

	return 0;
	/* NOTREACHED */
}

/*
 * trim withespaces from the input string and returns it
 * @str: string that needs to be trimmed
 */
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

/*
 * convert color from natural name into correponding number and return it
 * @col: color name
 */
int
cvrt_color(char *col)
{
	if (strcmp(col, _("black")) == 0)
		return BLACK;
		/* NOTREACHED */
	else if (strcmp(col, _("red")) == 0)
		return RED;
		/* NOTREACHED */
	else if (strcmp(col, _("green")) == 0)
		return GREEN;
		/* NOTREACHED */
	else if (strcmp(col, _("yellow")) == 0)
		return YELLOW;
		/* NOTREACHED */
	else if (strcmp(col, _("blue")) == 0)
		return BLUE;
		/* NOTREACHED */
	else if (strcmp(col, _("magenta")) == 0)
		return MAGENTA;
		/* NOTREACHED */
	else if (strcmp(col, _("cyan")) == 0)
		return CYAN;
		/* NOTREACHED */
	else if (strcmp(col, _("white")) == 0)
		return WHITE;
		/* NOTREACHED */
	else
		return -1;
		/* NOTREACHED */
}

/*
 * init a conf structure
 * @cnf: structure to be initiated
 */
void
init_conf(struct conf *cnf)
{
	cnf->clow =	GREEN;
	cnf->cmedium =	YELLOW;
	cnf->chigh =	RED;
	cnf->chead =	BLUE;

	cnf->gmedium =	50;
	cnf->ghigh =	75;

	cnf->gsymbol =	'=';
}
