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

#include <string.h>
#include <mntent.h>

#include <sys/param.h>
#include <sys/statvfs.h>

#include "dfc.h"

/* set flags for options */
int hflag, Hflag, vflag;

int
main(int argc, char *argv[])
{
	FILE *mtab;
	struct mntent *entbuf;
	struct statvfs vfsbuf;
	struct fsmntinfo *fmi;
	struct list queue;
	int ch;

	while ((ch = getopt(argc, argv, "hHv:")) != -1) {
		switch (ch) {
		case 'h':
			hflag = 1;
			break;
		case 'H':
			Hflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		case '?':
		default:
			usage(EXIT_FAILURE);
			/* NOTREACHED */
		}
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
			fmi->bsize = vfsbuf.f_bsize;
			fmi->blocks = vfsbuf.f_blocks;
			fmi->bfree = vfsbuf.f_bfree;

			/* pointer to the next element */
			fmi->next = NULL;

			/* enqueue the element into the queue */
			enqueue(&queue, *fmi);
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
		"	-h	print size in human readable format\n"
		"	-H	print size in human readable format but using powers of 1000, not 1024\n"
		"	-v	print program version\n",
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

/* Dequeues the first element of a queue
 * @lst: queue pointer
 * Returns:
 *	--> the element dequeued
 */
struct fsmntinfo
dequeue(struct list *lst)
{
	struct fsmntinfo *p = NULL;
	struct fsmntinfo fmi;

	if (is_empty((*lst))) {
		fmi = fmi_init();
		return fmi;
		/* NOTREACHED */
	}

	/* in case there is only one element in the queue */
	if (lst->head == lst->tail) {
		lst->head->next = NULL;
		lst->tail = NULL;
	}

	p = lst->head;
	lst->head = p->next;
	fmi = (*p);

	return fmi;
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
 * TODO: finish this function
 */
void
disp(struct list lst)
{
	struct fsmntinfo *p = NULL;
	int i;
	int bflen = 1;
	int bllen = 1;
	double used;

	/* legend on top (80 col wide) at most */
	(void)printf("FILESYSTEM");
	for (i = 0; i < 6; i++)
		(void)printf(" ");
	(void)printf("USED (*)");
	for (i = 0; i < 6; i++)
		(void)printf(" ");
	(void)printf("FREE (-) ");
	(void)printf("%%USED ");
	(void)printf("FREE        ");
	(void)printf("TOTAL       ");
	(void)puts("MOUNTED ON");

	/*
	 * here is what we want:
	 * 16 + 1 + 20 + 1 + 1 + 6 + 11 + 11 + 23 = 80
	 */
	p = lst.head;
	while (p != NULL) {
		(void)printf("%s", p->fsname);

		for (i = (int)strlen(p->fsname); i < 16; i++)
			(void)printf(" ");

		/* calculate the % used */
		if (p->blocks == 0)
			used = 100;
		else
			used = ((double)(p->blocks - p->bfree) / (double)(p->blocks)) * 100;

		(void)printf("[");

		for (i = 0; i < 10; i++)
			(void)printf("*");

		for (i = 0; i < 10; i++)
			(void)printf("-");

		/*
		 * to adjust to output, we need to get the len of bfree and
		 * blocks
		 */
		i = (int)(p->bfree);
		while (i > 9) {
			bflen++;
			i = i / 10;
		}

		i = (int)(p->blocks);
		while (i > 9) {
			bllen++;
			i = i / 10;
		}

		(void)printf("] %.f%% %10ld", used, p->bfree);
		for (i = bflen; i < 12; i++)
			(void)printf(" ");

		(void)printf("%ld",p->blocks);
		for (i = bllen; i < 12; i++)
			(void)printf(" ");
		printf("%s\n", p->dir);

		/* reinit the length */
		bflen = bllen = 1;
		p = p->next;
	}
}
