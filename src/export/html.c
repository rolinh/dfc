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
 * html.c
 *
 * HTML display functions
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

/* static functions declaration */
static void html_disp_init(void);
static void html_disp_deinit(void);
static void html_disp_header(void);
static void html_disp_sum(double stot, double utot, double ftot,
                   double ifitot, double ifatot);
static void html_disp_bar(double perct);
static void html_disp_uat(double n, double perct, int req_width);
static void html_disp_fs(const char *fsname);
static void html_disp_type(const char *type);
static void html_disp_inodes(uint64_t files, uint64_t favail);
static void html_disp_mount(const char *dir);
static void html_disp_mopt(const char *opts);
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
	disp->print_uat    = html_disp_uat;
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
 */
static void
html_disp_header(void)
{
	char *date;

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
	free(date);
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
html_disp_sum(double stot, double atot, double utot,
              double ifitot, double ifatot)
{
	double ptot = 0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;

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
		html_disp_uat(utot, ptot, 0);

	html_disp_uat(atot, ptot, 0);
	html_disp_uat(stot, ptot, 0);

	if (iflag)
		html_disp_inodes((uint64_t)ifitot, (uint64_t)ifatot);

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

/*
 * Display available and total correctly formatted
 * @n: number to print
 * @perct: ignored here
 * @req_width: ignored here
 */
static void
html_disp_uat(double n, double perct, int req_width)
{
	int i;

	(void)perct;
	(void)req_width;

	(void)printf("\t  <td style = \"text-align: right;\">");

	if (unitflag == 'h') {
		i = humanize(&n);
		(void)printf(i == 0 ? "%.f" : "%.1f", n);
		print_unit(i, 1);
	} else {
		if (unitflag == 'b' || unitflag == 'k')
			(void)printf("%.f", n);
		else
			(void)printf("%.1f", n);
	}
	(void)puts("</td>");
}

/*
 * Display file system
 * @fsname: list of the file system to print
 */
static void
html_disp_fs(const char *fsname)
{
	static int must_close = 0;

	if (must_close == 1)
		(void)puts("\t</tr>");

	(void)printf("\t<tr>\n\t  <td>%s</td>\n", fsname);
	must_close = 1;
}

/*
 * Display file system type
 * @type: the file system type to print
 */
static void
html_disp_type(const char *type)
{
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
	int i;

	if (unitflag == 'h') {
		i = humanize_i(&files);
		(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64,
				files);
		print_unit(i, 0);
		(void)printf("</td>\n");
		i = humanize_i(&favail);
		(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64,
				favail);
		print_unit(i, 0);
		(void)printf("</td>\n");
	} else {
		(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64
				"</td>\n", files);
		(void)printf("\t  <td style = \"text-align: right;\">%" PRIu64
				"</td>\n", favail);
	}
}

/*
 * Display mount point
 * @dir: mount point
 */
static void
html_disp_mount(const char *dir)
{
	(void)printf("\t  <td>%s</td>", dir);
}

/*
 * Display mount options
 * @opts: mount options
 */
static void
html_disp_mopt(const char *opts)
{
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
