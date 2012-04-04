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
 * disp.c
 *
 * Display functions
 */
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "disp.h"

/*
 * Actually displays infos in nice manner
 * @lst: queue containing all required information
 * @fsfilter: fstype to filter (can be nothing)
 */
void
disp(struct list *lst, char *fsfilter)
{
	struct fsmntinfo *p = NULL;
	int i, n;
	int skip = 1;
	int nm = 0;
	double perctused, size, avail, used;
	double stot, atot, utot, ifitot, ifatot;
	char *stropt;
	char *strtmp;

	stot = atot = utot = ifitot = ifatot = n = 0;

	/* activate negative matching? */
	if (tflag) {
		if (fsfilter[0] == '-') {
			nm = 1;
			skip = 0;
			fsfilter++;
		}
	}

	/* legend on top */
	if (!nflag)
		disp_header(lst);

	if (lst->fsmaxlen < 11)
		lst->fsmaxlen = 11;

	p = lst->head;

	while (p != NULL) {
		if (!aflag) {
			/* skip (pseudo)devices (which have a size of 0 usually) */
			if (p->blocks == 0) {
				p = p->next;
				continue;
				/*NOTREACHED */
			}
		}

		/* apply fsfiltering */
		if (tflag) {
			if ((strtmp = strdup(fsfilter)) == NULL) {
				(void)fprintf(stderr, "cannot duplicate "
					"fsfilter\n");
				exit(EXIT_FAILURE);
				/* NOTREACHED */
			}
			stropt = strtok(fsfilter, ",");
			while (stropt != NULL) {
				if (strcmp(p->type, stropt) == 0) {
					if (nm)
						skip = 1;
					else
						skip = 0;
				}
				stropt = strtok(NULL, ",");
			}
			/* strtok modifies fsfilter so give back its value */
			fsfilter = strdup(strtmp);
			if (skip) {
				if (nm)
					skip = 0;
				p = p->next;
				continue;
				/* NOTREACHED */
			} else {
				if (!nm)
					skip = 1;
			}
		}

		/* filesystem */
		(void)printf("%s", p->fsname);
		for (i = (int)strlen(p->fsname); i < lst->fsmaxlen + 1; i++)
			(void)printf(" ");

		/* type */
		if (Tflag) {
			(void)printf("%s", p->type);
			for (i = (int)strlen(p->type); i < lst->typemaxlen + 1; i++)
				(void)printf(" ");
		}
#ifdef __linux__
		size = (double)p->blocks *(double)p->frsize;
		avail = (double)p->bavail * (double)p->frsize;
		used = size - avail;
#else
		size = p->bsize * p->blocks;
		avail = p->bsize * p->bavail;
		used = p->bsize * (p->blocks - p->bfree);
#endif
		/* calculate the % used */
		if ((int)size == 0)
			perctused = 100.0;
		else
			perctused = (used / size) * 100.0;

		if (sflag) {
			stot += size;
			atot += avail;
			utot += used;
		}

		if (!bflag)
			disp_bar(perctused);

		/* %used */
		disp_perct(perctused);

		/* format to requested format */
		if (uflag) {
			size = cvrt(size);
			avail = cvrt(avail);
		}

		/* avail  and total */
		disp_at(avail, perctused);
		disp_at(size, perctused);

		/* info about inodes */
		if (iflag) {
			ifitot += (double)p->files;
			ifatot += (double)p->favail;
			(void)printf("%9ldk", p->files / 1000);
			(void)printf("%9ldk", p->favail / 1000);
		}

		/* mounted on */
		(void)printf(" %s", p->dir);

		/* info about mount option */
		if (oflag) {
			for (i = (int)strlen(p->dir);
					i < imax(lst->dirmaxlen + 1, 11); i++)
				(void)printf(" ");
			(void)printf("%s\n", p->opts);
		} else
			(void)printf("\n");

		p = p->next;
	}

	if (sflag)
		disp_sum(lst, stot, atot, utot, ifitot, ifatot);
}

/*
 * Display header
 * @lst: queue containing the information
 */
void
disp_header(struct list *lst)
{
	int i;
	int barinc = 5;

	/* use blue when color option is triggered */
	if (cflag)
		(void)printf("\033[;34m");

	(void)printf("FILESYSTEM ");
	for (i = 11; i < lst->fsmaxlen; i++)
		(void)printf(" ");

	if (Tflag) {
		(void)printf(" TYPE");
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
		(void)printf(" (=) USED");
		for (i = 0; i < (barinc + 1); i++)
			(void)printf(" ");
		(void)printf("FREE (-) ");
	}

	(void)printf("%%USED");
	if (unitflag == 'k')
		(void)printf("  ");
	else if (unitflag == 'b')
		(void)printf("       ");
	else
		(void)printf(" ");

	(void)printf("AVAILABLE");
	if (unitflag == 'k')
		(void)printf("      ");
	else if (unitflag == 'm')
		(void)printf("     ");
	else if (unitflag == 'b')
		(void)printf("           ");
	else
		(void)printf("     ");

	(void)printf("TOTAL");

	if (iflag) {
		(void)printf("   #INODES");
		(void)printf(" AV.INODES");
	}

	(void)printf(" MOUNTED ON ");

	if (oflag) {
		for (i = 10; i < lst->dirmaxlen; i++)
			(void)printf(" ");
		(void)printf("MOUNT OPTIONS\n");
	} else
		(void)printf("\n");

	reset_color();
}

/*
 * display the sum (used when -s option is triggered)
 * @lst: queue containing the information
 * @stot: size total
 * @utot:
 */
void
disp_sum(struct list *lst, double stot, double atot, double utot,
		double ifitot, double ifatot)
{
	int i,j;
	double ptot = 0;

	if ((int)stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;
	(void)printf("SUM:");

	j = lst->fsmaxlen + 1;
	if (Tflag)
		j += lst->typemaxlen + 1;
	for (i = 4; i < j; i++)
		(void)printf(" ");

	if (!bflag)
		disp_bar(ptot);

	disp_perct(ptot);

	if (uflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
	}

	disp_at(atot, ptot);
	disp_at(stot, ptot);

	if (ifitot && ifatot) {
		(void)printf("%9.fk", ifitot / 1000);
		(void)printf("%9.fk", ifatot / 1000);
	}

	(void)printf("\n");
}

/*
 * Display nice usage bar
 * @perct: percentage value
 */
void
disp_bar(double perct)
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
			(void)printf("=");

		for (j = i; j < 100; j += barinc)
			(void)printf("-");
	} else { /* color */

		/* green */
		(void)printf("\033[;32m");
		for (i = 0; (i < 50) && (i < perct); i += barinc)
			(void)printf("=");

		/* yellow */
		(void)printf("\033[;33m");
		for (; (i < 75) && (i < perct); i += barinc)
			(void)printf("=");

		/* red */
		(void)printf("\033[;31m");
		for (; (i < 100) && (i < perct); i += barinc)
			(void)printf("=");

		reset_color();

		for (j = i; j < 100; j += barinc)
			(void)printf("-");
	}

	(void)printf("]  ");
}

/*
 * Display available and total correctly formated
 * TODO: men... this is really UGLY! Just figure out something!!
 * @n: number to print
 * @perct: percentage (useful for finding which color to use)
 */
void
disp_at(double n, double perct)
{
    int i;

	change_color(perct);

	/* available  and total */
	switch (unitflag) {
	case 'h':
		i = humanize(&n);
		change_color(perct);
		(void)printf(i == 0 ? "%9.f" : "%9.1f", n);
		reset_color();
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
		(void)printf("%15.f", n);
		reset_color();
		(void)printf("B");
		return;
		/* NOTREACHED */
	case 'k':
		(void)printf("%10.f", n);
		reset_color();
		(void)printf("K");
		return;
		/* NOTREACHED */
	}

	(void)printf("%9.1f", n);
	reset_color();

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

/*
 * Display percentage
 * @perct: percentage
 */
void
disp_perct(double perct)
{
	if (!cflag) {
		(void)printf("%3.f%%", perct);
	} else {
		if (perct < 50.0) /* green */
			(void)printf("\033[;32m");
		else if (perct < 75.0) /* yellow */
			(void)printf("\033[;33m");
		else /* red */
			(void)printf("\033[;31m");

		(void)printf("%3.f", perct);
		reset_color();
		(void)printf("%%");
	}
}

/*
 * Change color according to perct
 * @perct: percentage
 */
void
change_color(double perct)
{
	if (cflag) {
		if (perct < 50.0) /* green */
			(void)printf("\033[;32m");
		else if (perct < 75.0) /* yellow */
			(void)printf("\033[;33m");
		else /* red */
			(void)printf("\033[;31m");
	}
}

/*
 * Reset color attribute to default
 */
void
reset_color(void)
{
	if (cflag)
		(void)printf("\033[;m");
}
