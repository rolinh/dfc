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
#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "dotfile.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

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
		if ((conf = strcat(xdg_c_h, "/dfc/dfcrc")) == NULL) {
			(void)fprintf(stderr, "strcat failed while guessing "
					"configuration file\n");
			return NULL;
			/* NOTREACHED */
		}
		if (stat(conf, &buf) == 0)
			return conf;
			/* NOTREACHED */
		else /* no configuration file exists or it cannot be accessed */
			return NULL;
			/* NOTREACHED */
	} else { /* maybe XDG_CONFIG_HOME is just not exported */
		/* lets assume that XDG_CONFIG_HOME is simply $HOME/.config */
		if ((home = getenv("HOME")) != NULL) {
			if ((conf = strcat(strdup(home), "/.config/dfc/dfcrc"))
					== NULL) {
				(void)fprintf(stderr, "strcat failed while "
						"guessing "
						"configuration file\n");
				return NULL;
				/* NOTREACHED */
			}
			if (stat(conf, &buf) == 0)
				return conf;
				/* NOTREACHED */
			else { /* support $HOME/.dfcrc */
				if ((conf = strcat(strdup(home), "/.dfcrc"))
						== NULL) {
					(void)fprintf(stderr, "strcat failed"
							"while guessing "
							"configuration file\n");
					return NULL;
					/* NOTREACHED */
				}
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
	char line[255];
	char *key, *val;
	int ret = 0;

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
			(void)fprintf(stderr, _("Error: no value for %s in "
				"configuration file\n"), key);
			if (fclose(fd) == EOF)
				perror("Could not close configuration file ");
			return -1;
			/* NOTREACHED */
		}

		ret += set_conf(key, val);
	}

	if (fclose(fd) == EOF)
		perror("Could not close configuration file ");

	if (ret < 0)
		return -1;
		/* NOTREACHED */

	return ret;
	/* NOTREACHED */
}

/*
 * Set configuration values taken from the configuration file
 * Return 0 if no error occured, -1 otherwise.
 * @key: key in configuration file
 * @val: value corresponding to the key
 */
int
set_conf(char *key, char *val)
{
	int tmp;
	int ret = 0;

	if (strcmp(key, "color_header") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
			/* NOTREACHED */
		else
			cnf.chead = tmp;
	} else if (strcmp(key, "color_low") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
			/* NOTREACHED */
		else
			cnf.clow = tmp;
	} else if (strcmp(key, "color_medium") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
			/* NOTREACHED */
		else
			cnf.cmedium = tmp;
	} else if (strcmp(key, "color_high") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
			/* NOTREACHED */
		else
			cnf.chigh = tmp;
	} else if (strcmp(key, "graph_medium") == 0) {
		ret = -1;
		/* reset errno value for strtol (see strtol(3)) */
		errno = 0;
		tmp = (int)strtol(val, (char **) NULL, 10);
		if (errno)
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
		else {
			ret = 0;
			cnf.gmedium = tmp;
		}
	} else if (strcmp(key, "graph_high") == 0) {
		ret = -1;
		/* reset errno value for strtol (see strtol(3)) */
		errno = 0;
		tmp = (int)strtol(val, (char **) NULL, 10);
		if (errno)
			(void)fprintf(stderr, _("Value conversion failed"
				" for graph_medium: %s. What were you "
				"expecting with such a thing anyway?\n"),
				val);
		else if (tmp < 0)
			(void)fprintf(stderr, _("High value cannot be"
				" set below 0: %s\n"), val);
		else if (tmp > 100)
			(void)fprintf(stderr, _("High value cannot be"
				" set above 100: %s\n"), val);
		else {
			ret = 0;
			cnf.ghigh = tmp;
		}
	} else if (strcmp(key, "graph_symbol") == 0) {
		if (strlen(val) == 1)
			cnf.gsymbol = val[0];
		else {
			(void)fprintf(stderr, _("Wrong symbol value: "
					"%s\n"), val);
			ret = -1;
		}
	} else if (strcmp(key, "html_color_header_bg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hcheadbg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_header_fg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hcheadfg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_cell_bg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hccellbg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_cell_fg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hccellfg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_hover_bg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hchoverbg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_hover_fg") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hchoverfg = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_low") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hclow = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else if (strcmp(key, "html_color_medium") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hcmedium = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */

	} else if (strcmp(key, "html_color_high") == 0) {
		if (chk_html_colorcode(val) == 0)
			cnf.hchigh = strdup(val);
		else
			goto non_valid_html_color;
			/* NOTREACHED */
	} else {
		(void)fprintf(stderr, _("Error: unknown option in configuration"
				" file: %s\n"), key);
		ret = -1;
	}

	return ret;
	/* NOTREACHED */

unknown_color_value:
	(void)fprintf(stderr, _("Unknown color value: %s\n"), val);
	return -1;
	/* NOTREACHED */

non_valid_html_color:
	(void)fprintf(stderr, _("Not a valid HTML color: %s\n"), val);
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
	cnf->chead	= BLUE;
	cnf->clow	= GREEN;
	cnf->cmedium	= YELLOW;
	cnf->chigh	= RED;

	cnf->gmedium	= 50;
	cnf->ghigh	= 75;

	cnf->gsymbol	= '=';

	cnf->hcheadbg	= "970000";
	cnf->hcheadfg	= "FFFFFF";
	cnf->hccellbg	= "E9E9E9";
	cnf->hccellfg	= "000000";
	cnf->hchoverbg	= "FFFFFF";
	cnf->hchoverfg	= "000000";
	cnf->hclow	= "348017";
	cnf->hcmedium	= "FDD017";
	cnf->hchigh	= "F62217";
}
