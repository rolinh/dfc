/*
 * Copyright (c) 2012-2015, Robin Hahling
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
 * dfc.c
 *
 * Displays free disk space in an elegant manner.
 */

#define _BSD_SOURCE

#ifdef __linux
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 500
#endif /* __linux__ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfc.h"

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

/* global variables definition, see declaration in extern.h */
char g_unknown_str[] = "unknown";
char g_none_str[]    = "none";
struct conf cnf;
struct maxwidths max;
int aflag, bflag, cflag, dflag, eflag, fflag, hflag, iflag, lflag, mflag,
    nflag, oflag, pflag, qflag, sflag, tflag, uflag, vflag, wflag;
int Tflag, Wflag;
char unitflag;

int
main(int argc, char *argv[])
{
	struct list queue;
	struct display sdisp;
	int ch;
	int tty_width;
	int ret = EXIT_SUCCESS;
	char *fsnfilter = NULL;
	char *fstfilter = NULL;
	char *subopts;
	char *value;
	char *cfgfile;

	/* enum for suboptions flags; first letter corresponds to option flag */
	enum {
		CALWAYS = 0,
		CNEVER = 1,
		CAUTO = 2,
		ETEXT = 0,
		ECSV = 1,
		EHTML = 2,
		ETEX = 3,
		SFSNAME = 0,
		SFSTYPE = 1,
		SFSDIR = 2,
		UH = 0,
		UB = 1,
		UK = 2,
		UM = 3,
		UG = 4,
		UT = 5,
		UP = 6,
		UE = 7,
		UZ = 8,
		UY = 9
	};

	static char always_str[] = "always";
	static char never_str[] = "never";
	static char auto_str[] = "auto";
	char *const color_opts[] = {
		always_str,
		never_str,
		auto_str,
		NULL
	};

	static char text_str[] = "text";
	static char csv_str[] = "csv";
	static char html_str[] = "html";
	static char tex_str[] = "tex";
	char *const export_opts[] = {
		text_str,
		csv_str,
		html_str,
		tex_str,
		NULL
	};

	static char name_str[] = "name";
	static char type_str[] = "type";
	static char mount_str[] = "mount";
	char *const sort_opts[] = {
		name_str,
		type_str,
		mount_str,
		NULL
	};

	static char h_str[] = "h";
	static char b_str[] = "b";
	static char k_str[] = "k";
	static char m_str[] = "m";
	static char g_str[] = "g";
	static char t_str[] = "t";
	static char p_str[] = "p";
	static char e_str[] = "e";
	static char z_str[] = "z";
	static char y_str[] = "y";
	char *const unit_opts[] = {
		h_str,
		b_str,
		k_str,
		m_str,
		g_str,
		t_str,
		p_str,
		e_str,
		z_str,
		y_str,
		NULL
	};

#ifdef NLS_ENABLED
	/* translation support */
	if (setlocale(LC_ALL, "") == NULL) {
		(void)fputs("Locale cannot be set\n", stderr);
		ret = EXIT_FAILURE;
	}
	if (bindtextdomain(PACKAGE, LOCALEDIR) == NULL) {
		(void)fputs("Cannot bind locale\n", stderr);
		ret = EXIT_FAILURE;
	}
	if (bind_textdomain_codeset(PACKAGE, "") == NULL) {
		(void)fputs("Cannot bind locale codeset\n", stderr);
		ret = EXIT_FAILURE;
	}
	if (textdomain(PACKAGE) == NULL) {
		(void)fputs("Cannot set translation domain\n", stderr);
		ret = EXIT_FAILURE;
	}
#endif /* NLS_ENABLED */

	/* default value for those globals */
	cflag = 1; /* color enabled by default */

	/*
	 * Now use -u option for choosing the size (b, k, m, etc.)
	 * When using the flag, should specifie the unit used (h is default).
	 * Have a look at unit_opts for the possible values.
	 */
	unitflag = 'h';

	 /* Init default colors and symbol sign */
	init_conf(&cnf);

	while ((ch = getopt(argc, argv, "abc:de:fhilmnop:q:st:Tu:vwW")) != -1) {
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
				case -1: /* FALLTHROUGH */
				default:
					(void)fprintf(stderr,
						_("-c: illegal sub option %s\n"),
						subopts);
					return EXIT_FAILURE;
				}
			}
			break;
		case 'd':
			dflag = 1;
			break;
		case 'e':
			eflag = 1;
			subopts = optarg;
			while (*subopts) {
				switch (getsubopt(&subopts, export_opts, &value)) {
				case ETEXT:
					eflag = 0;
					break;
				case ECSV:
					Wflag = 1;
					init_disp_csv(&sdisp);
					break;
				case EHTML:
					Wflag = 1;
					init_disp_html(&sdisp);
					break;
				case ETEX:
					Wflag = 1;
					init_disp_tex(&sdisp);
					break;
				case -1: /* FALLTHROUGH */
				default:
					(void)fprintf(stderr,
						_("-e: illegal sub option %s\n"),
						subopts);
					return EXIT_FAILURE;
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
		case 'l':
			lflag = 1;
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
		case 'p':
			pflag = 1;
			fsnfilter = strdup(optarg);
			break;
		case 'q':
			subopts = optarg;
			while (*subopts) {
				switch (getsubopt(&subopts, sort_opts, &value)) {
				case SFSNAME:
					qflag = 1;
					break;
				case SFSTYPE:
					qflag = 2;
					break;
				case SFSDIR:
					qflag = 3;
					break;
				case -1: /* FALLTHROUGH */
				default:
					(void)fprintf(stderr,
						_("-q: illegal sub option %s\n"),
						subopts);
					return EXIT_FAILURE;
				}
			}
			break;
		case 's':
			sflag = 1;
			break;
		case 't':
			tflag = 1;
			fstfilter = strdup(optarg);
			break;
		case 'T':
			Tflag = 1;
			break;
		case 'u':
			uflag = 1;
			subopts = optarg;
			while (*subopts) {
				switch (getsubopt(&subopts, unit_opts, &value)) {
				case UH:
					/*
					 * disable uflag since conversion to
					 * human readable format is computed
					 * very differently from other formats
					 */
					uflag = 0;
					unitflag = 'h';
					break;
				case UB:
					unitflag = 'b';
					break;
				case UK:
					unitflag = 'k';
					break;
				case UM:
					unitflag = 'm';
					break;
				case UG:
					unitflag = 'g';
					break;
				case UT:
					unitflag = 't';
					break;
				case UP:
					unitflag = 'p';
					break;
				case UE:
					unitflag = 'e';
					break;
				case UZ:
					unitflag = 'z';
					break;
				case UY:
					unitflag = 'y';
					break;
				case -1: /* FALLTHROUGH */
				default:
					(void)fprintf(stderr,
						_("-u: illegal sub option %s\n"),
						subopts);
					return EXIT_FAILURE;
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
		}
	}

	if (hflag)
		usage(EXIT_SUCCESS);

	if (vflag) {
		(void)printf("%s %s\n", PACKAGE, VERSION);
		return EXIT_SUCCESS;
	}

	/* init default max required width */
	init_maxwidths();

	tty_width = getttywidth();

	/* if fd is not a terminal and color mode is not "always", disable color */
	if (tty_width == 0 && cflag != 2)
		cflag = 0;

	/* change cnf value according to config file, it exists */
	if ((cfgfile = getconf()) != NULL) {
		if (parse_conf(cfgfile) == -1) {
			(void)fprintf(stderr, _("Error reading the configuration"
					" file: %s\n"), cfgfile);
			ret = EXIT_FAILURE;
		}
	}

	/* if nothing specified, text output is default */
	if (!eflag)
		init_disp_text(&sdisp);

	/* initializes the queue */
	init_queue(&queue);

	/* fetch information about the currently mounted filesystems */
	fetch_info(&queue);

	/* cannot display all information if tty is too narrow */
	if (!fflag && tty_width > 0 && !eflag)
		auto_adjust(tty_width);

	/* actually displays the info we have got */
	disp(&queue, fstfilter, fsnfilter, &sdisp);

	return ret;
}

/*
 * Display usage.
 * param: status --> status code (EXIT_SUCCESS, EXIT_FAILURE, ...)
 */
void
usage(int status)
{
	if (status != 0)
		(void)fputs(_("Try dfc -h for more information\n"), stderr);
	else {
		/* 2 fputs because string length limit is 509 */
		(void)fputs(_("Usage:  dfc [OPTION(S)] [-c WHEN] [-e FORMAT] "
					"[-p FSNAME] [-q SORTBY] [-t FSTYPE]\n"
					"\t[-u UNIT]\n"
			"Available options:\n"
			"\t-a\tprint all mounted filesystem\n"
			"\t-b\tdo not show the graph bar\n"
			"\t-c\tchoose color mode. Read the manpage\n"
			"\t\tfor details\n"
			"\t-d\tshow used size\n"
			"\t-e\texport to specified format. Read the manpage\n"
			"\t\tfor details\n"
			"\t-f\tdisable auto-adjust mode (force display)\n"
			"\t-h\tprint this message\n"
			"\t-i\tinfo about inodes\n"
			"\t-l\tonly show information about locally mounted\n"
			"\t\tfile systems\n"),
			stdout);
		(void)fputs(_(
			"\t-m\tuse metric (SI unit)\n"
			"\t-n\tdo not print header\n"
			"\t-o\tshow mount flags\n"
			"\t-p\tfilter by file system name. Read the manpage\n"
			"\t\tfor details\n"
			"\t-q\tsort the output. Read the manpage\n"
			"\t\tfor details\n"
			"\t-s\tsum the total usage\n"
			"\t-t\tfilter by file system type. Read the manpage\n"
			"\t\tfor details\n"
			"\t-T\tshow filesystem type\n"
			"\t-u\tchoose the unit in which\n"
			"\t\tto show the values. Read the manpage\n"
			"\t\tfor details\n"
			"\t-v\tprint program version\n"
			"\t-w\tuse a wider bar\n"
			"\t-W\twide filename (un truncate)\n"),
		stdout);
	}

	exit(status);
	/* NOTREACHED */
}

/*
 * Actually displays infos in nice manner
 * @lst: queue containing all required information
 * @fstfilter: fstype to filter (can be NULL)
 * @fsnfilter: fsname to filter (can be NULL)
 * @sdisp: display structure that points to the respective functions regarding
 *	  the selected output type
 */
void
disp(struct list *lst, const char *fstfilter, const char *fsnfilter,
    struct display *sdisp)
{
	struct fsmntinfo *p = NULL;
	int n;
	int nmt = 0;
	int nmn = 0;
	double stot, atot, utot, ifitot, ifatot;

	stot = atot = utot = ifitot = ifatot = n = 0;

	/* activate negative matching on fs type? */
	if (tflag) {
		if (fstfilter && fstfilter[0] == '-') {
			nmt = 1;
			fstfilter++;
		}
	}

	/* activate negative matching on fs name? */
	if (pflag) {
		if (fsnfilter && fsnfilter[0] == '-') {
			nmn = 1;
			fsnfilter++;
		}
	}

	/* only required for html and tex export (csv and text point to NULL) */
	if (sdisp->init)
		sdisp->init();

	/* legend on top */
	if (!nflag)
		sdisp->print_header();

	 /* sort the list */
	if (qflag)
		lst->head = msort(lst->head);

	p = lst->head;

	while (p != NULL) {
		/* ignore when needed */
		if (!aflag && (is_mnt_ignore(p) == 1)) {
			p = delete_struct_and_get_next(p);
			continue;
		}

		/* filtering on fs type */
		if (tflag && (fsfilter(p->fstype, fstfilter, nmt) == 0)) {
			p = delete_struct_and_get_next(p);
			continue;
		}
		/* filtering on fs name */
		if (pflag && (fsfilter(p->fsname, fsnfilter, nmn) == 0)) {
			p = delete_struct_and_get_next(p);
			continue;
		}

		/* skip remote file systems */
		if (lflag && is_remote(p)) {
			p = delete_struct_and_get_next(p);
			continue;
		}

		/* filesystem */
		sdisp->print_fs(p->fsname);

		/* type */
		if (Tflag) {
			sdisp->print_type(p->fstype);
		}

		if (sflag) {
			stot += p->total;
			atot += p->avail;
			utot += p->used;
		}

		if (!bflag)
			sdisp->print_bar(p->perctused);

		/* %used */
		sdisp->print_perct(p->perctused);


		/* format to requested format */
		if (uflag) {
			p->total = cvrt(p->total);
			p->avail = cvrt(p->avail);
			if (dflag)
				p->used = cvrt(p->used);
		}

		if (dflag)
			sdisp->print_uat(p->used, p->perctused, max.used);
		sdisp->print_uat(p->avail, p->perctused, max.avail);
		sdisp->print_uat(p->total, p->perctused, max.total);

		/* info about inodes */
		if (iflag) {
			ifitot += (double)p->files;
			ifatot += (double)p->favail;
#if defined(__linux__)
			sdisp->print_inodes((uint64_t)(p->files),
					(uint64_t)(p->favail));
#else
			sdisp->print_inodes((uint64_t)(p->files),
					(uint64_t)( p->ffree));
#endif /* __linux__ */
		}

		/* mounted on */
		sdisp->print_mount(p->mntdir);

		/* info about mount option */
		if (oflag)
			sdisp->print_mopt(p->mntopts);

		(void)printf("\n");

		p = delete_struct_and_get_next(p);
	}

	if (sflag)
		sdisp->print_sum(stot, atot, utot, ifitot, ifatot);

	/* only required for html and tex export (csv and text point to NULL) */
	if (sdisp->deinit)
		sdisp->deinit();
}
