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
 * csv.c
 *
 * CSV display functions
 * NB: color and graph do not make sense in CSV format so we just do not care
 * about those
 */
#include <stdio.h>

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
static void csv_disp_header(struct list *lst);
static void csv_disp_sum(struct list *lst, double stot, double utot, double ftot,
                  double ifitot, double ifatot);
static void csv_disp_bar(double perct);
static void csv_disp_at(double n, double perct);
static void csv_disp_fs(struct list *lst, char *fsname);
static void csv_disp_type(struct list *lst, char *type);
static void csv_disp_inodes(uint64_t files, uint64_t favail);
static void csv_disp_mount(char *dir);
static void csv_disp_mopt(struct list *lst, char *dir, char *opts);
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
    disp->print_at     = csv_disp_at;
    disp->print_fs     = csv_disp_fs;
    disp->print_type   = csv_disp_type;
    disp->print_inodes = csv_disp_inodes;
    disp->print_mount  = csv_disp_mount;
    disp->print_mopt   = csv_disp_mopt;
    disp->print_perct  = csv_disp_perct;
}

/*
 * Display header
 * @lst: queue containing the informations
 */
static void
csv_disp_header(struct list *lst)
{
	/* do not care about lst in CSV output */
	(void)lst;

	(void)printf(_("FILESYSTEM,"));

	if (Tflag)
		(void)printf(_("TYPE,"));

	(void)printf(_("%%USED,"));

	if (dflag)
		(void)printf(_("USED,"));

	(void)printf(_("AVAILABLE,"));

	(void)printf(_("TOTAL,"));

	if (iflag) {
		(void)printf(_("#INODES,"));
		(void)printf(_("AV.INODES,"));
	}

	(void)printf(_("MOUNTED ON"));

	if (oflag)
		(void)printf(_(",MOUNT OPTIONS"));

	(void)printf("\n");
}

/*
 * Display the sum (useful when -s option is used
 * @lst: queue containing the informations
 * @stot: total size of "total"
 * @atot: total size of "available"
 * @utot: total size of "used"
 * @ifitot: total number of inodes
 * @ifatot: total number of available inodes
 */
static void
csv_disp_sum(struct list *lst, double stot, double atot, double utot,
             double ifitot, double ifatot)
{
	double ptot = 0;

	/* do not care about lst in CSV output */
	(void)lst;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;
	(void)printf(_("SUM:,"));

	csv_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		csv_disp_at(utot, ptot);
	csv_disp_at(atot, ptot);
	csv_disp_at(stot, ptot);

	if (iflag) {
		(void)printf(",%.f,k", ifitot / 1000);
		(void)printf(",%.f,k", ifatot / 1000);
	}

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
 * @perct: percentage (useful for finding which color to use)
 */
static void
csv_disp_at(double n, double perct)
{
	int i;

	/* do not care about perct here */
	(void)perct;

	(void)printf(",");

	if (unitflag == 'h') {
		i = humanize(&n);
		(void)printf(i == 0 ? "%.f," : "%.1f,", n);
		print_unit(i, 1);
	} else {
		if (unitflag == 'b')
			(void)printf("%f,", n);
		else if (unitflag == 'k')
			(void)printf("%f,", n);
		else
			(void)printf("%.1f,", n);
		print_unit(0, 1);
	}
}

/*
 * Display file system
 * @lst: is ignored here
 * @fsname: list of the file system to print
 */
static void
csv_disp_fs(struct list *lst, char *fsname)
{
	/* we do not care about lst here */
	(void)lst;

	(void)printf("%s,",fsname);
}

/*
 * Display file system type
 * @lst: is ignored here
 * @type: the file system type to print
 */
static void
csv_disp_type(struct list *lst, char *type)
{
	/* we do not care about lst here */
	(void)lst;

	(void)printf("%s,", type);
}

/*
 * Display inodes
 *@files: number of inodes
 *@favail: number of available inodes
 */
static void
csv_disp_inodes(uint64_t files, uint64_t favail)
{
	(void)printf(",%" PRIu64 ",k", files);
	(void)printf(",%" PRIu64 ",k", favail);
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
csv_disp_mount(char *dir)
{
	(void)printf(",%s", dir);
}

/*
 * Display mount options
 * @lst: is ignored here
 * @dir: is ignored here
 * @opts: mount options
 */
static void
csv_disp_mopt(struct list *lst, char *dir, char *opts)
{
	/* we do not care about lst here */
	(void)lst;
	/* neither about dir */
	(void)dir;

	(void)printf(",%s", opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
csv_disp_perct(double perct)
{
	(void)printf("%.f,%%", perct);
}
