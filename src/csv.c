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

#include <libintl.h>

#include "csv.h"
#include "extern.h"
#include "util.h"

void
init_disp_csv(struct Display *disp)
{
    disp->init         = NULL;
    disp->deinit       = NULL;
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

void
csv_disp_header(struct list *lst)
{
	/* do not care about lst in CSV output */
	(void)lst;

	(void)printf(_("FILESYSTEM,"));

	if (Tflag)
		(void)printf(_("TYPE,"));

	(void)printf(_("%%USED,"));

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

void
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
	}

	csv_disp_at(atot, ptot);
	csv_disp_at(stot, ptot);

	if (iflag) {
		(void)printf(",%.f,k", ifitot / 1000);
		(void)printf(",%.f,k", ifatot / 1000);
	}

	(void)printf("\n");
}

void
csv_disp_bar(double perct)
{
	(void)perct;
	/* DUMMY */
}

void
csv_disp_at(double n, double perct)
{
	int i;

	/* do not care about perct here */
	(void)perct;

	(void)printf(",");

	/* available  and total */
	switch (unitflag) {
	case 'h':
		i = humanize(&n);
		(void)printf(i == 0 ? "%.f," : "%.1f,", n);
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
		/* NOTREACHED */
	case 'b':
		(void)printf("%f,B,", n);
		return;
		/* NOTREACHED */
	case 'k':
		(void)printf("%f,K,", n);
		return;
		/* NOTREACHED */
	}

	(void)printf("%.1f,", n);

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
}

void
csv_disp_fs(struct list *lst, char *fsname)
{
	/* we do not care about lst here */
	(void)lst;

	(void)printf("%s,",fsname);
}

void
csv_disp_type(struct list *lst, char *type)
{
	/* we do not care about lst here */
	(void)lst;

	(void)printf("%s,", type);
}

void
csv_disp_inodes(unsigned long files, unsigned long favail)
{
	(void)printf(",%ld,k", files);
	(void)printf(",%ld,k", favail);
}

void
csv_disp_mount(char *dir)
{
	(void)printf(",%s", dir);
}

void
csv_disp_mopt(struct list *lst, char *dir, char *opts)
{
	/* we do not care about lst here */
	(void)lst;
	/* neither about dir */
	(void)dir;

	(void)printf(",%s", opts);
}

void
csv_disp_perct(double perct)
{
	(void)printf("%.f,%%", perct);
}
