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
 * tex.c
 *
 * TeX display functions
 */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <inttypes.h>

#include "extern.h"
#include "export.h"
#include "display.h"
#include "list.h"
#include "util.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

/* static function declaration */
static void tex_disp_init(void);
static void tex_disp_deinit(void);
static void tex_disp_header(struct list *lst);
static void tex_disp_sum(struct list *lst, double stot, double utot, double ftot,
                  double ifitot, double ifatot);
static void tex_disp_bar(double perct);
static void tex_disp_at(double n, double perct);
static void tex_disp_fs(struct list *lst, char *fsname);
static void tex_disp_type(struct list *lst, char *type);
static void tex_disp_inodes(uint64_t files, uint64_t favail);
static void tex_disp_mount(char *dir);
static void tex_disp_mopt(struct list *lst, char *dir, char *opts);
static void tex_disp_perct(double perct);

/* init pointers from display structure to the functions found here */
void
init_disp_tex(struct display *disp)
{
	disp->init         = tex_disp_init;
	disp->deinit       = tex_disp_deinit;
	disp->print_header = tex_disp_header;
	disp->print_sum    = tex_disp_sum;
	disp->print_bar    = tex_disp_bar;
	disp->print_at     = tex_disp_at;
	disp->print_fs     = tex_disp_fs;
	disp->print_type   = tex_disp_type;
	disp->print_inodes = tex_disp_inodes;
	disp->print_mount  = tex_disp_mount;
	disp->print_mopt   = tex_disp_mopt;
	disp->print_perct  = tex_disp_perct;
}

/*
 * Print all that is required for the TeX file like documentclass and so on
 * Also, computes and print the required amount of columns for the table.
 */
static void
tex_disp_init(void)
{
	int i;
	int ncolumns = 5;

	(void)puts("\\documentclass[a4]{report}");
	(void)puts("\\usepackage[landscape]{geometry}");
	if (cflag)
		(void)puts("\\usepackage{color}");
	(void)puts("\\begin{document}");

	if (Tflag)
		ncolumns++;
	if (!bflag)
		ncolumns++;
	if (dflag)
		ncolumns++;
	if (iflag)
		ncolumns += 2;
	if (oflag)
		ncolumns++;
	(void)printf("\\begin{tabular}{");
	for (i = 0; i < ncolumns; i++)
		(void)printf("|l");
	printf("|}\n");

}

/*
 * Close TeX file (end document and so on)
 */
static void
tex_disp_deinit(void)
{
	(void)puts("\\\\");
	(void)puts("\\hline");
	(void)puts("\\end{tabular}");
	(void)puts("\\end{document}");
}

/*
 * Display header
 * @lst: is ignored here
 */
static void
tex_disp_header(struct list *lst)
{
	(void)lst;

	(void)puts("\\hline");
	(void)printf("%s", _("FILESYSTEM"));
	if (Tflag)
		(void)printf(" & %s", _("TYPE"));
	if (!bflag)
		(void)printf(" & %s", _("USAGE"));
	(void)printf(" & %s", _("\\%USED"));
	if (dflag)
		(void)printf(" & %s", _("USED"));
	(void)printf(" & %s ", _("AVAILABLE"));
	(void)printf(" & %s ", _("TOTAL"));
	if (iflag) {
		(void)printf(" & %s ", _("\\#INODES"));
		(void)printf(" & %s ", _("AV.INODES,"));
	}
	(void)printf(" & %s ", _("MOUNTED ON"));
	if (oflag)
		(void)printf(" & %s ", _("MOUNT OPTIONS"));

	(void)puts("\\\\");
	(void)puts("\\hline");
}

/*
 * Display the sum (useful when -s option is used
 * @lst: is ignored here
 * @stot: total size of "total"
 * @atot: total size of "available"
 * @utot: total size of "used"
 * @ifitot: total number of inodes
 * @ifatot: total number of available inodes
 */
static void
tex_disp_sum(struct list *lst, double stot, double atot, double utot,
             double ifitot, double ifatot)
{
	double ptot = 0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;

	(void)lst;

	(void)printf("\\\\ SUM");

	if (Tflag)
		(void)printf(" & N/A");

	if (!bflag)
		tex_disp_bar(ptot);

	tex_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		tex_disp_at(utot, ptot);

	tex_disp_at(atot, ptot);
	tex_disp_at(stot, ptot);

	if (iflag)
		tex_disp_inodes((uint64_t)ifitot, (uint64_t)ifatot);

	/* keep same amount of columns in table */
	(void)printf(" & NA");
	if (oflag)
		(void)printf(" & NA ");
}

/*
 * Display the nice usage bar
 * @perct: percentage value
 */
static void
tex_disp_bar(double perct)
{
	/*
	 * It could be nice to have a non-ASCII graph bar but it requires TeX
	 * packages usually using postscript and so on. So stick with ASCII for
	 * now until someone shows up with a better idea.
	 */
	int i, j;
	int barinc = 5;

	(void)printf(" & ");

	/* option to display a wider bar */
	if (wflag) {
		barinc = 2;
	}

	if (!cflag) {
		for (i = 0; i < perct; i += barinc)
			(void)printf("%c", cnf.gsymbol);

		for (j = i; j < 100; j += barinc)
			(void)printf("\\-");
	} else { /* color */
		/* green */
		(void)printf("\\textcolor{%s}{", colortostr(cnf.clow));
		for (i = 0; (i < cnf.gmedium) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		/* yellow */
		(void)printf("}\\textcolor{%s}{", colortostr(cnf.cmedium));
		for (; (i < cnf.ghigh) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		/* red */
		(void)printf("}\\textcolor{%s}{", colortostr(cnf.chigh));
		for (; (i < 100) && (i < perct); i += barinc)
			(void)printf("%c", cnf.gsymbol);

		(void)printf("}");

		for (j = i; j < 100; j += barinc)
			(void)printf("\\-");
	}
}

/*
 * Display available and total correctly formated
 * @n: number to print
 * @perct: percentage (useful for finding which color to use)
 */
static void
tex_disp_at(double n, double perct)
{
	int i;

	(void)perct;

	if (unitflag == 'h') {
		i = humanize(&n);
		(void)printf(i == 0 ? " & %.f" : " & %.1f", n);
		print_unit(i, 1);
	} else {
		if (unitflag == 'b' || unitflag == 'k')
			(void)printf(" & %.f", n);
		else
			(void)printf(" & %.1f", n);
		print_unit(0, 1);
	}
}

/*
 * Display file system
 * @lst: is ignored here
 * @fsname: list of the file system to print
 */
static void
tex_disp_fs(struct list *lst, char *fsname)
{
	static int must_close = 0;
	char *cleaned_fsname = sanitizestr(fsname);

	if (cleaned_fsname == NULL) {
		(void)fputs("Cannot print file system name\n", stderr);
		return;
		/* NOTREACHED */
	}

	(void)lst;

	if (must_close == 1)
		(void)puts("\\\\");

	(void)printf("%s", cleaned_fsname);
	free(cleaned_fsname);

	must_close = 1;
}

/*
 * Display file system type
 * @lst: is ignored here
 * @type: the file system type to print
 */
static void
tex_disp_type(struct list *lst, char *type)
{
	char *cleaned_type = sanitizestr(type);

	if (cleaned_type == NULL) {
		(void)fputs("Cannot print type\n", stderr);
		return;
		/* NOTREACHED */
	}

	(void) lst;

	(void) printf(" & %s", cleaned_type);
	free(cleaned_type);
}

/*
 * Display inodes
 *@files: number of inodes
 *@favail: number of available inodes
 */
static void
tex_disp_inodes(uint64_t files, uint64_t favail)
{
	int i;

	if (unitflag == 'h') {
		i = humanize_i(&files);
		(void)printf(" & %" PRIu64, files);
		print_unit(i, 0);
		i = humanize_i(&favail);
		(void)printf(" & %" PRIu64, favail);
		print_unit(i, 0);
	} else
		(void)printf(" & %" PRIu64 " & %" PRIu64, files, favail);
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
tex_disp_mount(char *dir)
{
	char *cleaned_dir = sanitizestr(dir);

	if (cleaned_dir == NULL) {
		(void)fputs("Cannot print mount point\n", stderr);
		return;
		/* NOTREACHED */
	}

	(void)printf(" & %s", cleaned_dir);
	free(cleaned_dir);
}

/*
 * Display mount options
 * @lst: is ignored here
 * @dir: is ignored here
 * @opts: mount options
 */
static void
tex_disp_mopt(struct list *lst, char *dir, char *opts)
{
	char *cleaned_opts = sanitizestr(opts);

	if (cleaned_opts == NULL) {
		(void)fputs("Cannot print mount options\n", stderr);
		return;
		/* NOTREACHED */
	}

	(void)lst;
	(void)dir;

	(void)printf(" & %s", cleaned_opts);
	free(cleaned_opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
tex_disp_perct(double perct)
{
	(void)printf(" & %.f\\%%", perct);
}
