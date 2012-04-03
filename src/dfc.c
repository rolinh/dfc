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
 * dfc.c
 *
 * (C) 2012 - Hahling Robin <robin.hahling@gw-computing.net>
 *
 * Displays free disk space in an elegant manner.
 */

/* What works for FreeBSD works for MacOS */
#ifndef __linux__
#define BSD
#endif

#define _BSD_SOURCE

#ifdef __linux
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 500
#endif

#define STRMAXLEN 24

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#ifdef __linux__
#include <mntent.h>
#endif
#include <string.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/statvfs.h>

#ifdef BSD
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#include "dfc.h"

int
main(int argc, char *argv[])
{
	struct list queue;
	int ch;
	unsigned int width;
	char *fsfilter = NULL;
	char *subopts;
	char *value;

	char *unit_opts[] = {
		#define H	0
			"h",
		#define B	1
			"b",
		#define K	2
			"k",
		#define M	3
			"m",
		#define G	4
			"g",
		#define T	5
			"t",
		#define P	6
			"p",
		#define E	7
			"e",
		#define Z	8
			"z",
		#define Y	9
			"y",
		NULL
	};

	char *color_opts[] = {
		#define CALWAYS	0
			"always",
		#define	CNEVER	1
			"never",
		#define	CAUTO	2
			"auto",
		NULL
	};


	/* default value for those globals */
	cflag = 1; /* color enabled by default */

	/*
	 * Now use -u option for choosing the size (b, k, m, etc.)
	 * When using the flag, should specifie the unit used (h is default).
	 * Have a look at unit_opts for the possible values.
	 */
	unitflag = 'h';

	while ((ch = getopt(argc, argv, "abc:fhimnost:Tu:vwW")) != -1) {
		switch (ch) {
		case 'a':
			aflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		case 'c':
			subopts = optarg;
			while (*subopts) {
				switch (getsubopt(&subopts, color_opts, &value)) {
				case CALWAYS:
					cflag = 2;
					break;
				case CNEVER:
					cflag = 0;
					break;
				case CAUTO:
					cflag = 1;
					break;
				case -1:
					(void)fprintf(stderr,
						"-c: illegal sub option %s\n",
						subopts);
					return EXIT_FAILURE;
					/* NOTREACHED */
				}
			}
			break;
		case 'f':
			fflag = 1;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'm':
			mflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'o':
			oflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 't':
			tflag = 1;
			fsfilter = strdup(optarg);
			break;
		case 'T':
			Tflag = 1;
			break;
		case 'u':
			uflag = 1;
			subopts = optarg;
			while (*subopts) {
				switch (getsubopt(&subopts, unit_opts, &value)) {
				case H:
					unitflag = 'h';
					break;
				case B:
					unitflag = 'b';
					break;
				case K:
					unitflag = 'k';
					break;
				case M:
					unitflag = 'm';
					break;
				case G:
					unitflag = 'g';
					break;
				case T:
					unitflag = 't';
					break;
				case P:
					unitflag = 'p';
					break;
				case E:
					unitflag = 'e';
					break;
				case Z:
					unitflag = 'z';
					break;
				case Y: unitflag = 'y';
					break;
				case -1:
					(void)fprintf(stderr,
						"-u: illegal sub option %s\n",
						subopts);
					return EXIT_FAILURE;
					/* NOTREACHED */
				}
			}
			break;
		case 'v':
			vflag = 1;
			break;
		case 'w':
			wflag = 1;
			break;
		case 'W':
			Wflag = 1;
			break;
		case '?':
		default:
			usage(EXIT_FAILURE);
			/* NOTREACHED */
		}
	}

	if (hflag)
		usage(EXIT_SUCCESS);
		/* NOTREACHED */

	if (vflag) {
		(void)printf("dfc %s\n", VERSION);
		return EXIT_SUCCESS;
		/* NOTREACHED */
	}

	width = getttywidth();

	/* if fd is not a terminal and color mode is not "always", disable color */
	if (width == 0 && cflag != 2)
		cflag = 0;

	/* cannot display all information if tty is too narrow */
	if (!fflag) {
		if (width < 151) {
			if (oflag) {
				Tflag = 0;
				bflag = 1;
			}
		}
		if (width < 125)
			oflag = 0;
		if (width < 81) {
			bflag = 1;
			Tflag = 0;
		}
	}

	/* initializes the queue */
	init_queue(&queue);

	/* fetch information about the currently mounted filesystems */
	fetch_info(&queue);

	/* actually displays the info we have got */
	disp(&queue, fsfilter);

	return EXIT_SUCCESS;
	/* NOTREACHED */
}

/*
 * Display usage.
 * param: status --> status code (EXIT_SUCCESS, EXIT_FAILURE, ...)
 */
void
usage(int status)
{
	if (status != 0)
		(void)fputs("Try dfc -h for more information\n", stderr);
	else {
		/* 2 fputs because string length limit is 509 */
		(void)fputs("Usage: dfc [OPTIONS(S)] [-c WHEN] [-u UNIT]"
			"[-t FILESYSTEM]\n"
			"Available options:\n"
			"\t-a\tprint all fs from mtab\n"
			"\t-b\tdo not show the graph bar\n"
			"\t-c\tchoose color mode. Read the manpage\n"
			"\t\tfor details\n",
			stdout);
		(void)fputs(
			"\t-f\tdisable auto-adjust mode (force display)\n"
			"\t-h\tprint this message\n"
			"\t-i\tinfo about inodes\n"
			"\t-m\tuse metric (SI unit)\n"
			"\t-n\tdo not print header\n"
			"\t-o\tshow mount flags\n"
			"\t-s\tsum the total usage\n"
			"\t-t\tfilter filesystems. Read the manpage\n"
			"\t\tfor details\n"
			"\t-T\tshow filesystem type\n"
			"\t-u\tchoose the unit in which\n"
			"\t\tto show the values. Read the manpage\n"
			"\t\tfor details\n"
			"\t-v\tprint program version\n"
			"\t-w\tuse a wider bar\n"
			"\t-W\twide filename (un truncate)\n",
		stdout);
	}

	exit(status);
	/* NOTREACHED */
}

/*
 * fetch information from getmntent and statvfs and store it into the queue
 * @lst: queue in which to store information
 */
void
fetch_info(struct list *lst)
{
	FILE *mtab;
	struct fsmntinfo *fmi;
#ifdef __linux__
	struct mntent *entbuf;
	struct statvfs vfsbuf;
#else
	int nummnt;
	struct statfs *entbuf;
	struct statfs vfsbuf, **fs;
#endif
	/* init fsmntinfo */
	if ((fmi = malloc(sizeof(struct fsmntinfo))) == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	*fmi = fmi_init();
#ifdef __linux__
	/* open mtab file */
	if ((mtab = fopen("/etc/mtab", "r")) == NULL) {
		perror("Error while opening mtab file ");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	/* loop to get infos from all the mounted fs */
	while ((entbuf = getmntent(mtab)) != NULL) {
		/* get infos from statvfs */
		if (statvfs(entbuf->mnt_dir, &vfsbuf) == -1) {
			/* permission denied for this one -> show warning */
			if (errno == EACCES) {
				(void)fprintf(stderr, "WARNING: %s was skipped "
					"because it cannot be stated",
					entbuf->mnt_dir);
				perror(" ");
			} else {
				(void)fprintf(stderr, "Error while stating %s",
					entbuf->mnt_dir);
				perror(" ");
				exit(EXIT_FAILURE);
				/* NOTREACHED */
			}
		} else {
#else
	if ((nummnt = getmntinfo(&entbuf, MNT_WAIT)) <= 0)
		err(EXIT_FAILURE, "Error while getting the list of mountpoints");
		/* NOTREACHED */

	for (fs = &entbuf; nummnt--; (*fs)++) {
		vfsbuf = **fs;
#endif
#ifdef __linux__
			/* infos from getmntent */
			if (Wflag) { /* Wflag to avoid name truncation */
				if ((fmi->fsname = strdup(entbuf->mnt_fsname))
						== NULL) {
					fmi->fsname = "unknown";
				}
				if ((fmi->dir = strdup(entbuf->mnt_dir))
						== NULL) {
					fmi->dir = "unknown";
				}
			} else {
				if ((fmi->fsname = strdup(shortenstr(
					entbuf->mnt_fsname,
					STRMAXLEN))) == NULL) {
				fmi->fsname = "unknown";
				}
				if ((fmi->dir = strdup(shortenstr(entbuf->mnt_dir,
							STRMAXLEN))) == NULL) {
					fmi->dir = "unknown";
				}
			}
			if ((fmi->type = strdup(shortenstr(entbuf->mnt_type,
							12))) == NULL) {
				fmi->type = "unknown";
			}
			if ((fmi->opts = strdup(entbuf->mnt_opts)) == NULL) {
				fmi->opts = "none";
			}
#else
			if (Wflag) { /* Wflag to avoid name truncation */
				if ((fmi->fsname = strdup(
						entbuf->f_mntfromname))	== NULL) {
					fmi->fsname = "unknown";
				}
				if ((fmi->dir = strdup((
						entbuf->f_mntonname ))) == NULL) {
					fmi->dir = "unknown";
				}
			} else {
				if ((fmi->fsname = strdup(shortenstr(
							entbuf->f_mntfromname,
							STRMAXLEN))) == NULL) {
					fmi->fsname = "unknown";
				}
				if ((fmi->dir = strdup(shortenstr(
							entbuf->f_mntonname,
							STRMAXLEN))) == NULL) {
					fmi->dir = "unknown";
				}
			}
			if ((fmi->type = strdup(shortenstr(
						entbuf->f_fstypename,
						12))) == NULL) {
				fmi->type = "unknown";
			}
			/* TODO add the options */
			fmi->opts = "non";
#endif
			/* infos from statvfs */
			fmi->bsize	= vfsbuf.f_bsize;
#ifdef __linux__
			fmi->frsize	= vfsbuf.f_frsize;
#else			/* *BSD do not have frsize */
			fmi->frsize	= 0;
#endif
			fmi->blocks	= vfsbuf.f_blocks;
			fmi->bfree	= vfsbuf.f_bfree;
			fmi->bavail	= vfsbuf.f_bavail;
			fmi->files	= vfsbuf.f_files;
			fmi->ffree	= vfsbuf.f_ffree;
#ifdef __linux__
			fmi->favail	= vfsbuf.f_favail;
#else			/* *BSD do not have favail */
			fmi->favail	= 0;
#endif
			/* pointer to the next element */
			fmi->next = NULL;

			/* enqueue the element into the queue */
			enqueue(lst, *fmi);

			/* adjust longest for the queue */
			if ((!aflag && fmi->blocks > 0) || aflag) {
				lst->fsmaxlen = imax((int)strlen(fmi->fsname),
					lst->fsmaxlen);
				lst->dirmaxlen = imax((int)strlen(fmi->dir),
						lst->dirmaxlen);
				lst->typemaxlen = imax((int)strlen(fmi->type),
						lst->typemaxlen);
			}
		}
#ifdef __linux__
	}
	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");
#endif
}

/*
 * Converts the argument to the correct unit
 * TODO: pretty crapy function... should do it in a smart way!
 * Plus there probably is some roundings errors...
 * Need to clean this crap ASAP
 * @n: number to convert
 */
double
cvrt(double n)
{
	switch (unitflag) {
	case 'b':
		return n;
		/* NOTREACHED */
	case 'e':
		if (mflag) /* 1000^6 */
			return n / 1000000000000000000.0;
			/* NOTREACHED */
		else /* 1024^6 */
			return n / 1152921504606846976.0;
			/* NOTREACHED */
	case 'g':
		if (mflag) /* 1000^3 */
			return n / 1000000000.0;
			/* NOTREACHED */
		else /* 1024^3 */
			return n / 1073741824.0;
			/* NOTREACHED */
	case 'k':
		if (mflag)
			return n / 1000.0;
			/* NOTREACHED */
		else
			return n / 1024.0;
			/* NOTREACHED */
	case 'm':
		if (mflag) /* 1000^2 */
			return n / 1000000.0;
			/* NOTREACHED */
		else /* 1024^2 */
			return n / 1048576.0;
			/* NOTREACHED */
	case 'p':
		if (mflag) /* 1000^5 */
			return n / 1000000000000000.0;
			/* NOTREACHED */
		else /* 1024^5 */
			return n / 1125899906842624.0;
			/* NOTREACHED */
	case 't':
		if (mflag) /* 1000^4 */
			return n / 1000000000000.0;
			/* NOTREACHED */
		else /* 1024^4 */
			return n / 1099511627776.0;
			/* NOTREACHED */
	case 'y':
		if (mflag) /* 1000^8 */
			return n / 1000000000000000000000000.0;
			/* NOTREACHED */
		else /* 1024^8 */
			return n / 1208925819614629174706176.0;
			/* NOTREACHED */
	case 'z':
		if (mflag)
			/* 1000^7 */
			return n / 1000000000000000000000.0;
			/* NOTREACHED */
		else /* 1024^7 */
			return n / 1180591620717411303424.0;
			/* NOTREACHED */
	}

	/* should not be able to reach this point but just in case... */
	return n;
	/* NOTREACHED */
}

/*
 * convert to human readable format and print the result
 * @n: number to convert and print
 * @perct: percentage (useful when using colors)
 */
void
humanize(double n, double perct)
{
	int i = 0;
	double divider = 1024.0;

	/* when using SI unit... */
	if (mflag)
		divider = 1000.0;

	while ((n >= 1000) && (i < 8)) {
		n /= divider;
		i++;
	}

	change_color(perct);

	if ( i == 0)
		(void)printf("%9.f", n);
	else
		(void)printf("%9.1f", n);

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
}
