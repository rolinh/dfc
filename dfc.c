/*
 * dfc.c
 *
 * (C) 2012 - Hahling Robin <robin.hahling@gw-computing.net>
 *
 * Displays free disk space in an elegant manner.
 */
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <mntent.h>
#include <string.h>

#include <sys/param.h>
#include <sys/statvfs.h>

#include "dfc.h"

/* set flags for options */
static int aflag, bflag, hflag, iflag, mflag, nflag, oflag, sflag, tflag,
	   uflag, vflag, wflag;
static int cflag = 1; /* color enabled by default */
static int Tflag;

/*
 * Now use -u option for choosing the size (b, k, m, etc.)
 * When using the flag, should specifie the unit used (h is default).
 * Have a look at unit_opts for the possible values.
 */
static char unit = 'h';

int
main(int argc, char *argv[])
{
	struct list queue;
	int ch;
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

	while ((ch = getopt(argc, argv, "abc:himnost:Tu:vw")) != -1) {
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
					unit = 'h';
					break;
				case B:
					unit = 'b';
					break;
				case K:
					unit = 'k';
					break;
				case M:
					unit = 'm';
					break;
				case G:
					unit = 'g';
					break;
				case T:
					unit = 't';
					break;
				case P:
					unit = 'p';
					break;
				case E:
					unit = 'e';
					break;
				case Z:
					unit = 'z';
					break;
				case Y: unit = 'y';
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

	/* if fd is not a terminal and color mode is not "always", disable color */
	if (!isatty(STDOUT_FILENO) && cflag != 2)
		cflag = 0;

	/* initializes the queue */
	init_queue(&queue);

	/* fetch information from getmntent and statvfs */
	fetch_info(&queue);

	/* actually displays the infos we have gotten */
	disp(&queue, fsfilter);

	return EXIT_SUCCESS;
	/* NOTREACHED */
}

/*
 * Display usage.
 * param: status --> status code (EXIT_SUCCESS, EXIT_FAILURE, ...)
 */
static void
usage(int status)
{
	if (status != 0)
		(void)fputs("Try dfc -h for more informations\n", stderr);
	else
		(void)fputs("Usage: dfc [OPTIONS(S)] [-c WHEN] [-u UNIT]"
			"[-t FILESYSTEM]\n"
			"Available options:\n", stdout);
		(void)fputs(
		"	-a	print all fs from mtab\n"
		"	-b	do not show the graph bar\n"
		"	-c	choose color mode. Read the manpage\n"
		"		for details\n"
		"	-h	print this message\n"
		"	-i	info about inodes\n"
		"	-m	use metric (SI unit)\n"
		"	-n	do not print header\n"
		"	-o	show mount flags\n"
		"	-s	sum the total usage\n"
		"	-t	filter filesystems. Read the manpage\n"
		"		for details\n"
		"	-T	show filesystem type\n"
		"	-u	choose the unit in which\n"
		"		to show the values. Read the manpage\n"
		"		for details\n"
		"	-v	print program version\n"
		"	-w	use a wider bar\n",
		stdout);

	exit(status);
	/* NOTREACHED */
}

/*
 * Initializes a queue structure
 * @lst: queue pointer
 */
void
init_queue(struct list *lst)
{
	lst->head = NULL;
	lst->tail = NULL;
	lst->fsmaxlen = -1;
	lst->dirmaxlen = -1;
	lst->typemaxlen = -1;
}

/*
 * Checks if a queue is empty
 * @lst: queue
 * Returns:
 *	--> 1 (true) if the queue is empty
 *	--> 0 if not
 */
int
is_empty(struct list lst)
{
	if (lst.head == NULL)
		return 1;
		/* NOTREACHED */
	else
		return 0;
		/* NOTREACHED */
}

/*
 * Enqueues an element into a queue
 * @lst: queue pointer
 * @elt: element
 * Returns:
 *	--> -1 on error
 *	-->  0 on sucess
 */
int
enqueue(struct list *lst, struct fsmntinfo fmi)
{
	struct fsmntinfo *new_fmi = malloc(sizeof(struct fsmntinfo));

	if (new_fmi == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		return -1;
		/* NOTREACHED */
	}

	/* initialize the new element to be inserted in the queue */
	*new_fmi = fmi;

	if (is_empty((*lst)))
		lst->head = new_fmi;
	else
		lst->tail->next = new_fmi;

	lst->tail = new_fmi;

	return 0;
	/* NOTREACHED */
}

/*
 * Inits an fsmntinfo to some defaults values
 * Returns:
 *	--> fsmntinfo that has been initialized
 */
struct fsmntinfo
fmi_init(void)
{
	struct fsmntinfo fmi;

	fmi.fsname	= "unknown";
	fmi.dir		= "unknown";
	fmi.type	= "unknown";
	fmi.opts	= "none";

	fmi.bsize	= 0;
	fmi.frsize	= 0;
	fmi.blocks	= 0;
	fmi.bfree	= 0;
	fmi.bavail	= 0;
	fmi.files	= 0;
	fmi.ffree	= 0;
	fmi.favail	= 0;

	fmi.next	= NULL;

	return fmi;
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
	struct mntent *entbuf;
	struct statvfs vfsbuf;
	struct fsmntinfo *fmi;

	/* init fsmntinfo */
	if ((fmi = malloc(sizeof(struct fsmntinfo))) == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	*fmi = fmi_init();

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
			/* infos from getmntent */
			if ((fmi->fsname = strdup(trk(entbuf->mnt_fsname)))
					== NULL) {
				fmi->fsname = "unknown";
			}
			if ((fmi->dir = strdup(trk(entbuf->mnt_dir))) == NULL) {
				fmi->dir = "unknown";
			}
			if ((fmi->type = strdup(trk(entbuf->mnt_type))) == NULL) {
				fmi->type = "unknown";
			}
			if ((fmi->opts = strdup(trk(entbuf->mnt_opts))) == NULL) {
				fmi->opts = "none";
			}

			/* infos from statvfs */
			fmi->bsize	= vfsbuf.f_bsize;
			fmi->frsize	= vfsbuf.f_frsize;
			fmi->blocks	= vfsbuf.f_blocks;
			fmi->bfree	= vfsbuf.f_bfree;
			fmi->bavail	= vfsbuf.f_bavail;
			fmi->files	= vfsbuf.f_files;
			fmi->ffree	= vfsbuf.f_ffree;
			fmi->favail	= vfsbuf.f_favail;

			/* pointer to the next element */
			fmi->next = NULL;

			/* enqueue the element into the queue */
			enqueue(lst, *fmi);


			/* adjust for gvfs-fuse-daemon */
			if (strcmp(fmi->fsname, "gvfs-fuse-daemon") == 0) {
				lst->fsmaxlen = imax(lst->fsmaxlen, 11);
				lst->typemaxlen = imax(lst->typemaxlen, 4);
				lst->dirmaxlen = imax((int)strlen(fmi->dir),
						lst->dirmaxlen);
			} else { /* adjust longest for the queue */
				lst->fsmaxlen = imax((int)strlen(fmi->fsname),
					lst->fsmaxlen);
				lst->dirmaxlen = imax((int)strlen(fmi->dir),
						lst->dirmaxlen);
				lst->typemaxlen = imax((int)strlen(fmi->type),
						lst->typemaxlen);
			}
		}
	}

	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF) {
		perror("Could not close mtab file ");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
}

/*
 * Actually displays infos in nice manner
 * @lst: queue containing all required informations
 * @fsfilter: fstype to filter (can be nothing)
 */
void
disp(struct list *lst, char *fsfilter)
{
	struct fsmntinfo *p = NULL;
	int i, n;
	int skip = 1;
	double perctused, size, avail, used;
	double stot, atot, utot, ifitot, ifatot;
	char *stropt;
	char *strtmp = strdup(fsfilter);

	stot = atot = utot = ifitot = ifatot = n = 0;

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
		} else {
			/*
			 * gvfs-fuse-daemon is way too long for a name --> it
			 * breaks the display so rename it
			 */
			if (strcmp(p->fsname, "gvfs-fuse-daemon") == 0) {
				(void)strcpy(p->fsname, "fuse-daemon");
				(void)strcpy(p->type, "gvfs");
			}
		}

		/* apply fsfiltering */
		if (tflag) {
			stropt = strtok(fsfilter, ",");
			while (stropt != NULL) {
				if (strcmp(p->type, stropt) == 0)
					skip = 0;
				stropt = strtok(NULL, ",");
			}
			/* strtok modifies fsfilter so give back its value */
			fsfilter = strdup(strtmp);
			if (skip) {
				p = p->next;
				continue;
				/* NOTREACHED */
			} else
				skip = 1;
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

		size = (double)p->blocks *(double)p->frsize;
		avail = (double)p->bavail * (double)p->frsize;
		used = size - avail;

		/* calculate the % used */
		if (size == 0)
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
			for (i = (int)strlen(p->dir); i < 16; i++)
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
 * @lst: queue containing the informations
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
		(void)printf("TYPE ");
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
		(void)printf(" (*) USED");
		for (i = 0; i < (barinc + 1); i++)
			(void)printf(" ");
		(void)printf("FREE (-) ");
	}

	(void)printf("%%USED");
	if (unit == 'k')
		(void)printf("  ");
	else if (unit == 'b')
		(void)printf("       ");
	else
		(void)printf(" ");

	(void)printf("AVAILABLE");
	if (unit == 'k')
		(void)printf("      ");
	else if (unit == 'm')
		(void)printf("     ");
	else if (unit == 'b')
		(void)printf("           ");
	else
		(void)printf("     ");

	(void)printf("TOTAL");

	if (iflag) {
		(void)printf("   #INODES");
		(void)printf(" AV.INODES");
	}

	(void)printf(" MOUNTED ON ");

	if (oflag)
		(void)printf("     MOUNT OPTIONS\n");
	else
		(void)printf("\n");

	reset_color();
}

/*
 * display the sum (useful when -s option is used
 * @lst: queue containing the informations
 * @stot: size total
 * @utot:
 */
void
disp_sum(struct list *lst, double stot, double atot, double utot,
		double ifitot, double ifatot)
{
	int i,j;
	double ptot = 0;

	if (stot == 0)
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
 * Display the nice usage bar
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
			(void)printf("*");

		for (j = i; j < 100; j += barinc)
			(void)printf("-");
	} else { /* color */

		/* green */
		(void)printf("\033[;32m");
		for (i = 0; (i < 50) && (i < perct); i += barinc)
			(void)printf("*");

		/* yellow */
		(void)printf("\033[;33m");
		for (; (i < 75) && (i < perct); i += barinc)
			(void)printf("*");

		/* red */
		(void)printf("\033[;31m");
		for (; (i < 100) && (i < perct); i += barinc)
			(void)printf("*");

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
	change_color(perct);

	/* available  and total */
	switch (unit) {
	case 'h':
		humanize(n, perct);
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

	switch (unit) {
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
	switch (unit) {
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

	/* should not be able to be here but just in case... */
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

/*
 * Return the longest of the two parameters
 */
int
imax(int a, int b)
{
	return (a > b ? a : b);
	/* NOTREACHED */
}

/*
 * Truncate str to the third occurrence of /
 * Example: str: /dev/disk/by-uuid/3c301e8b-560c-4914-b50d-8a49e713003c
 * It then returns: /dev/disk/by-uuid
 * Returns unmodified str otherwise
 * @str: char* to truncate
 */
char *
trk(char *str)
{
	char *p = str;
	char trunc[MAXPATHLEN];
	int i = 0;
	size_t len;

	/* if no occurrence of /, return the unmodified str */
	if ((p = strchr(str, '/')) == NULL)
		return str;
		/* NOTREACHED */

	while ((i < 3) && (p != NULL)) {
		/* in case there is only in / */
		if ((p = strchr(p + 1, '/')) == NULL)
			return str;
			/* NOTREACHED */
		i++;
	}

	/* p contains the part of str we want to truncate from str */
	len = strlen(str) - strlen(p);
	(void)strncpy(trunc, str, len);
	trunc[len] = '\0';

	str = strdup(trunc);

	return str;
	/* NOTREACHED */
}
