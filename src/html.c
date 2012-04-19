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
    (void) fprintf(stdout, "<!DOCTYPE html>");
    (void) fprintf(stdout, "<html><head>");
    (void) fprintf(stdout, "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/>");
    (void) fprintf(stdout, "<title>dfc</title></head><body>");
}

static void
html_disp_deinit(void)
{
    (void) fprintf(stdout, "</tr></table></body></html>");
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
    (void) fprintf(stdout, "<table border=\"1\"><tr>");
    (void) fprintf(stdout, "<th>%s</th>", _("FILESYSTEM"));
    if (Tflag)
        (void) fprintf(stdout, "<th>%s</th>", _("TYPE"));
    if (!bflag)
        (void) fprintf(stdout, "<th>BAR</th>");
    (void) fprintf(stdout, "<th>%s</th>", _("%USED"));
    (void) fprintf(stdout, "<th>%s</th>", _("AVAILABLE"));
    (void) fprintf(stdout, "<th>%s</th>", _("TOTAL"));
    if (iflag) {
        (void) fprintf(stdout, "<th>%s</th>", _("#INODES"));
        (void) fprintf(stdout, "<th>%s</th>", _("AV.INODES,"));
    }
    (void) fprintf(stdout, "<th>%s</th>", _("MOUNTED ON"));
    if (oflag)
        (void) fprintf(stdout, "<th>%s</th></tr>", _("MOUNT OPTIONS"));
}

void
html_disp_sum(struct list *lst, double stot, double atot, double utot,
              double ifitot, double ifatot)
{
    (void) lst;

    double ptot = stot==0?100.0:100.0*(utot/stot);

    (void) fprintf(stdout, "</tr><tr>");
    (void) fprintf(stdout, "<td>Sum</td>");
    if (Tflag)
        (void) fprintf(stdout, "<td>N/A</td>");

    if (!bflag)
        (void) fprintf(stdout, "<td>N/A</td>");
    html_disp_perct(ptot);
    if (uflag) {
        stot = cvrt(stot);
        atot = cvrt(atot);
    }
    html_disp_at(atot, ptot);
    html_disp_at(stot, ptot);

    if (ifitot && ifatot) {
        (void) fprintf(stdout, "<td>%9.fk</td>", ifitot/1000);
        (void) fprintf(stdout, "<td>%9.fk</td>", ifatot/1000);
    }
}

void
html_disp_bar(double perct)
{
    fprintf(stdout, "<td>");
    int barwidth = 100; /* In pixels */
    int barheight = 25; /* In pixels */

    if (wflag)
        barwidth *= 2;

    if (!cflag) {
        (void) fprintf(stdout,
                       "<span style=\"width:%dpx; height:%dpx;background-color:black;float:left\"></span>",
                       (int)perct*barwidth/100, barheight);
    } else { /* color */
        int size = (perct < 50.0) ? (int)perct:50;
        (void) fprintf(stdout,
                       "<span style=\"width:%dpx; height:%dpx;background-color:green;float:left\"></span>",
                       size*barwidth/100, barheight);

        if (perct >= 50.0) {
            size = (perct < 75.0) ? (int)perct:75;
            size -= 50;
            (void) fprintf(stdout,
                           "<span style=\"width:%dpx; height:%dpx;background-color:yellow;float:left\"></span>",
                           size*barwidth/100, barheight);
        }

        if (perct >= 75.0) {
            size = (int)perct - 75;
            (void) fprintf(stdout,
                           "<span style=\"width:%dpx; height:%dpx;background-color:red;float:left\"></span>",
                           size*barwidth/100, barheight);
        }

        if (perct < 100.0) {
            size = 100 - (int) perct;
            (void) fprintf(stdout,
                           "<span style=\"width:%dpx; height:%dpx;background-color:black;float:left\"></span>",
                           size*barwidth/100, barheight);
        }
    }
    (void) fprintf(stdout, "</td>");
}

void
html_disp_at(double n, double perct)
{
    (void) perct;
    int i;
    (void) fprintf(stdout, "<td>");
    /* XXX: This is a huge copy/paste of src/text.c. This should probably be
     * factorized in src/util.c */
    /* available  and total */
    switch (unitflag) {
        case 'h':
            i = humanize(&n);
            (void) fprintf(stdout, i == 0 ? "%9.f" : "%9.1f", n);
            switch (i) {
                case 0:	/* bytes */
                    (void) fprintf(stdout, "B");
                    break;
		case 1: /* Kio  or Ko */
                    (void) fprintf(stdout, "K");
                    break;
                case 2: /* Mio or Mo */
                    (void) fprintf(stdout, "M");
                    break;
                case 3: /* Gio or Go*/
                    (void) fprintf(stdout, "G");
                    break;
                case 4: /* Tio or To*/
                    (void) fprintf(stdout, "T");
                    break;
                case 5: /* Pio or Po*/
                    (void) fprintf(stdout, "P");
                    break;
                case 6: /* Eio or Eo*/
                    (void) fprintf(stdout, "E");
                     break;
                case 7: /* Zio or Zo*/
                    (void) fprintf(stdout, "Z");
                    break;
                case 8: /* Yio or Yo*/
                    (void) fprintf(stdout, "Y");
                     break;
            }
            return;
        case 'b':
            (void) fprintf(stdout, "%15.f", n);
            (void) fprintf(stdout, "B");
            return;
        case 'k':
            (void) fprintf(stdout, "%10.f", n);
            (void) fprintf(stdout, "K");
            return;
    }

    (void) fprintf(stdout, "%9.1f", n);

    switch (unitflag) {
        case 'm':
            (void) fprintf(stdout, "M");
            break;
        case 'g':
            (void) fprintf(stdout, "G");
            break;
        case 't':
            (void) fprintf(stdout, "T");
            break;
        case 'p':
            (void) fprintf(stdout, "P");
            break;
        case 'e':
            (void) fprintf(stdout, "E");
            break;
        case 'z':
            (void) fprintf(stdout, "Z");
            break;
        case 'y':
            (void) fprintf(stdout, "Y");
            break;
    }
    (void) fprintf(stdout, "</td>");
}

void
html_disp_fs(struct list *lst, char *fsname)
{
    (void) lst;
    static int must_close = 0;
    if (must_close == 1)
        fprintf(stdout, "</tr>");
    fprintf(stdout, "<tr><td>%s</td>", fsname);
    must_close = 1;
}

void
html_disp_type(struct list *lst, char *type)
{
    (void) lst;
    fprintf(stdout, "<td>%s</td>", type);
}

void
html_disp_inodes(unsigned long files, unsigned long favail)
{
    (void) fprintf(stdout, "<td>%9ldk</td>", files);
    (void) fprintf(stdout, "<td>%9ldk</td>", favail);
}

void
html_disp_mount(char *dir)
{
    (void) fprintf(stdout, "<td>%s</td>", dir);
}

void
html_disp_mopt(struct list *lst, char *dir, char *opts)
{
    (void) lst;
    (void) dir;
    (void) fprintf(stdout, "<td>%s</td>", opts);
}

void
html_disp_perct(double perct)
{
    (void) fprintf(stdout, "<td>%2.f</td>", perct);
}
