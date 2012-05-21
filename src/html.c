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
 * html.c
 *
 * HTML display functions
 */
#include <stdio.h>
#include <stdlib.h>

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

/* static functions declaration */
static void html_disp_init(void);
static void html_disp_deinit(void);
static void html_disp_header(struct list *lst);
static void html_disp_sum(struct list *lst, double stot, double utot, double ftot,
                   double ifitot, double ifatot);
static void html_disp_bar(double perct);
static void html_disp_at(double n, double perct);
static void html_disp_fs(struct list *lst, char *fsname);
static void html_disp_type(struct list *lst, char *type);
static void html_disp_inodes(uint64_t files, uint64_t favail);
static void html_disp_mount(char *dir);
static void html_disp_mopt(struct list *lst, char *dir, char *opts);
static void html_disp_perct(double perct);

/* init pointers from display structure to the functions found here */
void
init_disp_html(struct display *disp)
{
	disp->init         = html_disp_init;
	disp->deinit       = html_disp_deinit;
	disp->print_header = html_disp_header;
	disp->print_sum    = html_disp_sum;
	disp->print_bar    = html_disp_bar;
	disp->print_at     = html_disp_at;
	disp->print_fs     = html_disp_fs;
	disp->print_type   = html_disp_type;
	disp->print_inodes = html_disp_inodes;
	disp->print_mount  = html_disp_mount;
	disp->print_mopt   = html_disp_mopt;
	disp->print_perct  = html_disp_perct;
}

/*
 * Print DOCTYPE and everything that is required before the html body
 */
static void
html_disp_init(void)
{
	(void)puts("<!DOCTYPE html>");
	(void)puts("<html>");
	(void)puts("  <head>");
	(void)puts("    <meta http-equiv=\"Content-Type\" content=\"text/html; "
			"charset=utf-8\"/>");
	(void)puts("    <meta name=\"author\" content=\"Robin Hahling\"/>");
	(void)printf("    <meta name=\"description\" content=\"%s-%s - Display "
			"file system space usage using graph and colors\"/>\n",
			PACKAGE, VERSION);
	(void)puts("    <meta name=\"keywords\" content=\"dfc,file system, usage, "
			"display, cli, df\"/>");
	(void)puts("    <style type=\"text/css\">");
	(void)puts("\ttable { border-collapse: collapse; border: 1px solid #333; }");
	(void)puts("\ttd, th { padding: 0.5em; border: 1px #BBBBBB solid; }");
	if (cflag) {
		(void)printf("\tthead, tfoot { background-color: #%s; color: #%s; }\n",
			cnf.hcheadbg, cnf.hcheadfg);
		(void)printf("\ttbody { background-color: #%s; color: #%s }\n",
			cnf.hccellbg, cnf.hccellfg);
		(void)printf("\ttbody tr:hover { background-color: #%s; color: #%s; }\n",
			cnf.hchoverbg, cnf.hchoverfg);
	} else {
		(void)puts("\tthead, tfoot { background-color: gray; color: #FFFFFF; }");
		(void)puts("\ttbody { background-color: #E9E9E9; color: #000000 }");
		(void)puts("\ttbody tr:hover { background-color: #FFFFFF; color: #000000; }");
	}
	(void)puts("    </style>");
	(void)printf("    <title>%s-%s</title>", PACKAGE, VERSION);
	(void)puts("  </head>\n  <body>");
}

/*
 * Close all open html tag that need to be closed after html body
 */
static void
html_disp_deinit(void)
{
    (void)puts("\t</tr>");
    if (sflag)
	    (void)puts("\t</tfoot>");
    (void)puts("    </table>\n  </body>\n</html>");
}

/*
 * Display header
 * @lst: is ignored here
 */
static void
html_disp_header(struct list *lst)
{
	char *date;

	(void) lst;

	if ((date = fetchdate()) == NULL)
		date = _("Unknown date");

	(void)printf("    <table>\n    <caption style = \"caption-side: bottom;\">");
	(void)printf(_("Generated by %s-%s on %s"), PACKAGE, VERSION, date);
	(void)puts("</caption>");
	(void)puts("\t<thead>\n\t<tr>");
	(void)printf("\t  <th>%s</th>\n", _("FILESYSTEM"));

	if (Tflag)
		(void)printf("\t  <th>%s</th>\n", _("TYPE"));
	if (!bflag)
		(void)printf("\t  <th>USAGE</th>\n");

	(void)printf("\t  <th>%s</th>\n", _("%USED"));
	if (dflag)
		(void)printf("\t  <th>%s</th>\n", _("USED"));
	(void)printf("\t  <th>%s</th>\n", _("AVAILABLE"));
	(void)printf("\t  <th>%s</th>\n", _("TOTAL"));

	if (iflag) {
		(void)printf("\t  <th>%s</th>\n", _("#INODES"));
		(void)printf("\t  <th>%s</th>\n", _("AV.INODES,"));
	}

	(void)printf("\t  <th>%s</th>\n", _("MOUNTED ON"));

	if (oflag)
		(void)printf("\t  <th>%s</th>\n", _("MOUNT OPTIONS"));

	(void)puts("\t</tr>\n\t</thead>");
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
html_disp_sum(struct list *lst, double stot, double atot, double utot,
              double ifitot, double ifatot)
{
	double ptot = stot == 0 ? 100.0 : 100.0 * (utot / stot);

	(void)lst;

	(void)puts("\t</tr>\n\t<tfoot>\n\t<tr>\n\t  <td><strong>SUM</strong></td>");

	if (Tflag)
		(void)puts("\t  <td>N/A</td>");

	if (!bflag)
		html_disp_bar(ptot);

	html_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
		if (dflag)
			utot = cvrt(utot);
	}

	if (dflag)
		html_disp_at(utot, ptot);

	html_disp_at(atot, ptot);
	html_disp_at(stot, ptot);

	if (iflag) {
		(void)printf("\t  <td style = \"text-align: right;\">%.fk</td>\n", ifitot / 1000.0);
		(void)printf("\t  <td style = \"text-align: right;\">%.fk</td>\n", ifatot / 1000.0);
	}

	/* keep same amount of columns in table */
	(void)puts("\t  <td>N/A</td>");
	if (oflag)
		(void)puts("\t  <td>N/A</td>");
}

/*
 * Display the nice usage bar
 * @perct: percentage value
 */
static void
html_disp_bar(double perct)
{
	int barwidth = 100; /* In pixels */
	int barheight = 25; /* In pixels */
	int size;

	(void)puts("\t  <td>");

	if (wflag)
		barwidth *= 2;

	if (!cflag) {
		(void)printf("\t    <span style=\"width: %dpx; height: %dpx; "
			"background-color:silver; float: left;\"></span>\n",
                       (int)perct*barwidth/100, barheight);
	} else { /* color */
		size = (perct < cnf.gmedium) ? (int)perct : cnf.gmedium;
		(void)printf("\t    <span style=\"width:%dpx; height: %dpx; "
			"background-color: #%s; float: left;\"></span>\n",
                       size * barwidth / 100, barheight, cnf.hclow);

		if (perct >= cnf.gmedium) {
			size = (perct < cnf.ghigh) ? (int)perct : cnf.ghigh;
			size -= cnf.gmedium;
			(void)printf("\t    <span style=\"width: %dpx; height: %dpx; "
			    "background-color: #%s; float: left;\"></span>\n",
                           size * barwidth / 100, barheight, cnf.hcmedium);
		}

		if (perct >= cnf.ghigh) {
			size = (int)perct - cnf.ghigh;
			(void)printf("\t    <span style=\"width: %dpx; height: %dpx; "
				"background-color: #%s; float: left;\"></span>\n",
                           size * barwidth / 100, barheight, cnf.hchigh);
		}
	}
	(void)puts("\t  </td>");
}

static void
html_disp_at(double n, double perct)
{
	int i;

	(void)perct;
	(void)printf("\t  <td style = \"text-align: right;\">");

	/* XXX: This is a huge copy/paste of src/text.c. This should probably be
	* factorized in src/util.c */
	/* available  and total */
	switch (unitflag) {
	case 'h':
		i = humanize(&n);
		(void)printf(i == 0 ? "%.f" : "%.1f", n);
		switch (i) {
		case 0:	/* bytes */
			(void)printf("B");
			break;
		case 1: /* Kio  or Ko */
			(void)printf("K");
			break;
		case 2: /* Mio or Mo */
			(void)printf("M");
			break;
		case 3: /* Gio or Go*/
			(void)printf("G");
			break;
		case 4: /* Tio or To*/
			(void)printf("T");
			break;
		case 5: /* Pio or Po*/
			(void)printf("P");
			break;
		case 6: /* Eio or Eo*/
			(void)printf("E");
			break;
		case 7: /* Zio or Zo*/
			(void)printf("Z");
			break;
		case 8: /* Yio or Yo*/
			(void)printf("Y");
			break;
		}
		(void)puts("</td>");
		return;
	case 'b':
		(void)printf("%.f", n);
		(void)puts("B</td>");
		return;
	case 'k':
		(void)printf("%.f", n);
		(void)puts("K</td>");
		return;
	}

	(void)printf("%.1f", n);

	switch (unitflag) {
	case 'm':
		(void)printf("M");
		break;
	case 'g':
		(void)printf("G");
		break;
	case 't':
		(void)printf("T");
		break;
	case 'p':
		(void)printf("P");
		break;
	case 'e':
		(void)printf("E");
		break;
	case 'z':
		(void)printf("Z");
		break;
	case 'y':
		(void)printf("Y");
		break;
	}
	(void)puts("\t  </td>");
}

/*
 * Display file system
 * @lst: is ignored here
 * @fsname: list of the file system to print
 */
static void
html_disp_fs(struct list *lst, char *fsname)
{
	static int must_close = 0;

	(void)lst;

	if (must_close == 1)
		(void)puts("\t</tr>");

	(void)printf("\t<tr>\n\t  <td>%s</td>\n", fsname);
	must_close = 1;
}

/*
 * Display file system type
 * @lst: is ignored here
 * @type: the file system type to print
 */
static void
html_disp_type(struct list *lst, char *type)
{
	(void)lst;
	(void)printf("\t  <td>%s</td>\n", type);
}

/*
 * Display inodes
 *@files: number of inodes
 *@favail: number of available inodes
 */
static void
html_disp_inodes(uint64_t files, uint64_t favail)
{
	(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64 "k</td>\n",
			files);
	(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64 "k</td>\n",
			favail);
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
html_disp_mount(char *dir)
{
	(void)printf("\t  <td>%s</td>", dir);
}

/*
 * Display mount options
 * @lst: is ignored here
 * @dir: is ignored here
 * @opts: mount options
 */
static void
html_disp_mopt(struct list *lst, char *dir, char *opts)
{
	(void)lst;
	(void)dir;

	(void)printf("\n\t  <td>%s</td>", opts);
}

/*
 * Display percentage
 * @perct: percentage
 */
static void
html_disp_perct(double perct)
{
	(void)printf("\t  <td style = \"text-align: right;\">%.f%%</td>\n", perct);
}
