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
 * text.c
 *
 * Text display functions
 */
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <inttypes.h>

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
static void text_disp_header(struct list *lst);
static void text_disp_sum(struct list *lst, double stot, double utot, double ftot,
                   double ifitot, double ifatot);
static void text_disp_bar(double perct);
static void text_disp_at(double n, double perct);
static void text_disp_fs(struct list *lst, char *fsname);
static void text_disp_type(struct list *lst, char *type);
static void text_disp_inodes(uint64_t files, uint64_t favail);
static void text_disp_mount(char *dir);
static void text_disp_mopt(struct list *lst, char *dir, char *opts);
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
    disp->print_at     = text_disp_at;
    disp->print_fs     = text_disp_fs;
    disp->print_type   = text_disp_type;
    disp->print_inodes = text_disp_inodes;
    disp->print_mount  = text_disp_mount;
    disp->print_mopt   = text_disp_mopt;
    disp->print_perct  = text_disp_perct;
}

/*
 * Display header
 * @lst: queue containing the informations
 */
static void
text_disp_header(struct list *lst)
{
	int i;
	int barinc = 5;

	/* use color option is triggered */
	if (cflag)
		(void)printf("\033[;%dm", cnf.chead);

	(void)printf(_("FILESYSTEM "));
	for (i = 11; i < lst->fsmaxlen; i++)
		(void)printf(" ");

	if (Tflag) {
		(void)printf(_(" TYPE"));
		if (lst->typemaxlen > 5)
			for (i = 5; i < lst->typemaxlen + 1; i++)
				(void)printf(" ");
		else
			lst->typemaxlen = 5;
	}

	/* option to display a wider bar */
	if (wflag) {
		barinc = 35;
	}
	if (!bflag) {
		(void)printf(_(" (=) USED"));
		for (i = 0; i < (barinc + 1); i++)
			(void)printf(" ");
		(void)printf(_("FREE (-) "));
	}

	(void)printf(_("%%USED"));

	if (dflag) {
		if (unitflag == 'k')
			(void)printf("       ");
		else if (unitflag == 'b')
			(void)printf("            ");
		else
			(void)printf("      ");
		(void)printf(_("USED"));
	}

	if (unitflag == 'k')
		(void)printf("  ");
	else if (unitflag == 'b')
		(void)printf("       ");
	else
		(void)printf(" ");

	(void)printf(_("AVAILABLE"));
	if (unitflag == 'k')
		(void)printf("      ");
	else if (unitflag == 'm')
		(void)printf("     ");
	else if (unitflag == 'b')
		(void)printf("           ");
	else
		(void)printf("     ");

	(void)printf(_("TOTAL"));

	if (iflag) {
		(void)printf(_("   #INODES"));
		(void)printf(_(" AV.INODES"));
	}

	(void)printf(_(" MOUNTED ON "));

	if (oflag) {
		for (i = 10; i < lst->dirmaxlen; i++)
			(void)printf(" ");
		(void)printf(_("MOUNT OPTIONS\n"));
	} else
		(void)printf("\n");

	reset_color();
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
text_disp_sum(struct list *lst, double stot, double atot, double utot,
              double ifitot, double ifatot)
{
	int i,j;
	double ptot = 0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;

	/* use color option is triggered */
	if (cflag)
		(void)printf("\033[;%dm", cnf.chead);
	(void)printf(_("SUM:"));
	reset_color();

	j = lst->fsmaxlen + 1;
	if (Tflag)
		j += lst->typemaxlen + 1;
	for (i = 4; i < j; i++)
		(void)printf(" ");

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
		text_disp_at(utot, ptot);
	text_disp_at(atot, ptot);
	text_disp_at(stot, ptot);

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

	(void)printf("]  ");
}

/*
 * Display available and total correctly formated
 * @n: number to print
 * @perct: percentage (useful for finding which color to use)
 */
static void
text_disp_at(double n, double perct)
{
	int i;

	change_color(perct);

	if (unitflag == 'h') {
		i = humanize(&n);
		change_color(perct);
		(void)printf(i == 0 ? "%9.f" : "%9.1f", n);
		reset_color();
		print_unit(i, 1);
	} else {
		change_color(perct);
		if (unitflag == 'b')
			(void)printf("%15.f", n);
		else if (unitflag == 'k')
			(void)printf("%10.f", n);
		else
			(void)printf("%9.1f", n);

		reset_color();
		print_unit(0, 1);
	}
}

/*
 * Display file system
 * @lst: list containing the information
 * @fsname: list of the file system to print
 */
static void
text_disp_fs(struct list *lst, char *fsname)
{
	int i;

	(void)printf("%s", fsname);
	for (i = (int)strlen(fsname); i < lst->fsmaxlen + 1; i++)
			(void)printf(" ");
}

/*
 * Display file system type
 * @lst: list containing the information
 * @type: the file system type to print
 */
static void
text_disp_type(struct list* lst, char *type)
{
	int i;

	(void)printf("%s", type);
	for (i = (int)strlen(type); i < lst->typemaxlen + 1; i++)
		(void)printf(" ");
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
		(void)printf("%9" PRIu64, files);
		print_unit(i, 0);
		i = humanize_i(&favail);
		(void)printf("%9" PRIu64, favail);
		print_unit(i, 0);
	} else {
		(void)printf(" %9" PRIu64, files);
		(void)printf(" %9" PRIu64, favail);
	}
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
text_disp_mount(char *dir)
{
	(void)printf(" %s", dir);
}

/*
 * Display mount options
 * @lst: structure containing information
 * @dir: mount point
 * @opts: mount options
 */
static void
text_disp_mopt(struct list* lst, char *dir, char *opts)
{
	int i;

	for (i = (int)strlen(dir);
		i < imax(lst->dirmaxlen + 1, 11); i++)
		(void)printf(" ");
	(void)printf("%s", opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
text_disp_perct(double perct)
{
	change_color(perct);
	(void)printf("%3.f", perct);
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
