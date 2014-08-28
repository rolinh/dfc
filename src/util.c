/*
 * Copyright (c) 2012-2014, Robin Hahling
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
 * util.c
 *
 * Various util functions
 */
#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>

#include "util.h"

#ifdef NLS_ENABLED
#include <libintl.h>
#endif

/*
 * Return the longest of the two parameters
 * @a: first element to compare
 * @b: second element to compare
 */
int
imax(int a, int b)
{
	return (a > b ? a : b);
}

/*
 * trim withespaces from the input string and returns it
 * @str: string that needs to be trimmed
 */
char *
strtrim(char *str)
{
	char *end;

	if (!str)
		return NULL;

	while (isspace(*str))
		str++;

	if (*str == '\0')
		return str;

	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;

	*(end + 1) = '\0';

	return str;
}

/*
 * Shorten the input string to the specified length
 * @str: string to shorten
 * @len: the length the new string should be
 */
char *
shortenstr(char *str, int len)
{
	int slen = (int)strlen(str);

	if (slen < len + 1)
		return str;

	str += (slen - len);

	str[0] = '+';

	return str;
}

/*
 * Given a string S, returns the same string where the '_' character is replaced
 * by "\_". The returned string must be freed by the caller.
 * Return NULL if it fails.
 */
char *
sanitizestr(const char *str)
{
	int i = 0;
	int j = 0;
	size_t nchars = 1; /* Trailing \0 */
	char *new;

	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == '_')
			nchars += 2;
		else
			nchars++;
	}

	if (i == (int)nchars) /* No '_' was found. */
		return strdup(str);

	if ((new = malloc(nchars)) == NULL) {
		(void)fputs("malloc failed\n", stderr);
		return NULL;
	}

	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == '_') {
			new[j] = '\\';
			new[j+1] = '_';
			j += 2;
		} else {
			new[j] = str[i];
			j++;
		}
	}

	new[nchars-1] = '\0';

	return new;
}

/*
 * convert to human readable format and return the information i to format
 * correctly the output.
 * @n: address of the number to convert
 */
int
humanize(double *n)
{
	int i = 0;
	double divider = 1024.0;

	/* when using SI units... */
	if (mflag)
		divider = 1000.0;

	while ((*n >= 1000) && (i < 8)) {
		*n /= divider;
		i++;
	}

    return i;
}

/*
 * convert to human readable format and return the information i to format
 * correctly the output. This one is intended to convert inodes to h-r
 * @n: address of the number to convert
 */
int
humanize_i(uint64_t *n)
{
	int i = 0;

	while ((*n >= 10000) && (i < 8)) {
		*n /= 1000;
		i++;
	}

    return i;
}

/*
 * Print a letter according to the desired unit
 * @i: should be the result of the humanize or humanize_i function when humanize
 *     is used
 * @mode: either 0 or 1. 1 mode should be used when called to print filesystem
 *	  unit and 0 should be used when wanting to display "inodes unit"
 */
void
print_unit(int i, int mode)
{
	switch (unitflag) {
	case 'h':
		switch (i) {
		case 0: /* bytes */
			if (mode)
				(void)printf("B");
			else
				(void)printf(" ");
			return;
		case 1: /* Kio  or Ko */
			(void)printf("K");
			return;
		case 2: /* Mio or Mo */
			(void)printf("M");
			return;
		case 3: /* Gio or Go*/
			(void)printf("G");
			return;
		case 4: /* Tio or To*/
			(void)printf("T");
			return;
		case 5: /* Pio or Po*/
			(void)printf("P");
			return;
		case 6: /* Eio or Eo*/
			   (void)printf("E");
			return;
		case 7: /* Zio or Zo*/
			(void)printf("Z");
			return;
		case 8: /* Yio or Yo*/
			(void)printf("Y");
			return;
		default:
			(void)fputs("Could not print unit type in"
					" human-readable format\n", stderr);
		}
	case 'b':
		if (mode)
			(void)printf("B");
		else
			(void)printf(" ");
		return;
	case 'k':
		(void)printf("K");
		return;
	case 'm':
		(void)printf("M");
		return;
	case 'g':
		(void)printf("G");
		return;
	case 't':
		(void)printf("T");
		return;
	case 'p':
		(void)printf("P");
		return;
	case 'e':
		(void)printf("E");
		return;
	case 'z':
		(void)printf("Z");
		return;
	case 'y':
		(void)printf("Y");
		return;
	default:
		(void)fputs("Could not print unit type\n", stderr);
	}
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
	case 'e':
		if (mflag) /* 1000^6 */
			return n / 1000000000000000000.0;
		else /* 1024^6 */
			return n / 1152921504606846976.0;
	case 'g':
		if (mflag) /* 1000^3 */
			return n / 1000000000.0;
		else /* 1024^3 */
			return n / 1073741824.0;
	case 'k':
		if (mflag)
			return n / 1000.0;
		else
			return n / 1024.0;
	case 'm':
		if (mflag) /* 1000^2 */
			return n / 1000000.0;
		else /* 1024^2 */
			return n / 1048576.0;
	case 'p':
		if (mflag) /* 1000^5 */
			return n / 1000000000000000.0;
		else /* 1024^5 */
			return n / 1125899906842624.0;
	case 't':
		if (mflag) /* 1000^4 */
			return n / 1000000000000.0;
		else /* 1024^4 */
			return n / 1099511627776.0;
	case 'y':
		if (mflag) /* 1000^8 */
			return n / 1000000000000000000000000.0;
		else /* 1024^8 */
			return n / 1208925819614629174706176.0;
	case 'z':
		if (mflag)
			/* 1000^7 */
			return n / 1000000000000000000000.0;
		else /* 1024^7 */
			return n / 1180591620717411303424.0;
	default:
		(void)fputs("Could not convert unit size\n", stderr);
		return n;
	}
}

/*
 * Return:
 *	1 if the given fs should be showed
 *	0 if the given fs should be skipped
 * @fs: fs type or name to check
 * @filter: filter string
 * @nm: boolean indicating if the negative matching is activated
 */
int
fsfilter(const char *fs, const char *filter, int nm)
{
	int ret = 0; /* assume it should not be shown */
	char *stropt;
	char *strtmp;

	if ((strtmp = strdup(filter)) == NULL) {
		(void)fputs("Cannot duplicate filter\n", stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	stropt = strtok(strtmp, ",");
	while (stropt != NULL) {
		if (strncmp(fs, stropt, strlen(stropt)) == 0) {
			ret = 1;
			break;
		}
		stropt = strtok(NULL, ",");
	}
	free(strtmp);

	/* reverse result if negative matching activated */
	return nm ? !ret : ret;
}

/*
 * Compares regarding qflag
 * @a: first element of comparison
 * @b: second element of comparison
 */
int
cmp(struct fsmntinfo *a, struct fsmntinfo *b)
{
	switch(qflag) {
	case 1:
		return strcmp(a->fsname, b->fsname);
	case 2:
		return strcmp(a->fstype, b->fstype);
	case 3:
		return strcmp(a->mntdir, b->mntdir);
	default:
		return -1;
	}
}

/*
 * Perform a mergesort algorithm to sort the list by ascending
 * Results depends on what was chosen for comparison (fsname, fstype or mntdir)
 * @fmi: pointer to the first element of the linked list structure to be sorted
 */
struct fsmntinfo *
msort(struct fsmntinfo *fmi)
{
	struct fsmntinfo *tail, *left, *right, *next;
	int nmerges, lsize, rsize;
	int size = 1;

	/*
	 * trivial case: list is sorted if there is no or only one element in
	 * the linked list
	 */
	if (fmi == NULL || fmi->next == NULL)
		return fmi;

	do {
		nmerges = 0;
		left = fmi;
		tail = fmi = NULL;
		while (left) {
			nmerges++;
			right = left;
			lsize = 0;
			rsize = size;
			while (right && lsize < size) {
				lsize++;
				right = right->next;
			}
			while (lsize > 0 || (rsize > 0 && right)) {
				if (!lsize) {
					next = right;
					right = right->next;
					rsize--;
				} else if (!rsize || !right) {
					next = left;
					left = left->next;
					lsize--;
				} else if (cmp(left, right) <= 0) {
					next = left;
					left = left->next;
					lsize--;
				} else {
					next = right;
					right = right->next;
					rsize--;
				}
				if (tail)
					tail->next = next;
				else
					fmi = next;
				tail = next;
			}
			left = right;
		}
		tail->next = NULL;
		size *= 2;
	} while (nmerges > 1);

	return fmi;
}

/*
 * Get the with of TTY and retun it.
 * 0 is returned if stdout is not a tty.
 */
int
getttywidth(void)
{
	int width = 0;
	struct winsize win;

	if (!isatty(STDOUT_FILENO))
		return 0;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == 0)
		width = win.ws_col;

	return width == 0 ? 80 : width;
}

/*
 * init a maxwidths structure
 */
void
init_maxwidths(void)
{
	/*
	 * init min width to header names and width of the graph bar + 1 to have
	 * a space between each column.
	 */
	max.fsname	= (int)strlen(_("FILESYSTEM")) + 1;
	max.fstype	= Tflag ? (int)strlen(_("TYPE")) + 1 : 0;
	max.bar		= bflag ? 0 : wflag ? GRAPHBAR_WIDE : GRAPHBAR_SHORT;
	max.perctused	= (int)strlen(_("%USED")) + 1;
	max.used	= dflag ? (int)strlen(_("USED")) + 1 : 0;
	max.avail	= (int)strlen(_("AVAILABLE")) + 1;
	max.total	= (int)strlen(_("TOTAL")) + 1;
	max.nbinodes	= iflag ? (int)strlen(_("#INODES")) + 1 : 0;
	max.avinodes	= iflag ? (int)strlen(_("AV.INODES")) + 1 : 0;
	max.mntdir	= (int)strlen(_("MOUNTED ON")) + 1;
	max.mntopts	= oflag ? (int)strlen(_("MOUNT OPTIONS")) + 1: 0;
}

/*
 * Return the required width necessary for the number to be displayed.
 * This functions takes into account the unit used (b, k, m, g, ...).
 * @fs_size: file system size from which the required width for displaying
 * should be computed.
 * */
int
get_req_width(double fs_size)
{
	long i, index;
	double req_width, req_min;
	const char *unitstring = "bkmgtpezy";
	char *match;

	/* spaces for the unit symbol and floating point */
	req_min = 4.0;
	req_width = req_min;

	if (unitflag == 'h') {
		req_width += 5; /* max 111.1 => 5 */
	} else {
		if ((match = strchr(unitstring, unitflag)) == NULL) {
			(void)fprintf(stderr,
			    _("Cannot compute required width"));
			return -1;
		}

		if (fs_size > 0.0)
			req_width += 1.0 + floor(log10(fs_size));

		index = match - unitstring + 1;
		printf("index: %ld\n", index);
		for (i = 1; i < index; i++)
			req_width -= 3.0;

		req_width = ceil(req_width);
	}

	/* XXX: cannot cast double to int */
	return (req_width < req_min) ? (int)req_min : (int)req_width;
}

void
update_maxwidth(struct fsmntinfo *fmi)
{
	if (!aflag && (is_mnt_ignore(fmi) == 1))
		return;

	/* + 1 for a space between each column */
	max.fsname = imax((int)strlen(fmi->fsname) + 1, max.fsname);
	max.fstype = imax((int)strlen(fmi->fstype) + 1, max.fstype);
	max.mntdir = imax((int)strlen(fmi->mntdir) + 1, max.mntdir);
	max.mntopts = imax((int)strlen(fmi->mntopts) + 1, max.mntopts);

	printf("mntdir: %s\n", fmi->mntdir);
	max.used = imax(get_req_width(fmi->used), max.used);
	max.avail = imax(get_req_width(fmi->avail), max.avail);
	max.total = imax(get_req_width(fmi->total), max.total);
	printf("max.total: %d\n", max.total);
}

/*
 * auto-adjust options based on the size needed to display the informations
 * @lst: list containing info
 * @tty_width: width of the output terminal
 */
void
auto_adjust(int tty_width)
{
	int req_width;

	req_width = max.fsname + max.fstype + max.bar + max.perctused + max.used
		    + max.avail + max.total + max.nbinodes + max.avinodes
		    + max.mntdir + max.mntopts;

	if (tty_width > req_width)
		return; /* nothing to adjust */

	(void)fputs(_("WARNING: TTY too narrow. Some options have been disabled"
		" to make dfc output fit (use -f to override).\n"), stderr);
	if (!bflag) {
		if (wflag) {
			wflag = 0;
			req_width -= GRAPHBAR_WIDE - GRAPHBAR_SHORT;
			if (tty_width >= req_width)
				return;
		}
		bflag = 1;
		req_width -= GRAPHBAR_SHORT;
		if (tty_width >= req_width)
			return;
	}
	if (dflag) {
		dflag = 0;
		req_width -= max.used;
		if (tty_width >= req_width)
			return;
	}
	if (Tflag) {
		Tflag = 0;
		req_width -= max.fstype;
		if (tty_width >= req_width)
			return;
	}
	if (iflag) {
		iflag = 0;
		req_width -= max.nbinodes;
		if (tty_width >= req_width)
			return;
	}
	if (oflag) {
		oflag = 0;
		req_width -= max.mntopts;
		if (tty_width >= req_width)
			return;
	}

	(void)fputs(_("WARNING: Output still messed up. Enlarge your "
			"terminal if you can...\n"), stderr);
}

/*
 * return the current date as of date(1) format
 * NULL is returned in case of errors
 */
char *
fetchdate(void)
{
	char date[255];
	time_t t;
	struct tm *tmp;

	if ((t = time(NULL)) == -1) {
		perror("time");
		return NULL;
	}
	if ((tmp = localtime(&t)) == NULL) {
		perror("localtime");
		return NULL;
	}

	if (strftime(date, sizeof(date), "%c", tmp) == 0) {
		(void)fputs("Could not retrieve date\n", stderr);
		return NULL;
	}

	return strdup(date);
}

/*
 * Return the name of the given color or NULL
 * @color: color as defined in extern.h
 */
const char *
colortostr(int color)
{
	switch (color) {
	case BLACK:
		return "black";
	case RED:
		return "red";
	case GREEN:
		return "green";
	case YELLOW:
		return "yellow";
	case BLUE:
		return "blue";
	case MAGENTA:
		return "magenta";
	case CYAN:
		return "cyan";
	case WHITE:
		return "white";
	default:
		return NULL;
	}
}

/*
 * convert color from natural name into correponding number and return it
 * @col: color name
 */
int
colortoint(const char *col)
{
	if (strcoll(col, _("black")) == 0)
		return BLACK;
	else if (strcoll(col, _("red")) == 0)
		return RED;
	else if (strcoll(col, _("green")) == 0)
		return GREEN;
	else if (strcoll(col, _("yellow")) == 0)
		return YELLOW;
	else if (strcoll(col, _("blue")) == 0)
		return BLUE;
	else if (strcoll(col, _("magenta")) == 0)
		return MAGENTA;
	else if (strcoll(col, _("cyan")) == 0)
		return CYAN;
	else if (strcoll(col, _("white")) == 0)
		return WHITE;
	else
		return -1;
}

/*
 * check if the input string is a valid html color code.
 * ie: it should be an hexadecimal value
 * Returns 0 if all went well, otherwise it returns -1
 * @color: input color
 * NOTE: color should ommit the #: white is FFFFFF and not #FFFFFF
 */
int
chk_html_colorcode(const char *color)
{
	int i;

	if (strlen(color) != HTMLCOLORCODELENGTH)
		return -1;

	for (i = 0; i < HTMLCOLORCODELENGTH; i++)
		if (isxdigit(color[i]) == 0)
			return -1;

	return 0;
}
