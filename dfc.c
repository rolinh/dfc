/*
 * dfc.c
 *
 * (C) 2012 - Hahling Robin <robin.hahling@gw-computing.net>
 *
 * Displays free disk space in an elegant manner.
 */
#define _POSIX_C_SOURCE 2
#define _BSD_SOURCE

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
int aflag, bflag, gflag, hflag, iflag, kflag, mflag, nflag, sflag, tflag, vflag, wflag;
int Kflag, Mflag, Gflag;

int
main(int argc, char *argv[])
{
	struct list queue;
	int ch;

	while ((ch = getopt(argc, argv, "abghiGkKmMnstvw")) != -1) {
		switch (ch) {
		case 'a':
			aflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		case 'g':
			gflag = 1;
			Gflag = 0;
			break;
		case 'G':
			gflag = 0;
			Gflag = 1;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'k':
			kflag = 1;
			Kflag = 0;
			break;
		case 'K':
			kflag = 0;
			Kflag = 1;
			break;
		case 'm':
			mflag = 1;
			Mflag = 0;
			break;
		case 'M':
			mflag = 0;
			Mflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 't':
			tflag = 1;
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

	/* initializes the queue */
	init_queue(&queue);

	/* fetch information from getmntent and statvfs */
	fetch_info(&queue);

	/* actually displays the infos we have gotten */
	disp(&queue);

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
		(void)fputs("Usage: dfc [OPTIONS(S)]\n"
		"Available options:\n"
		"	-a	print all fs from mtab\n"
		"	-b	size in Bytes\n"
		"	-g	size in Gio\n"
		"	-G	size in Go\n"
		"	-h	print this message\n"
		"	-i	info about inodes\n"
		"	-k	size in Kio\n"
		"	-K	size in Ko\n"
		"	-m	size in Mio\n"
		"	-M	size in Mo\n"
		"	-n	do not print header\n"
		"	-s	sum the total usage\n"
		"	-t	hide filesystem type\n"
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
			(void)fprintf(stderr, "Error using statvfs on %s\n",
					entbuf->mnt_dir);
			perror("with this error code ");
			exit(EXIT_FAILURE);
			/* NOTREACHED */
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

			/* skip fuse-daemon */
			if (strcmp(fmi->fsname, "gvfs-fuse-daemon") == 0)
					continue;

			/* adjust longest for the queue */
			if (aflag) {
				/* is it the longest fsname? */
				lst->fsmaxlen = imax((int)strlen(fmi->fsname),
						lst->fsmaxlen);
				/* is it the longest dir */
				lst->dirmaxlen = imax((int)strlen(fmi->dir),
						lst->dirmaxlen);
				/* is it the longest type? */
				lst->typemaxlen = imax((int)strlen(fmi->type),
						lst->typemaxlen);
			} else {
				/* we do not care about stuff not from /dev/ */
				if (strncmp(fmi->fsname, "/dev/", 5) == 0) {
					/* is it the longest fsname? */
					lst->fsmaxlen = imax(
							(int)strlen(fmi->fsname),
							lst->fsmaxlen);
					/* is it the longest dir */
					lst->dirmaxlen = imax(
							(int)strlen(fmi->dir),
							lst->dirmaxlen);
					/* is it the longest type? */
					lst->typemaxlen = imax(
							(int)strlen(fmi->type),
							lst->typemaxlen);
				}
			}
		}
	}

	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");
}

/*
 * Actually displays infos in nice manner
 * @lst: queue containing all required informations
 */
void
disp(struct list *lst)
{
	struct fsmntinfo *p = NULL;
	int i, j, n;
	int barinc = 5;
	double perctused, size, avail, used;
	double stot, atot, utot;

	stot = atot = utot = n = 0;

	/* legend on top */
	if (!nflag)
		disp_header(lst);

	if (lst->fsmaxlen < 11)
		lst->fsmaxlen = 11;

	p = lst->head;
	while (p != NULL) {

		/* we do not care about gvfs-fuse-daemon and the others */
		if (strcmp(p->fsname, "gvfs-fuse-daemon") == 0 ||
			strcmp(p->fsname, "proc") == 0 ||
			strcmp(p->fsname, "sys") == 0 ||
			strcmp(p->fsname, "devpts") == 0) {
			p = p->next;
			continue;
			/* NOTREACHED */
		}

		if (!aflag) {
			/* skip some stuff we do not care about */
			if (strncmp(p->fsname, "/dev/", 5) != 0) {
				p = p->next;
				continue;
				/* NOTREACHED */
			}
		}

		/* filesystem */
		(void)printf("%s", p->fsname);
		for (i = (int)strlen(p->fsname); i < lst->fsmaxlen + 1; i++)
			(void)printf(" ");

		/* type */
		if (!tflag) {
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

		/* option to display a wider bar */
		if (wflag) {
			barinc = 2;
		}
		/* used (*) */
		(void)printf("[");
		for (i = 0; i < perctused; i += barinc)
			(void)printf("*");

		for (j = i; j < 100; j += barinc)
			(void)printf("-");

		/* %used */
		(void)printf("]  %3.f%%", perctused);

		/* format to requested format (k,m,g) */
		if (bflag || kflag || Kflag || mflag || Mflag || gflag || Gflag) {
			size = cvrt(size);
			avail = cvrt(avail);
		}

		/* avail  and total */
		if (kflag || Kflag) {
			(void)printf("%10.fK", avail);
			(void)printf("%10.fK", size);
		} else if (mflag || Mflag) {
			(void)printf("%9.1fM", avail);
			(void)printf("%9.1fM", size);
		} else if (gflag || Gflag) {
			(void)printf("%9.1fG", avail);
			(void)printf("%9.1fG", size);
		} else if (bflag) {
			(void)printf("%15.fB", avail);
			(void)printf("%15.fB", size);
		} else { /* human readable */
			humanize(avail);
			humanize(size);
		}

		/* info about inodes */
		if (iflag) {
			(void)printf("%10ld", p->files);
			(void)printf("%10ld", p->favail);
		}

		/* mounted on */
		(void)printf(" %s\n", p->dir);

		p = p->next;
	}

	if (sflag)
		disp_sum(lst, stot, atot, utot);
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

	(void)printf("FILESYSTEM  ");
	for (i = 11; i < lst->fsmaxlen; i++)
		(void)printf(" ");

	if (!tflag) {
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
	(void)printf("(*) USED");
	for (i = 0; i < (barinc + 1); i++)
		(void)printf(" ");
	(void)printf("FREE (-) ");

	(void)printf("%%USED");
	if (kflag || Kflag)
		(void)printf("  ");
	else if (bflag)
		(void)printf("       ");
	else
		(void)printf(" ");

	(void)printf("AVAILABLE");
	if (kflag || Kflag)
		(void)printf("      ");
	else if (mflag || Mflag)
		(void)printf("     ");
	else if (bflag)
		(void)printf("           ");
	else
		(void)printf("     ");

	(void)printf("TOTAL");

	if (iflag) {
		(void)printf("   #INODES");
		(void)printf(" AV.INODES");
	}

	(void)puts(" MOUNTED ON");
}

/*
 * display the sum (useful when -s option is used
 * @lst: queue containing the informations
 * @stot: size total
 * @utot:
 */
void
disp_sum(struct list *lst, double stot, double atot, double utot)
{
	int i,j;
	int barinc = 5;
	double ptot = 0;

	if (stot == 0)
		ptot = 100.0;
	else
		ptot = (utot / stot) * 100.0;
	(void)printf("SUM:");

	j = lst->fsmaxlen + 1;
	if (!tflag)
		j += lst->typemaxlen + 1;
	for (i = 4; i < j; i++)
		(void)printf(" ");

	/* option to display a wider bar */
	if (wflag)
		barinc = 2;

	(void)printf("[");
	for (i = 0; i < ptot ; i += barinc)
		(void)printf("*");

	for (j = i; j < 100; j += barinc)
		(void)printf("-");

	(void)printf("]  %3.f%%", ptot);

	if (bflag || kflag || Kflag || mflag || Mflag || gflag || Gflag) {
		stot = cvrt(stot);
		atot = cvrt(atot);
	}

	/* free  and total */
	if (kflag || Kflag) {
		(void)printf("%10.fK", atot);
		(void)printf("%10.fK", stot);
	} else if (mflag || Mflag) {
		(void)printf("%9.1fM", atot);
		(void)printf("%9.1fM", stot);
	} else if (gflag || Gflag) {
		(void)printf("%9.1fG", atot);
		(void)printf("%9.1fG", stot);
	} else if (bflag) {
		(void)printf("%15.fB", atot);
		(void)printf("%15.fB", stot);
	} else {/* human readable */
		humanize(atot);
		humanize(stot);
	}
	(void)printf("\n");
}

/*
 * Converts the argument to the correct format (K,M,G)
 * @n: number to convert
 */
double
cvrt(double n)
{
	if (kflag)
		n /= 1024.0;
	else if (mflag)
		/* 1024^2 */
		n /= 1048576.0;
	else if (gflag)
		/* 1024^3 */
		n /= 1073741824.0;
	else if (Kflag)
		n /= 1000.0;
	else if (Mflag)
		n /= 1000000.0;
	else if (Gflag)
		n /= 1000000000.0;

	return n;
	/* NOTREACHED */
}

/*
 * convert to human readable format and print it
 * @n: number to convert and print
 */
double
humanize(double n)
{
	int i = 0;

	while ((n >= 1000) && (i < 3)) {
		n /= 1024.0;
		i++;
	}

	switch (i) {
	case 0:	/* bytes */
		(void)printf("%15.fB", n);
		break;
	case 1: /* Kio */
		(void)printf("%10.fK", n);
		break;
	case 2: /* Mio */
		(void)printf("%9.1fM", n);
		break;
	case 3: /* Gio */
		(void)printf("%9.1fG", n);
		break;
	default:/* else ??? */
		(void)fprintf(stderr, "Error while humanizing %f\n", n);
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
	char trunc[strlen(str)];
	int i = 0;
	int len;

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
