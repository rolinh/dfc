/*
 * main.c
 *
 * (C) 2011 - Hahling Robin <robin.hahling@gw-computing.net>
 *
 * Displays free disk space in an elegant manner with color support
 */
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <sys/param.h>

/* structure to store infos from /etc/mtab */
struct mtabinfo {
	char device[MAXPATHLEN];
	char mntpt[MAXPATHLEN];
	char fstype[MAXPATHLEN];
};

/* set flags for options */
int hflag, Hflag, vflag, Vflag;

/* functions declaration */
static void colormsg(FILE *output, unsigned int color);
static int display(void);
struct mntpt getmntpt(FILE *mtab);
int getfileusg();
static void usage(void);

int
main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "hHvV:")) != -1) {
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
		case 'V':
			Vflag = 1;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
}

static void
colormsg(FILE *output, unsigned int color)
{

}

static int
display(void)
{

}

struct mtabinfo
getmntpt(FILE *mtab)
{
	struct mtabinfo crtdev = {"","",""};

	if (fscanf(mtab, "%s %s %s", crtdev.device, crtdev.mntpt, crtdev.fstype) == EOF) {
		/* error occured */
		if (ferror(mtab) != 0) {
			perror("Error while reading mtab");
			(void)fclose(mtab);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}
	}

	/* in case of EOF, crtdev does not change (length of elt = 0) */
	return crtdev;
	/* NOTREACHED */
}

getfileusg()
{

}

static void
usage(void)
{
	(void)fputs("Usage: dfc [OPTIONS(S)]\n"
		"Available options:\n"
		"	-h	print size in human readable format\n"
		"	-H	print size in human readable format but using powers of 1000, not 1024\n"
		"	-v	verbose mode\n"
		"	-V	print program version\n",
		stderr)

	exit(EXIT_FAILURE);
	/* NOTREACHED */
}
