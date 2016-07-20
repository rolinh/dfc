/*
 * Copyright (c) 2016, Robin Hahling
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
 * json.c
 *
 * JSON export functions
 * NB: color and graph do not make sense in JSON format so we just do not care
 * about those
 */
#include <stdio.h>
#include <stdlib.h>

#include "extern.h"
#include "export.h"
#include "display.h"
#include "list.h"
#include "util.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

static int first_element;

/* static function declaration */
static void json_disp_init(void);
static void json_disp_deinit(void);
static void json_disp_header(void);
static void json_disp_sum(double stot, double utot, double ftot, double ifitot,
		double ifatot);
static void json_disp_bar(double perct);
static void json_disp_uat(double n, const char *key);
static void json_disp_used(double used, double perct, int req_width);
static void json_disp_avail(double avail, double perct, int req_width);
static void json_disp_total(double total, double perct, int req_width);
static void json_disp_fs(const char *fsname);
static void json_disp_type(const char *type);
static void json_disp_inodes(uint64_t files, uint64_t favail);
static void json_disp_mount(const char *dir);
static void json_disp_mopt(const char *opts);
static void json_disp_perct(double perct);
static void json_disp_ln_end(void);

/* init pointers from display structure to the functions found here */
void
init_disp_json(struct display *disp)
{
	disp->init         = json_disp_init;
	disp->deinit       = json_disp_deinit;
	disp->print_header = json_disp_header;
	disp->print_sum    = json_disp_sum;
	disp->print_bar    = json_disp_bar;
	disp->print_used   = json_disp_used;
	disp->print_avail  = json_disp_avail;
	disp->print_total  = json_disp_total;
	disp->print_fs     = json_disp_fs;
	disp->print_type   = json_disp_type;
	disp->print_inodes = json_disp_inodes;
	disp->print_mount  = json_disp_mount;
	disp->print_mopt   = json_disp_mopt;
	disp->print_perct  = json_disp_perct;
	disp->print_ln_end = json_disp_ln_end;
}

static void
json_disp_init(void)
{
	(void)printf("{\"filesystems\":[");
	first_element = 1;
}

static void
json_disp_deinit(void)
{
	if (sflag)
		(void)printf("}\n");  /* sum func closes the list of fs */
	else
		(void)printf("]}\n");
}

static void
json_disp_header(void)
{
	/* DUMMY */
}

static void
json_disp_sum(double stot, double atot, double utot, double ifitot, double ifatot)
{
	double ptot = 0.0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;

	(void)printf("],\"sum\":{\"usage\":\"%f%%\"", ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		json_disp_uat(utot, "used");

	json_disp_uat(atot, "available");
	json_disp_uat(stot, "total");

	if (iflag)
		json_disp_inodes((uint64_t)ifitot, (uint64_t)ifatot);

	(void)printf("}");
}

static void
json_disp_bar(double perct)
{
	(void)perct;
	/* DUMMY */
}

static void
json_disp_uat(double n, const char *key)
{
	int i;
	i = 0;

	if (unitflag == 'h')
		i = humanize(&n);

	(void)printf(",\"%s\":\"", key);
	(void)printf(i == 0 ? "%.f" : "%.1f", n);
	print_unit(i, 1);
	(void)printf("\"");
}

static void
json_disp_used(double used, double perct, int req_width)
{
	(void)perct;
	(void)req_width;

	json_disp_uat(used, "used");
}

static void
json_disp_avail(double avail, double perct, int req_width)
{
	(void)perct;
	(void)req_width;

	json_disp_uat(avail, "available");
}

static void
json_disp_total(double total, double perct, int req_width)
{
	(void)perct;
	(void)req_width;

	json_disp_uat(total, "total");
}

static void
json_disp_fs(const char *fsname)
{
	if (!first_element)
		(void)printf(",");
	(void)printf("{\"filesystem\":\"%s\",", fsname);

	if (first_element)
		first_element = 0;
}

static void
json_disp_type(const char *type)
{
	(void)printf("\"type\":\"%s\",", type);
}

static void
json_disp_inodes(uint64_t files, uint64_t favail)
{
	int i;

	if (unitflag == 'h') {
		i = humanize_i(&files);
		(void)printf(",\"inodes_count\":\"%" PRIu64, files);
		print_unit(i, 0);
		(void)printf("\"");
		i = humanize_i(&favail);
		(void)printf(",\"inodes_available\":\"%" PRIu64, favail);
		print_unit(i, 0);
		(void)printf("\"");
	} else {
		(void)printf(",\"inodes_count\":\"%" PRIu64 "\"", files);
		(void)printf(",\"inodes_available\":%" PRIu64 "\"", favail);
	}
}

static void
json_disp_mount(const char *dir)
{
	(void)printf(",\"mount_point\":\"%s\"", dir);
}

static void
json_disp_mopt(const char *opts)
{
	(void)printf(",\"mount_options\":\"%s\"", opts);
}

static void
json_disp_perct(double perct)
{
	(void)printf("\"usage\":\"%f%%\"", perct);
}

static void
json_disp_ln_end(void)
{
	(void)printf("}");
}
