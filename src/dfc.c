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

#include <locale.h>
#include <libintl.h>

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
	struct Display display;
	int ch;
	int width;
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

	/* translation support */
	if (setlocale(LC_ALL, "") == NULL)
		(void)fprintf(stderr, "Locale cannot be set\n");
	if (bindtextdomain(PACKAGE, LOCALEDIR) == NULL)
		(void)fprintf(stderr, "Cannot bind locale\n");
	if (bind_textdomain_codeset(PACKAGE, "") == NULL)
		(void)fprintf(stderr, "Cannot bind locale codeset\n");
	if (textdomain(PACKAGE) == NULL)
		(void)fprintf(stderr, "Cannot set translation domain\n");

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

	while ((ch = getopt(argc, argv, "abc:e:fhimnop:q:st:Tu:vwW")) != -1) {
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
						_("-c: illegal sub option %s\n"),
						subopts);
					return EXIT_FAILURE;
					/* NOTREACHED */
				}
			}
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
					init_disp_csv(&display);
					break;
				case EHTML:
					/* init_disp_html(&display); */
					break;
				case ETEX:
					/* init_disp_tex(&display); */
					break;
				case -1:
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
				case -1:
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
				case -1:
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
		if (parse_conf(cfgfile) == -1)
			(void)fprintf(stderr, "Error reading the configuration"
					" file: %s\n", cfgfile);
	}

	/* if nothing specified, text output is default */
	if (!eflag)
		init_disp_text(&display);

	/* initializes the queue */
	init_queue(&queue);

	/* fetch information about the currently mounted filesystems */
	fetch_info(&queue);

	/* cannot display all information if tty is too narrow */
	if (!fflag && width > 0 && !eflag)
		auto_adjust(queue, width);

	/* actually displays the info we have got */
	disp(&queue, fstfilter, fsnfilter, &display);

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
		(void)fputs(_("Try dfc -h for more information\n"), stderr);
	else {
		/* 2 fputs because string length limit is 509 */
		(void)fputs(_("Usage: dfc [OPTIONS(S)] [-c WHEN] [-p FSNAME] "
			" [-q SORTBY] [-t FSTYPE] [-u UNIT]\n"
			"Available options:\n"
			"\t-a\tprint all fs from mtab\n"
			"\t-b\tdo not show the graph bar\n"
			"\t-c\tchoose color mode. Read the manpage\n"
			"\t\tfor details\n"
			"\t-f\tdisable auto-adjust mode (force display)\n"
			"\t-h\tprint this message\n"),
			stdout);
		(void)fputs(_(
			"\t-i\tinfo about inodes\n"
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
#else
	if ((nummnt = getmntinfo(&entbuf, MNT_NOWAIT)) <= 0)
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
#ifdef __MACH__
			/* TODO: implement feature for MacOS */
			fmi->opts = _("sorry, not available on MacOS...");
#else
			if ((fmi->opts = statfs_flags_to_str(entbuf)) == NULL) {
				fmi->opts = "none";
			}
#endif /* __MACH__ */
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
				lst->mntoptmaxlen = imax((int)strlen(fmi->opts),
						lst->mntoptmaxlen);
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
 * Actually displays infos in nice manner
 * @lst: queue containing all required information
 * @fstfilter: fstype to filter (can be NULL)
 * @fsnfilter: fsname to filter (can be NULL)
 * @disp: display structure that points to the respective functions regarding
 *	  the selected output type
 */
void
disp(struct list *lst, char *fstfilter, char *fsnfilter, struct Display *disp)
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
		if (fstfilter[0] == '-') {
			nmt = 1;
			fstfilter++;
		}
	}

	/* activate negative matching on fs name? */
	if (pflag) {
		if (fsnfilter[0] == '-') {
			nmn = 1;
			fsnfilter++;
		}
	}

	/* legend on top */
	if (!nflag)
		disp->print_header(lst);

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
				p = p->next;
				continue;
				/*NOTREACHED */
			}
		}

		/* apply filtering on fs type */
		if (fstypefilter(p->type, fstfilter, nmt) == 0) {
			p = p->next;
			continue;
			/* NOTREACHED */
		}
		/* apply filtering on fs name */
		if (fsnamefilter(p->fsname, fsnfilter, nmn) == 0) {
			p = p->next;
			continue;
			/* NOTREACHED */
		}

		/* filesystem */
		disp->print_fs(lst, p->fsname);

		/* type */
		if (Tflag) {
			disp->print_type(lst, p->type);
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
			disp->print_bar(perctused);

		/* %used */
		disp->print_perct(perctused);

		/* format to requested format */
		if (uflag) {
			size = cvrt(size);
			avail = cvrt(avail);
		}

		/* avail  and total */
		disp->print_at(avail, perctused);
		disp->print_at(size, perctused);

		/* info about inodes */
		if (iflag) {
			ifitot += (double)p->files;
			ifatot += (double)p->favail;
			disp->print_inodes(p->files / 1000, p->files / 1000);
		}

		/* mounted on */
		disp->print_mount(p->dir);

		/* info about mount option */
		if (oflag)
			disp->print_mopt(lst, p->dir, p->opts);

		(void)printf("\n");

		p = p->next;
	}

	if (sflag)
		disp->print_sum(lst, stot, atot, utot, ifitot, ifatot);
}

/* does not work on Mac OS */
#ifdef __FreeBSD__
/*
 * Turn the f_flags member of the given struct statfs to a human-readable string
 * of the form "opt1,opt2..."
 * Returns NULL if an error occurred.
 * @s: struct statfs * to parse.
 */
char *
statfs_flags_to_str(struct statfs *s)
{
       uint64_t flags = s->f_flags;
       size_t bufsize = 128;
       char *buffer = malloc(bufsize);
       if (!buffer) {
               (void)fprintf(stderr, _("Could not retrieve mount flags for %s\n"),
                       s->f_mntonname);
               return NULL;
	       /* NOTREACHED */
       }
       buffer[0] = '\0';

       /* Comparing flags to all possible flags, in the same order as mount -p */
       if (flags & MNT_RDONLY) {
               if (strlcat(buffer, "ro", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       } else {
               if (strlcat(buffer, "rw", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       }

       if (flags & MNT_SYNCHRONOUS)
               if (strlcat(buffer, ",sync", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOEXEC)
               if (strlcat(buffer, ",noexec", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOSUID)
               if (strlcat(buffer, ",nosuid", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_UNION)
               if (strlcat(buffer, ",union", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_ASYNC)
               if (strlcat(buffer, ",async", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOATIME)
               if (strlcat(buffer, ",noatime", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOCLUSTERR)
               if (strlcat(buffer, ",noclusterr", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOCLUSTERW)
               if (strlcat(buffer, ",noclusterw", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NOSYMFOLLOW)
               if (strlcat(buffer, ",nosymfollow", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_SUIDDIR)
               if (strlcat(buffer, ",suiddir", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_MULTILABEL)
               if (strlcat(buffer, ",multilabel", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_ACLS)
               if (strlcat(buffer, ",acls", bufsize) >= bufsize)
                       goto truncated;
			/* NOTREACHED */
       if (flags & MNT_NFS4ACLS)
               if (strlcat(buffer, ",nfsv4acls", bufsize) >= bufsize)
		       goto truncated;
			/* NOTREACHED */

       return buffer;
       /* NOTREACHED */

truncated:
       (void)fprintf(stderr, _("Truncating mount options for %s\n"),
                       s->f_mntonname);
       return buffer;
       /* NOTREACHED */
}

#endif /* __FreeBSD__ */
