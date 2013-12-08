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
 * @type: fs type to check
 * @filter: filter string
 * @nm: boolean indicating if the negative matching is activated
 */
int
fstypefilter(const char *type, const char *filter, int nm)
{
	int ret = 1;
	char *stropt;
	char *strtmp;

	if (tflag) {
		if ((strtmp = strdup(filter)) == NULL) {
			(void)fputs("Cannot duplicate filter\n", stderr);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}
		/* assume it should not be shown */
		ret = 0;
		stropt = strtok(strtmp, ",");
		while (stropt != NULL) {
			if (strcmp(type, stropt) == 0) {
				ret = 1;
				break;
			}
			stropt = strtok(NULL, ",");
		}
		free(strtmp);
	}

	/* reverse result if negative matching activated */
	if (nm) {
		if (ret)
			ret = 0;
		else
			ret = 1;
	}

	return ret;
}

/*
 * Return:
 *	1 if the given fs should be showed
 *	0 if the given fs should be skipped
 * @type: fs type to check
 * @filter: filter string
 * @nm: boolean indicating if the negative matching is activated
 */
int
fsnamefilter(const char *fsname, const char *filter, int nm)
{
	int ret = 1;
	char *stropt;
	char *strtmp;

	if (pflag) {
		if ((strtmp = strdup(filter)) == NULL) {
			(void)fputs("Cannot duplicate filter\n", stderr);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}
		/* assume it should not be shown */
		ret = 0;
		stropt = strtok(strtmp, ",");
		while (stropt != NULL) {
			if (strncmp(fsname, stropt, strlen(stropt)) == 0) {
				ret = 1;
				break;
			}
			stropt = strtok(NULL, ",");
		}
		free(strtmp);
	}

	/* reverse result if negative matching activated */
	if (nm) {
		if (ret)
			ret = 0;
		else
			ret = 1;
	}

	return ret;
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
		return strcmp(a->type, b->type);
	case 3:
		return strcmp(a->dir, b->dir);
	default:
		return -1;
	}
}

/*
 * Perform a mergesort algorithm to sort the list by ascending
 * Results depends on what was chosen for comparison (fsname, type or dir)
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
 * auto-adjust options based on the size needed to display the informations
 * @lst: list containing info
 * @width: width of the output
 */
void
auto_adjust(struct list lst, int width)
{
	int req, gap;

	req = req_width(lst);

	/* nothing to adjust here */
	if ((gap = (width - req)) >= 0)
		return;

	(void)fputs(_("WARNING: TTY too narrow. Some options have been disabled"
		" to make dfc output fit (use -f to override).\n"), stderr);

	if (!bflag) {
		/* large graph should be the first option to disable */
		if (wflag) {
			wflag = 0;
			gap += 30;
			if (gap >= 0)
				return;
		}
		bflag = 1;
		gap += 23;
		if (gap >= 0)
			return;
	}
	if (dflag) {
		dflag = 0;
		gap += 4;
		if (unitflag == 'k')
			gap += 7;
		else if (unitflag == 'b')
			gap += 12;
		else
			gap += 6;
		if (gap >= 0)
			return;
	}
	if (Tflag) {
		Tflag = 0;
		gap += imax(lst.typemaxlen, 5);
		if (gap >= 0)
			return;
	}
	if (iflag) {
		iflag = 0;
		gap += 20;
		if (gap >= 0)
			return;
	}
	if (oflag) {
		oflag = 0;
		gap += imax(lst.mntoptmaxlen, 13);
		if (gap >= 0)
			return;
	}
	if (gap < 0)
		(void)fputs(_("WARNING: Output still messed up. Enlarge your "
				"terminal if you can...\n"), stderr);
}

/*
 * compute the required width needed for the output and return it
 * (computation based on text_disp_header function)
 * @lst: list containing the info
 */
int
req_width(struct list lst)
{
	int ret;

	/* dir and fs are always displayed */
	ret = imax(lst.fsmaxlen, 11);

	if (Tflag)
		ret += imax(lst.typemaxlen, 5);
	if (!bflag) {
		ret += 23;
		if (wflag)
			ret += 30;
	}
	/* % */
	ret += 5;

	if (dflag) {
		if (unitflag == 'k')
			ret += 7;
		else if (unitflag == 'b')
			ret += 12;
		else
			ret += 6;
		ret += 4;
	}

	/* available */
	ret += 9;

	switch (unitflag) {
	case 'b':
		ret += 7 + 11;
		break;
	case 'k':
		ret += 2 + 6;
		break;
	case 'm':
		ret += 5;
		break;
	case 'h': /* FALLTHROUGH */
	case 'g': /* FALLTHROUGH */
	case 't': /* FALLTHROUGH */
	case 'p': /* FALLTHROUGH */
	case 'e': /* FALLTHROUGH */
	case 'z': /* FALLTHROUGH */
	case 'y': /* FALLTHROUGH */
		ret += 1 + 5;
		break;
	default:
		(void)fputs("Unknown unit type\n", stderr);
	}

	/* total */
	ret += 5;

	if (iflag)
		ret += 20;

	/* mounted on */
	ret += imax(lst.dirmaxlen, 12);

	if (oflag)
		ret += imax(lst.mntoptmaxlen, 13);

	return ret;
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
