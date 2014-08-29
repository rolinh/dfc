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
 * csv.c
 *
 * CSV display functions
 * NB: color and graph do not make sense in CSV format so we just do not care
 * about those
 */
#include <stdio.h>

#include "extern.h"
#include "export.h"
#include "display.h"
#include "list.h"
#include "util.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

/* static function declaration */
static void csv_disp_header(void);
static void csv_disp_sum(double stot, double utot, double ftot,
		double ifitot, double ifatot);
static void csv_disp_bar(double perct);
static void csv_disp_uat(double n, double perct, int req_width);
static void csv_disp_fs(const char *fsname);
static void csv_disp_type(const char *type);
static void csv_disp_inodes(uint64_t files, uint64_t favail);
static void csv_disp_mount(const char *dir);
static void csv_disp_mopt(const char *opts);
static void csv_disp_perct(double perct);

/* init pointers from display structure to the functions found here */
void
init_disp_csv(struct display *disp)
{
    disp->init         = NULL; /* not required --> not implemented here */
    disp->deinit       = NULL; /* not required --> not implemented here */
    disp->print_header = csv_disp_header;
    disp->print_sum    = csv_disp_sum;
    disp->print_bar    = csv_disp_bar;
    disp->print_uat    = csv_disp_uat;
    disp->print_fs     = csv_disp_fs;
    disp->print_type   = csv_disp_type;
    disp->print_inodes = csv_disp_inodes;
    disp->print_mount  = csv_disp_mount;
    disp->print_mopt   = csv_disp_mopt;
    disp->print_perct  = csv_disp_perct;
}

/*
 * Display header
 */
static void
csv_disp_header(void)
{
	(void)printf(_("FILESYSTEM%c"), cnf.csvsep);

	if (Tflag)
		(void)printf(_("TYPE%c"), cnf.csvsep);

	(void)printf(_("%%USED%c"), cnf.csvsep);

	if (dflag)
		(void)printf(_("USED%c"), cnf.csvsep);

	(void)printf(_("AVAILABLE%c"), cnf.csvsep);

	(void)printf(_("TOTAL%c"), cnf.csvsep);

	if (iflag) {
		(void)printf(_("#INODES%c"), cnf.csvsep);
		(void)printf(_("AV.INODES%c"), cnf.csvsep);
	}

	(void)printf("%s", _("MOUNTED ON"));

	if (oflag)
		(void)printf(_("%cMOUNT OPTIONS"), cnf.csvsep);

	(void)printf("\n");
}

/*
 * Display the sum (useful when -s option is used
 * @stot: total size of "total"
 * @atot: total size of "available"
 * @utot: total size of "used"
 * @ifitot: total number of inodes
 * @ifatot: total number of available inodes
 */
static void
csv_disp_sum(double stot, double atot, double utot, double ifitot, double ifatot)
{
	double ptot = 0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;
	(void)printf(_("SUM:%c"), cnf.csvsep);

	if (Tflag)
		(void)printf("%c", cnf.csvsep);

	csv_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		csv_disp_uat(utot, ptot, 0);
	csv_disp_uat(atot, ptot, 0);
	csv_disp_uat(stot, ptot, 0);

	if (iflag)
		csv_disp_inodes((uint64_t)ifitot, (uint64_t)ifatot);

	(void)printf("\n");
}

/*
 * Should display the nice usage bar but this makes no sense in CSV export
 * Therefore, this is a dummy function that does nothing when called from
 * dfc.c but it is required in order to avoid stupid checks in dfc.c
 * @perct: is ignored
 */
static void
csv_disp_bar(double perct)
{
	(void)perct;
	/* DUMMY */
}

/*
 * Display available and total correctly formated
 * @n: number to print
 * @perct: ignored here
 * @req_width: ignored here
 */
static void
csv_disp_uat(double n, double perct, int req_width)
{
	int i;

	(void)perct;
	(void)req_width;

	(void)printf("%c", cnf.csvsep);

	if (unitflag == 'h') {
		i = humanize(&n);
		(void)printf(i == 0 ? "%.f" : "%.1f", n);
		print_unit(i, 1);
	} else {
		if (unitflag == 'b')
			(void)printf("%f", n);
		else if (unitflag == 'k')
			(void)printf("%f", n);
		else
			(void)printf("%.1f", n);
		print_unit(0, 1);
	}
}

/*
 * Display file system
 * @fsname: list of the file system to print
 */
static void
csv_disp_fs(const char *fsname)
{
	(void)printf("%s%c",fsname, cnf.csvsep);
}

/*
 * Display file system type
 * @type: the file system type to print
 */
static void
csv_disp_type(const char *type)
{
	(void)printf("%s%c", type, cnf.csvsep);
}

/*
 * Display inodes
 *@files: number of inodes
 *@favail: number of available inodes
 */
static void
csv_disp_inodes(uint64_t files, uint64_t favail)
{
	int i;

	if (unitflag == 'h') {
		i = humanize_i(&files);
		(void)printf("%c%" PRIu64, cnf.csvsep, files);
		print_unit(i, 0);
		i = humanize_i(&favail);
		(void)printf("%c%" PRIu64, cnf.csvsep, favail);
		print_unit(i, 0);
	} else {
		(void)printf("%c%" PRIu64, cnf.csvsep, files);
		(void)printf("%c%" PRIu64, cnf.csvsep, favail);
	}
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
csv_disp_mount(const char *dir)
{
	(void)printf("%c%s", cnf.csvsep, dir);
}

/*
 * Display mount options
 * @opts: mount options
 */
static void
csv_disp_mopt(const char *opts)
{
	(void)printf("%c%s", cnf.csvsep, opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
csv_disp_perct(double perct)
{
	(void)printf("%.f%%", perct);
}
