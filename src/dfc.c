/*
 * Copyright (c) 2013, Robin Hahling
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

#define STRMAXLEN 24

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#ifdef __linux__
#include <mntent.h>
#endif /* __linux__ */
#include <string.h>

#include <sys/types.h>
#include <inttypes.h>
#include <sys/param.h>
#include <sys/statvfs.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) \
	|| defined(__APPLE__) || defined(__DragonFly__)
#include <sys/ucred.h>
#include <sys/mount.h>
#endif /* __FreeBSD__ || __NetBSD__ || __OpenBSD__ || __APPLE__ || __DragonFly__ */

#include "dfc.h"

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

int
main(int argc, char *argv[])
{
	struct list queue;
	struct display sdisp;
	int ch;
	int width;
	int ret = EXIT_SUCCESS;
	char *fsnfilter = NULL;
	char *fstfilter = NULL;
	char *subopts;
	char *value;
	char *cfgfile;

	char *color_opts[] = {
		#define CALWAYS	0
			"always",
		#define	CNEVER	1
			"never",
		#define	CAUTO	2
			"auto",
		NULL
	};

	char *export_opts[] = {
		#define ETEXT	0
			"text",
		#define	ECSV	1
			"csv",
		#define	EHTML	2
			"html",
		#define	ETEX	3
			"tex",
		NULL
	};

	char *sort_opts[] = {
		#define SFSNAME	0
			"name",
		#define SFSTYPE	1
			"type",
		#define SFSDIR	2
			"mount",
		NULL
	};

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
					/* NOTREACHED */
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
					/* NOTREACHED */
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
				case -1: /* FALLTHROUGH */
				default:
					(void)fprintf(stderr,
						_("-u: illegal sub option %s\n"),
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
		(void)printf("%s %s\n", PACKAGE, VERSION);
		return EXIT_SUCCESS;
		/* NOTREACHED */
	}

	width = getttywidth();

	/* if fd is not a terminal and color mode is not "always", disable color */
	if (width == 0 && cflag != 2)
		cflag = 0;

	/* change cnf value according to config file, it it exists */
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
	if (!fflag && width > 0 && !eflag)
		auto_adjust(queue, width);

	/* actually displays the info we have got */
	disp(&queue, fstfilter, fsnfilter, &sdisp);

	return ret;
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
 * fetch information from getmntent and statvfs and store it into the queue
 * @lst: queue in which to store information
 */
void
fetch_info(struct list *lst)
{
	struct fsmntinfo *fmi;
#ifdef __linux__
	FILE *mtab;
	struct mntent *entbuf;
	struct statvfs vfsbuf;
#else /* *BSD */
	int nummnt;
#if defined(__NetBSD__)
	struct statvfs *entbuf;
	struct statvfs vfsbuf, **fs;
#else
	struct statfs *entbuf;
	struct statfs vfsbuf, **fs;
#endif
#endif /* __linux__ */
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
				(void)fprintf(stderr, _("WARNING: %s was skipped "
					"because it cannot be stated"),
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
#else /* BSD */
	if ((nummnt = getmntinfo(&entbuf, MNT_NOWAIT)) <= 0)
		err(EXIT_FAILURE, "Error while getting the list of mountpoints");
		/* NOTREACHED */

	for (fs = &entbuf; nummnt--; (*fs)++) {
		vfsbuf = **fs;
#endif /* __linux__ */
#ifdef __linux__
			/* infos from getmntent */
			if (Wflag) { /* Wflag to avoid name truncation */
				if ((fmi->fsname = strdup(entbuf->mnt_fsname))
						== NULL) {
					/* g_unknown_str is def. in extern.h(.in) */
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup(entbuf->mnt_dir))
						== NULL) {
					fmi->dir = g_unknown_str; 
				}
			} else {
				if ((fmi->fsname = strdup(shortenstr(
					entbuf->mnt_fsname,
					STRMAXLEN))) == NULL) {
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup(shortenstr(entbuf->mnt_dir,
							STRMAXLEN))) == NULL) {
					fmi->dir = g_unknown_str;
				}
			}
			if ((fmi->type = strdup(shortenstr(entbuf->mnt_type,
							12))) == NULL) {
				fmi->type = g_unknown_str;
			}
			if ((fmi->opts = strdup(entbuf->mnt_opts)) == NULL) {
				fmi->opts = g_none_str;
			}
#else /* BSD */
			if (Wflag) { /* Wflag to avoid name truncation */
				if ((fmi->fsname = strdup(
						entbuf->f_mntfromname))	== NULL) {
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup((
						entbuf->f_mntonname ))) == NULL) {
					fmi->dir = g_unknown_str;
				}
			} else {
				if ((fmi->fsname = strdup(shortenstr(
							entbuf->f_mntfromname,
							STRMAXLEN))) == NULL) {
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup(shortenstr(
							entbuf->f_mntonname,
							STRMAXLEN))) == NULL) {
					fmi->dir = g_unknown_str;
				}
			}
			if ((fmi->type = strdup(shortenstr(
						entbuf->f_fstypename,
						12))) == NULL) {
				fmi->type = g_unknown_str;
			}
			if ((fmi->opts = statfs_flags_to_str(entbuf)) == NULL) {
				fmi->opts = g_none_str;
			}
#endif /* __linux__ */
			/* infos from statvfs */
			fmi->bsize    = vfsbuf.f_bsize;
#if defined(__linux__) || defined(__NetBSD__)
			fmi->frsize   = vfsbuf.f_frsize;
#else			/* *BSD do not have frsize */
			fmi->frsize   = 0;
#endif /* __linux__ */
			fmi->blocks   = vfsbuf.f_blocks;
			fmi->bfree    = vfsbuf.f_bfree;
			fmi->bavail   = vfsbuf.f_bavail;
			fmi->files    = vfsbuf.f_files;
			fmi->ffree    = vfsbuf.f_ffree;
#if defined(__linux__) || defined(__NetBSD__)
			fmi->favail   = vfsbuf.f_favail;
#else			/* *BSD do not have favail */
			fmi->favail   = 0;
#endif /* __linux__ */
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
				lst->mntoptmaxlen = imax((int)strlen(fmi->opts),
						lst->mntoptmaxlen);
			}
		}
#ifdef __linux__
	}
	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");
#endif /* __linux__ */
	free(fmi);
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
disp(struct list *lst, char *fstfilter, char *fsnfilter, struct display *sdisp)
{
	struct fsmntinfo *p = NULL;
	int n;
	int nmt = 0;
	int nmn = 0;
	double perctused, size, avail, used;
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
		sdisp->print_header(lst);

	if (lst->fsmaxlen < 11)
		lst->fsmaxlen = 11;

	 /* sort the list */
	if (qflag)
		lst->head = msort(lst->head);

	p = lst->head;

	while (p != NULL) {
		if (!aflag) {
			/* skip (pseudo)devices (which have a size of 0 usually) */
			if (p->blocks == 0) {
				p = delete_struct_and_get_next(p);
				continue;
				/*NOTREACHED */
			}
		}

		/* apply filtering on fs type */
		if (fstypefilter(p->type, fstfilter, nmt) == 0) {
			p = delete_struct_and_get_next(p);
			continue;
			/* NOTREACHED */
		}
		/* apply filtering on fs name */
		if (fsnamefilter(p->fsname, fsnfilter, nmn) == 0) {
			p = delete_struct_and_get_next(p);
			continue;
			/* NOTREACHED */
		}

		/* skip remote file systems */
		if (lflag) {
			if (is_remote(p->type)) {
				p = delete_struct_and_get_next(p);
				continue;
				/* NOTREACHED */
			}
		}

		/* filesystem */
		sdisp->print_fs(lst, p->fsname);

		/* type */
		if (Tflag) {
			sdisp->print_type(lst, p->type);
		}

#if defined(__linux__) || defined(__NetBSD__)
		size  = (double)p->frsize * (double)p->blocks;
		avail = (double)p->frsize * (double)p->bavail;
		used  = (double)p->frsize * ((double)p->blocks - (double)p->bfree);
#else /* *BSD */
		size  = (double)p->bsize * (double)p->blocks;
		avail = (double)p->bsize * (double)p->bavail;
		used  = (double)p->bsize * ((double)p->blocks - (double)p->bfree);
#endif /* __linux__ */
		/* calculate the % used */
		if ((int)size == 0)
			perctused = 100.0;
		else
			/*
			 * compute percent based on bfree as it is a given
			 * value and not a computed one like used size
			 */
			perctused = 100.0 -
				((double)p->bavail / (double)p->blocks) * 100.0;

		if (sflag) {
			stot += size;
			atot += avail;
			utot += used;
		}

		if (!bflag)
			sdisp->print_bar(perctused);

		/* %used */
		sdisp->print_perct(perctused);


		/* format to requested format */
		if (uflag) {
			size = cvrt(size);
			avail = cvrt(avail);
			if (dflag)
				used = cvrt(used);
		}

		if (dflag)
			sdisp->print_at(used, perctused);
		/* avail  and total */
		sdisp->print_at(avail, perctused);
		sdisp->print_at(size, perctused);

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
		sdisp->print_mount(p->dir);

		/* info about mount option */
		if (oflag)
			sdisp->print_mopt(lst, p->dir, p->opts);

		(void)printf("\n");

		p = delete_struct_and_get_next(p); /* XXX returns p->next! */
	}

	if (sflag)
		sdisp->print_sum(lst, stot, atot, utot, ifitot, ifatot);

	/* only required for html and tex export (csv and text point to NULL) */
	if (sdisp->deinit)
		sdisp->deinit();
}

#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
	|| defined(__OpenBSD__) || defined(__APPLE__)
/*
 * All the flags found in *BSD and Mac OS X, alphabetically sorted.
 */
struct flag_str {
	long long flag;
	const char *str;
} possible_flags[] = {
#if defined(__FreeBSD__)
	{ MNT_ACLS,               "acls"               },
#endif
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
	|| defined(__OpenBSD__) || defined (__APPLE__)
	{ MNT_ASYNC,              "async"              },
#endif
#if defined(__APPLE__)
	{ MNT_AUTOMOUNTED,        "automounted"        },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	{ MNT_DEFEXPORTED,        "defexported"        },
#endif
#if defined(__APPLE__)
	{ MNT_DEFWRITE,           "defwrite"           },
	{ MNT_DONTBROWSE,         "dontbrowse"         },
	{ MNT_DOVOLFS,            "dovolfs"            },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	{ MNT_EXKERB,              "exkerb"             },
#endif
#if defined(__NetBSD__)
	{ MNT_EXNORESPORT,          "noresport"          },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	{ MNT_EXPORTANON,         "exportanon"         },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) ||  defined(__DragonFly__) \
	|| defined(__APPLE__)
	{ MNT_EXPORTED,           "exported"           },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__)
	{ MNT_EXPUBLIC,           "expublic"            },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	{ MNT_EXRDONLY,           "exrdonly"           },
#endif
#if defined(__FreeBSD__)
	{ MNT_GJOURNAL,           "gjournal"            },
#endif
#if defined(__APPLE__)
	{ MNT_JOURNALED,          "journaled"          },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) \
	|| defined(__OpenBSD__) || defined(__APPLE__)
	{ MNT_LOCAL,              "local"              },
#endif
#if defined(__FreeBSD__) || defined(__APPLE__)
	{ MNT_MULTILABEL,         "multilabel"         },
#endif
#if defined(__FreeBSD__)
	{ MNT_NFS4ACLS,           "nfs4acls"           },
#endif
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
	|| defined(__OpenBSD__) || defined(__APPLE__)
	{ MNT_NOATIME,            "noatime"            },
#endif
#if defined(__FreeBSD__)
	{ MNT_NOCLUSTERR,         "noclusterr"         },
	{ MNT_NOCLUSTERW,         "noclusterw"         },
#endif
#if defined(__NetBSD__)
	{ MNT_NOCOREDUMP,         "nocoredump"         },
#endif
#if defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__) \
	|| defined(__APPLE__)
	{ MNT_NODEV,              "nodev"              },
#endif
#if defined(__NetBSD__)
	{ MNT_NODEVMTIME,         "nodevmtime"         },
#endif
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
	|| defined(__OpenBSD__) || defined(__APPLE__)
	{ MNT_NOEXEC,             "noexec"             },
	{ MNT_NOSUID,             "nosuid"             },
#endif
#if defined(__FreeBSD__)
	{ MNT_NOSYMFOLLOW,        "nosymfollow"        },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) \
	|| defined(__APPLE__)
	{ MNT_QUOTA,              "quota"              },
#endif
	/* MNT_RDONLY is treated separately in statfs_flags_to_str(). */
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) \
	|| defined(__APPLE__)
	{ MNT_ROOTFS,             "rootfs"             },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	{ MNT_SOFTDEP,            "softdep"            },
#endif
#if defined(__FreeBSD__)
	{ MNT_SUIDDIR,            "suiddir"            },
#endif
#if defined(__NetBSD__)
	{ MNT_SYMPERM,            "symperm"            },
#endif
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
	|| defined(__OpenBSD__) || defined(__APPLE__)
	{ MNT_SYNCHRONOUS,        "sync"               },
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
	{ MNT_UNION,              "union"              },
#endif
#if defined(__FreeBSD__)
	{ MNT_USER, "user" },
#endif
#if defined(__APPLE__)
	{ MNT_UNKNOWNPERMISSIONS, "unknownpermissions" },
#endif
};

/*
 * Turn the f_flags member of the given struct statfs to a human-readable string
 * of the form "opt1,opt2..."
 * Returns NULL if an error occurred.
 * @s: struct statfs * to parse.
 */
char *
statfs_flags_to_str(
#ifdef __NetBSD__
struct statvfs *s
#else
struct statfs *s
#endif
)
{
	int i, n_flags;
#if defined(__DragonFly__)
	int flags = s->f_flags;
#elif defined(__NetBSD__)
	unsigned long flags = s->f_flag;
#else
	uint64_t flags = s->f_flags;
#endif /* __DragonFly__ */
	size_t bufsize = 128;
	char *buffer = malloc(bufsize);
	if (!buffer) {
		(void)fprintf(stderr,
				_("Could not retrieve mount flags for %s\n"),
				s->f_mntonname);
		return NULL;
		/* NOTREACHED */
	}
	buffer[0] = '\0';

	/* There is no MNT_RDWRITE flag, so we have to do this. */
	if (flags & MNT_RDONLY) {
		if (strlcat(buffer, "ro", bufsize) >= bufsize)
			goto truncated;
			/* NOTREACHED */
	} else {
		if (strlcat(buffer, "rw", bufsize) >= bufsize)
			goto truncated;
			/* NOTREACHED */
	}

	/* Comparing flags to all possible flags. */
	n_flags = sizeof(possible_flags) / sizeof(possible_flags[0]);
	for (i = 0; i < n_flags; i++) {
		if (!(flags & possible_flags[i].flag))
			continue;
			/* NOTREACHED */
		if (strlcat(buffer, ",", bufsize) >= bufsize)
			goto truncated;
			/* NOTREACHED */
		if (strlcat(buffer, possible_flags[i].str, bufsize) >= bufsize)
			goto truncated;
			/* NOTREACHED */
	}

	return buffer;
	/* NOTREACHED */

truncated:
       (void)fprintf(stderr, _("Truncating mount options for %s\n"),
			s->f_mntonname);
	return buffer;
	/* NOTREACHED */
}
#endif /* __FreeBSD__ || __OpenBSD__ || __APPLE__ || __DragonFly__ */
