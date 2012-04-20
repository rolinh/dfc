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

#include <libintl.h>

#include "extern.h"
#include "html.h"
#include "util.h"

/*
 * TODO: Add some CSS to make this cool :)
 */
static void
html_disp_init(void)
{
	(void)printf("<!DOCTYPE html>");
	(void)printf("<html><head>");
	(void)printf("<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/>");
	(void)printf("<title>dfc</title></head><body>");
}

static void
html_disp_deinit(void)
{
    (void)printf("</tr></table></body></html>");
}

void
init_disp_html(struct Display *disp)
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

void
html_disp_header(struct list *lst)
{
	(void) lst;

	(void)printf("<table border=\"1\"><tr>");
	(void)printf("<th>%s</th>", _("FILESYSTEM"));

	if (Tflag)
		(void)printf("<th>%s</th>", _("TYPE"));
	if (!bflag)
		(void)printf("<th>BAR</th>");

	(void)printf("<th>%s</th>", _("%USED"));
	(void)printf("<th>%s</th>", _("AVAILABLE"));
	(void)printf("<th>%s</th>", _("TOTAL"));

	if (iflag) {
		(void)printf("<th>%s</th>", _("#INODES"));
		(void)printf("<th>%s</th>", _("AV.INODES,"));
	}

	(void)printf("<th>%s</th>", _("MOUNTED ON"));

	if (oflag)
		(void)printf("<th>%s</th></tr>", _("MOUNT OPTIONS"));
}

void
html_disp_sum(struct list *lst, double stot, double atot, double utot,
              double ifitot, double ifatot)
{
	double ptot = stot == 0 ? 100.0 : 100.0 * (utot / stot);

	(void)lst;

	(void)fprintf(stdout, "</tr><tr>");
	(void)fprintf(stdout, "<td>Sum</td>");

	if (Tflag)
		(void)printf("<td>N/A</td>");

	if (!bflag)
		(void)fprintf(stdout, "<td>N/A</td>");

	html_disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
	}

	html_disp_at(atot, ptot);
	html_disp_at(stot, ptot);

	if (ifitot && ifatot) {
		(void)fprintf(stdout, "<td>%9.fk</td>", ifitot / 1000.0);
		(void)fprintf(stdout, "<td>%9.fk</td>", ifatot / 1000.0);
	}
}

void
html_disp_bar(double perct)
{
	int barwidth = 100; /* In pixels */
	int barheight = 25; /* In pixels */
	int size;

	(void)printf("<td>");

	if (wflag)
		barwidth *= 2;

	if (!cflag) {
	(void)printf("<span style=\"width:%dpx; height:%dpx;background-color:black;float:left\"></span>",
                       (int)perct*barwidth/100, barheight);
	} else { /* color */
		size = (perct < 50.0) ? (int)perct : 50;
        (void)printf("<span style=\"width:%dpx; height:%dpx;background-color:green;float:left\"></span>",
                       size * barwidth / 100, barheight);

        if (perct >= 50.0) {
		size = (perct < 75.0) ? (int)perct:75;
		size -= 50;
            (void)printf("<span style=\"width:%dpx; height:%dpx;background-color:yellow;float:left\"></span>",
                           size * barwidth / 100, barheight);
        }

        if (perct >= 75.0) {
		size = (int)perct - 75;
		(void)printf("<span style=\"width:%dpx; height:%dpx;background-color:red;float:left\"></span>",
                           size * barwidth / 100, barheight);
        }

        if (perct < 100.0) {
		size = 100 - (int)perct;
		(void)printf("<span style=\"width:%dpx; height:%dpx;background-color:black;float:left\"></span>",
                           size * barwidth / 100, barheight);
        }
    }
    (void)printf("</td>");
}

void
html_disp_at(double n, double perct)
{
	int i;

	(void)perct;
	(void)printf("<td>");

	/* XXX: This is a huge copy/paste of src/text.c. This should probably be
	* factorized in src/util.c */
	/* available  and total */
	switch (unitflag) {
	case 'h':
		i = humanize(&n);
		(void)printf(i == 0 ? "%9.f" : "%9.1f", n);
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
		return;
	case 'b':
		(void)printf("%15.f", n);
		(void)printf("B");
		return;
	case 'k':
		(void)printf("%10.f", n);
		(void)printf("K");
		return;
	}

	(void)printf("%9.1f", n);

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
	(void)printf("</td>");
}

void
html_disp_fs(struct list *lst, char *fsname)
{
	static int must_close = 0;

	(void)lst;

	if (must_close == 1)
		(void)printf("</tr>");

	(void)printf("<tr><td>%s</td>", fsname);
	must_close = 1;
}

void
html_disp_type(struct list *lst, char *type)
{
	(void)lst;
	(void)printf("<td>%s</td>", type);
}

void
html_disp_inodes(unsigned long files, unsigned long favail)
{
	(void)printf("<td>%9ldk</td>", files);
	(void)printf("<td>%9ldk</td>", favail);
}

void
html_disp_mount(char *dir)
{
	(void)printf("<td>%s</td>", dir);
}

void
html_disp_mopt(struct list *lst, char *dir, char *opts)
{
	(void)lst;
	(void)dir;

	(void)printf("<td>%s</td>", opts);
}

void
html_disp_perct(double perct)
{
	(void)printf("<td>%2.f</td>", perct);
}
