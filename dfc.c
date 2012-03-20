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
int aflag, hflag, gflag, kflag, mflag, vflag, wflag;

int
main(int argc, char *argv[])
{
	FILE *mtab;
	struct mntent *entbuf;
	struct statvfs vfsbuf;
	struct fsmntinfo *fmi;
	struct list queue;
	int ch;

	while ((ch = getopt(argc, argv, "ahgkmvw")) != -1) {
		switch (ch) {
		case 'a':
			aflag = 1;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'g':
			gflag = 1;
			break;
		case 'k':
			kflag = 1;
			break;
		case 'm':
			mflag = 1;
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

	/* init fsmntinfo */
	if ((fmi = malloc(sizeof(struct fsmntinfo))) == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		return EXIT_FAILURE;
		/* NOTREACHED */
	}
	*fmi = fmi_init();

	/* open mtab file */
	if ((mtab = fopen("/etc/mtab", "r")) == NULL) {
		perror("Error while opening mtab file ");
		return EXIT_FAILURE;
		/* NOTREACHED */
	}

	/* loop to get infos from all the mounted fs */
	while ((entbuf = getmntent(mtab)) != NULL) {
		/* get infos from statvfs */
		if (statvfs(entbuf->mnt_dir, &vfsbuf) == -1) {
			(void)fprintf(stderr, "Error using statvfs on %s\n",
					entbuf->mnt_dir);
			perror("with this error code ");
			return EXIT_FAILURE;
			/* NOTREACHED */
		} else {
			/* infos from getmntent */
			if ((fmi->fsname = strdup(entbuf->mnt_fsname)) == NULL) {
				fmi->fsname = "unknown";
			}
			if ((fmi->dir = strdup(entbuf->mnt_dir)) == NULL) {
				fmi->dir = "unknown";
			}
			if ((fmi->type = strdup(entbuf->mnt_type)) == NULL) {
				fmi->type = "unknown";
			}

			/* infos from statvfs */
			fmi->bsize = vfsbuf.f_frsize;
			fmi->blocks = vfsbuf.f_blocks;
			fmi->bfree = vfsbuf.f_bavail;

			/* pointer to the next element */
			fmi->next = NULL;

			/* enqueue the element into the queue */
			enqueue(&queue, *fmi);

			/* skip fuse-daemon */
			if (strcmp(fmi->fsname, "gvfs-fuse-daemon") == 0)
					continue;

			/* adjust longest for the queue */
			if (aflag) {
				/* is it the longest type? */
				queue.fsmaxlen = imax((int)strlen(fmi->type),
						queue.typemaxlen);
				/* is it the longest dir */
				queue.dirmaxlen = imax((int)strlen(fmi->dir),
						queue.dirmaxlen);
				/* is it the longest fsname? */
				queue.typemaxlen = imax((int)strlen(fmi->fsname),
						queue.fsmaxlen);
			} else {
				/* we do not care about stuff not from /dev/ */
				if (strncmp(fmi->fsname, "/dev/", 5) == 0) {
					/* is it the longest type? */
					queue.fsmaxlen = imax(
							(int)strlen(fmi->type),
							queue.typemaxlen);
					/* is it the longest dir */
					queue.dirmaxlen = imax(
							(int)strlen(fmi->dir),
							queue.dirmaxlen);
					/* is it the longest fsname? */
					queue.typemaxlen = imax(
							(int)strlen(fmi->fsname),
							queue.fsmaxlen);
				}
			}
		}
	}

	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");

	/* actually displays the infos we have gotten */
	disp(queue);

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
		"	-h	print this message\n"
		"	-g	size in Gio\n"
		"	-k	size in Kio\n"
		"	-m	size in Mio\n"
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
	fmi.bsize	= 0;
	fmi.blocks	= 0;
	fmi.bfree	= 0;
	fmi.next	= NULL;

	return fmi;
	/* NOTREACHED */
}

/*
 * Actually displays infos in nice manner
 * @lst: queue containing all required informations
 */
void
disp(struct list lst)
{
	struct fsmntinfo *p = NULL;
	int i, j;
	int barinc = 5;
	double perctused, size, free, used;

	/* legend on top */
	disp_header(&lst);

	p = lst.head;
	while (p != NULL) {

		/* we do not care about gvfs-fuse-daemon */
		if (strcmp(p->fsname, "gvfs-fuse-daemon") == 0){
			p = p->next;
			continue;
		}

		if (!aflag) {
			/* skip some stuff we do not care about */
			if (strncmp(p->fsname, "/dev/", 5) != 0) {
				p = p->next;
				continue;
			}
		}

		/* filesystem */
		(void)printf("%s", p->fsname);
		for (i = (int)strlen(p->fsname); i < lst.fsmaxlen; i++)
			(void)printf(" ");

		/* type */
		(void)printf("%s", p->type);
		for (i = (int)strlen(p->type); i < lst.typemaxlen; i++)
			(void)printf(" ");


		size = (double)p->blocks *(double)p->bsize;
		free = (double)p->bfree * (double)p->bsize;

		/* calculate the % used */
		used = size - free;
		if (size == 0)
			perctused = 100.0;
		else
			perctused = (used / size) * 100.0;

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
		size = cvrt(size);
		free = cvrt(free);

		/* free  and total */
		if (kflag) {
			(void)printf("%10.fK", free);
			(void)printf("%10.fK", size);
		} else if (mflag) {
			(void)printf("%9.1fM", free);
			(void)printf("%9.1fM", size);
		} else if (gflag) {
			(void)printf("%9.1fG", free);
			(void)printf("%7.1fG", size);
		} else {
			(void)printf("%15.fB", free);
			(void)printf("%15.fB", size);
		}

		/* mounted on */
		(void)printf(" %s\n", p->dir);

		/* reinit the length */
		p = p->next;
	}
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

	(void)printf("FILESYSTEM ");
	if (lst->fsmaxlen > 11)
		for (i = 11; i < lst->fsmaxlen; i++)
			(void)printf(" ");
	else
		lst->fsmaxlen = 11;

	(void)printf("TYPE ");
	if (lst->typemaxlen > 5)
		for (i = 5; i < lst->typemaxlen; i++)
			(void)printf(" ");
	else
		lst->typemaxlen = 5;

	/* option to display a wider bar */
	if (wflag) {
		barinc = 35;
	}
	(void)printf("USED (*)");
	for (i = 0; i < (barinc + 1); i++)
		(void)printf(" ");
	(void)printf("FREE (-) ");

	(void)printf("%%USED");
	if (kflag)
		(void)printf("  ");
	else if (mflag || gflag)
		(void)printf(" ");
	else
		(void)printf("       ");

	(void)printf("AVAILABLE");
	if (kflag)
		(void)printf("      ");
	else if (mflag)
		(void)printf("     ");
	else if (gflag)
		(void)printf("   ");
	else
		(void)printf("           ");
	(void)printf("TOTAL");
	(void)puts(" MOUNTED ON");
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

	return n;
	/* NOTREACHED */
}

/*
 * Return the longest of the two parameters
 */
int
imax(int a, int b)
{
	return (a > b ? a : b);
}
