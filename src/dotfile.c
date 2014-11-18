/*
 * Copyright (c) 2012-2014, Robin Hahling
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the author nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

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
			(void)fputs("strcat failed while guessing "
					"configuration file\n", stderr);
			return NULL;
		}
		if (stat(conf, &buf) == 0)
			return conf;
		else /* no configuration file exists or it cannot be accessed */
			return NULL;
	} else { /* maybe XDG_CONFIG_HOME is just not exported */
		/* lets assume that XDG_CONFIG_HOME is simply $HOME/.config */
		if ((home = getenv("HOME")) != NULL) {
			if ((conf = strcat(home, "/.config/dfc/dfcrc")) == NULL) {
				(void)fputs("strcat failed while guessing "
						"configuration file location\n",
						stderr);
				return NULL;
			}
			if (stat(conf, &buf) == 0)
				return conf;
			else { /* support $HOME/.dfcrc */
				/* home has been modified by strcat */
				if ((home = getenv("HOME")) == NULL)
					return NULL;
				if ((conf = strcat(home, "/.dfcrc")) == NULL) {
					(void)fputs("strcat failed while guessing"
							" configuration file\n",
							stderr);
					return NULL;
				}
				if (stat(conf, &buf) == 0)
					return conf;
				else
					return NULL;
			}
		} else /* sorry, there is nothing I can do... */
			return NULL;
	}
}

/*
 * parse the configuration file and update options
 * return -1 in case of error, otherwise, 0 is returned
 * @conf: path to the configuration file
 */
int
parse_conf(const char *conf)
{
	FILE *fd;
	char line[255];
	char *key, *val;
	int ret = 0;

	if ((fd = fopen(conf, "r")) == NULL) {
		(void)fprintf(stderr, "Cannot read file %s", conf);
		perror(" ");
		return -1;
	}

	while ((fgets(line, (int)sizeof(line), fd)) != NULL) {

		/* skip empty lines and lines beginning with # */
		if (!strlen(strtrim(line)) || line[0] == '#')
			continue;

		key = strtok(line, "=");
		val = strtok(NULL, "");

		key = strtrim(key);
		if ((val = strtrim(val)) == NULL) {
			(void)fprintf(stderr, _("Error: no value for %s in "
				"configuration file\n"), key);
			if (fclose(fd) == EOF)
				perror("Could not close configuration file ");
			return -1;
		}

		ret += set_conf(key, val);
	}

	if (fclose(fd) == EOF)
		perror("Could not close configuration file ");

	if (ret < 0)
		return -1;

	return ret;
}

/*
 * Set configuration values taken from the configuration file
 * Return 0 if no error occurred, -1 otherwise.
 * @key: key in configuration file
 * @val: value corresponding to the key
 */
int
set_conf(const char *key, const char *val)
{
	int tmp;
	int ret = 0;
	char *tmpc = NULL;

	if (strcmp(key, "color_header") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
		else
			cnf.chead = tmp;
	} else if (strcmp(key, "color_low") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
		else
			cnf.clow = tmp;
	} else if (strcmp(key, "color_medium") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
		else
			cnf.cmedium = tmp;
	} else if (strcmp(key, "color_high") == 0) {
		if ((tmp = colortoint(val)) == -1)
			goto unknown_color_value;
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
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hcheadbg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_header_fg") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hcheadfg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_cell_bg") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hccellbg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_cell_fg") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hccellfg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_hover_bg") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hchoverbg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_hover_fg") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hchoverfg = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_low") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hclow = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "html_color_medium") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hcmedium = tmpc;
		} else
			goto non_valid_html_color;

	} else if (strcmp(key, "html_color_high") == 0) {
		if (chk_html_colorcode(val) == 0) {
			if (!(tmpc = strdup(val)))
				goto strdup_failed;
			else
				cnf.hchigh = tmpc;
		} else
			goto non_valid_html_color;
	} else if (strcmp(key, "csv_separator") == 0) {
		if (strlen(val) == 1)
			cnf.csvsep = val[0];
		else {
			(void)fprintf(stderr, _("Wrong csv separator: "
					"%s\n"), val);
			ret = -1;
		}
	} else {
		(void)fprintf(stderr, _("Error: unknown option in configuration"
				" file: %s\n"), key);
		ret = -1;
	}

	free(tmpc);
	return ret;

unknown_color_value:
	(void)fprintf(stderr, _("Unknown color value: %s\n"), val);
	free(tmpc);
	return -1;

non_valid_html_color:
	(void)fprintf(stderr, _("Not a valid HTML color: %s\n"), val);
	free(tmpc);
	return -1;

strdup_failed:
	(void)fprintf(stderr, _("Could not assign value from configuration "
			"file: %s\n"), val);
	free(tmpc);
	return -1;
}

/*
 * init a conf structure
 * @cnf: structure to be initiated
 */
void
init_conf(struct conf *config)
{
	config->chead	= BLUE;
	config->clow	= GREEN;
	config->cmedium	= YELLOW;
	config->chigh	= RED;

	config->gmedium	= 50;
	config->ghigh	= 75;

	config->gsymbol	= '=';

	config->hcheadbg	= "970000";
	config->hcheadfg	= "FFFFFF";
	config->hccellbg	= "E9E9E9";
	config->hccellfg	= "000000";
	config->hchoverbg	= "FFFFFF";
	config->hchoverfg	= "000000";
	config->hclow		= "348017";
	config->hcmedium	= "FDD017";
	config->hchigh		= "F62217";

	config->csvsep = ',';
}
