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
 * text.c
 *
 * Text display functions
 */
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "dotfile.h"
#include "export.h"
#include "extern.h"
#include "list.h"
#include "util.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

/* static function declaration */
static void text_disp_header(void);
static void text_disp_sum(double stot, double utot,
		double ftot, double ifitot, double ifatot);
static void text_disp_bar(double perct);
static void text_disp_uat(double n, double perct, int req_width);
static void text_disp_fs(const char *fsname);
static void text_disp_type(const char *type);
static void text_disp_inodes(uint64_t files, uint64_t favail);
static void text_disp_mount(const char *dir);
static void text_disp_mopt(const char *opts);
static void text_disp_perct(double perct);

static void change_color(double perct);
static void reset_color(void);

/* init pointers from display structure to the functions found here */
void
init_disp_text(struct display *disp)
{
    disp->init         = NULL; /* not required --> not implemented here */
    disp->deinit       = NULL; /* not required --> not implemented here */
    disp->print_header = text_disp_header;
    disp->print_sum    = text_disp_sum;
    disp->print_bar    = text_disp_bar;
    disp->print_uat    = text_disp_uat;
    disp->print_fs     = text_disp_fs;
    disp->print_type   = text_disp_type;
    disp->print_inodes = text_disp_inodes;
    disp->print_mount  = text_disp_mount;
    disp->print_mopt   = text_disp_mopt;
    disp->print_perct  = text_disp_perct;
}

/*
 * Display header
 */
static void
text_disp_header(void)
{
	int gap;

	/* use color option if triggered */
	if (cflag)
		(void)printf("\033[;%dm", cnf.chead);

	(void)printf("%-*s", max.fsname, _("FILESYSTEM"));

	if (Tflag)
		(void)printf("%-*s", max.fstype, _("TYPE"));

	if (!bflag) {
		(void)printf("%s", _("(=) USED"));
		gap = max.bar -
		         (int)strlen(_("(=) USED")) - (int)strlen(_("FREE (-)"));
		(void)printf("%*s", gap, "");
		(void)printf("%s", _("FREE (-)"));
	}

	(void)printf("%*s", max.perctused + 1, _("%USED"));

	if (dflag)
		(void)printf("%*s", max.used, _("USED"));

	(void)printf("%*s", max.avail, _("AVAILABLE"));
	(void)printf("%*s", max.total, _("TOTAL"));

	if (iflag) {
		(void)printf("%*s", max.nbinodes, _("#INODES"));
		(void)printf("%*s", max.avinodes, _("AV.INODES"));
	}

	/* precedded by a space because previous colum is right aligned */
	(void)printf(" %-*s", max.mntdir, _("MOUNTED ON"));

	if (oflag)
		(void)printf("%-*s", max.mntopts, _("MOUNT OPTIONS"));
	(void)printf("\n");

	reset_color();
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
text_disp_sum(double stot, double atot, double utot,
              double ifitot, double ifatot)
{
	double ptot = 0;
	int width;

	width = Tflag ? max.fsname + max.fstype : max.fsname;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;

	/* use color option if triggered */
	if (cflag)
		(void)printf("\033[;%dm", cnf.chead);
	(void)printf("%-*s", width, _("SUM:"));
	reset_color();

	if (!bflag)
		text_disp_bar(ptot);

	text_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		text_disp_uat(utot, ptot, max.used);
	text_disp_uat(atot, ptot, max.avail);
	text_disp_uat(stot, ptot, max.total);

	if (iflag)
		text_disp_inodes((uint64_t)ifitot, (uint64_t)ifatot);

	(void)printf("\n");
}

/*
 * Display the nice usage bar
 * @perct: percentage value
 */
static void
text_disp_bar(double perct)
{
	int i, j;
	int barinc = 5;

	/* option to display a wider bar */
	if (wflag) {
		barinc = 2;
	}

	/* used (*) */
	(void)printf("[");

	if (!cflag) {
		for (i = 0; i < perct; i += barinc)
			(void)printf("%c", cnf.gsymbol);

		for (j = i; j < 100; j += barinc)
			(void)printf("-");
	} else { /* color */

		/* green */
		(void)printf("\033[;%dm", cnf.clow);
		for (i = 0; (i < cnf.gmedium) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		/* yellow */
		(void)printf("\033[;%dm", cnf.cmedium);
		for (; (i < cnf.ghigh) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		/* red */
		(void)printf("\033[;%dm", cnf.chigh);
		for (; (i < 100) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		reset_color();

		for (j = i; j < 100; j += barinc)
			(void)printf("-");
	}

	(void)printf("]");
}

/*
 * Display used, available and total correctly formatted
 * @n: number to print
 * @perct: percentage (useful for finding which color to use)
 * @req_width: required width (used for terminal display, otherwise can be 0)
 */
static void
text_disp_uat(double n, double perct, int req_width)
{
	int i;

	i = 0;

	if (unitflag == 'h')
		i = humanize(&n);

	change_color(perct);
	(void)printf("%*.1f", req_width - 1, n); /* -1 for the unit symbol */
	reset_color();
	print_unit(i, 1);
}

/*
 * Display file system
 * @fsname: list of the file system to print
 */
static void
text_disp_fs(const char *fsname)
{
	(void)printf("%-*s", max.fsname, fsname);
}

/*
 * Display file system type
 * @type: the file system type to print
 */
static void
text_disp_type(const char *type)
{
	(void)printf("%-*s", max.fstype, type);
}

/*
 * Display inodes
 *@files: number of inodes
 *@favail: number of available inodes
 */
static void
text_disp_inodes(uint64_t files, uint64_t favail)
{
	int i;

	if (unitflag == 'h') {
		i = humanize_i(&files);
		(void)printf("%*" PRIu64, max.nbinodes - 1, files);
		print_unit(i, 0);
		i = humanize_i(&favail);
		(void)printf("%*" PRIu64, max.avinodes - 1, favail);
		print_unit(i, 0);
	} else {
		(void)printf(" %*" PRIu64, max.nbinodes - 1, files);
		(void)printf(" %*" PRIu64, max.avinodes - 1, favail);
	}
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
text_disp_mount(const char *dir)
{
	/* preceded by a space because previous colum is right aligned */
	(void)printf(" %-*s", max.mntdir, dir);
}

/*
 * Display mount options
 * @opts: mount options
 */
static void
text_disp_mopt(const char *opts)
{
	(void)printf("%-*s", max.mntopts, opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
text_disp_perct(double perct)
{
	change_color(perct);
	(void)printf("%*.1f", max.perctused, perct);
	reset_color();
	(void)printf("%%");
}

/*
 * Change color according to perct
 * @perct: percentage
 */
static void
change_color(double perct)
{
	if (cflag) {
		if (perct < (double)cnf.gmedium) /* green */
			(void)printf("\033[;%dm", cnf.clow);
		else if (perct < (double)cnf.ghigh) /* yellow */
			(void)printf("\033[;%dm", cnf.cmedium);
		else /* red */
			(void)printf("\033[;%dm", cnf.chigh);
	}
}

/*
 * Reset color attribute to default
 */
static void
reset_color(void)
{
	if (cflag)
		(void)printf("\033[;m");
}
