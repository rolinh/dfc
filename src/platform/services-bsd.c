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
 * services-bsd.c
 *
 * BSDs implemention of services.
 */
#if defined(__APPLE__)   || defined(__DragonFly__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

#include <sys/mount.h>

#include "extern.h"
#include "services.h"
#include "util.h"

/* Hide differences between NetBSD and the other BSD. */
#if defined(__NetBSD__)
typedef struct statvfs statfs;
#define GET_FLAGS(vfsbuf)  ((vfsbuf).f_flag)
#define GET_FRSIZE(vfsbuf) ((vfsbuf).f_frsize)
#define GET_FAVAIL(vfsbuf) ((vfsbuf).f_favail)
#else /* Other BSDs. */
typedef struct statfs statfs;
#define GET_FLAGS(vfsbuf)  ((vfsbuf).f_flags)
#define GET_FRSIZE(vfsbuf) (0)
#define GET_FAVAIL(vfsbuf) (0)
#endif

int
is_mnt_ignore(const struct fsmntinfo *fs)
{
	/* TODO: check MNT_IGNORE flags exists on all supported platforms */
	if (fs->flags & MNT_IGNORE)
	    return 1;

	return 0;
}

int
is_remote(const struct fsmntinfo *fs)
{
	if (fs->flags & MNT_LOCAL)
		return 0;

	return 1;
}

int
getttywidth(void)
{
	int width = 0;
#ifdef TIOCGSIZE
	struct ttysize win;
#elif defined(TIOCGWINSZ)
	struct winsize win;
#endif /* TIOCGSIZE */

	if (!isatty(STDOUT_FILENO))
		return 0;

#ifdef TIOCGSIZE
	if (ioctl(STDOUT_FILENO, TIOCGSIZE, &win) == 0)
#if defined(__FreeBSD__)
		width = win.ws_col;
#else
		width = win.ts_cols;
#endif /* __FreeBSD__ */

#elif defined(TIOCGWINSZ)
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == 0)
#if defined(__FreeBSD__)
		width = win.ws_col;
#else
		width = win.ts_cols;
#endif /* __FreeBSD__ */

#endif /* TIOCGSIZE */

	return width == 0 ? 80 : width;
}

void
fetch_info(struct list *lst)
{
	struct fsmntinfo *fmi;
	int nummnt;
	statfs *entbuf;
	statfs vfsbuf, **fs;
	/* init fsmntinfo */
	if ((fmi = malloc(sizeof(struct fsmntinfo))) == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	*fmi = fmi_init();
	if ((nummnt = getmntinfo(&entbuf, MNT_NOWAIT)) <= 0)
		err(EXIT_FAILURE, "Error while getting the list of mountpoints");
		/* NOTREACHED */

	for (fs = &entbuf; nummnt--; (*fs)++) {
		vfsbuf = **fs;
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
		fmi->flags    = GET_FLAGS(vfsbuf);
		/* infos from statvfs */
		fmi->bsize    = vfsbuf.f_bsize;
		fmi->frsize   = GET_FRSIZE(vfsbuf);
		fmi->blocks   = vfsbuf.f_blocks;
		fmi->bfree    = vfsbuf.f_bfree;
		fmi->bavail   = vfsbuf.f_bavail;
		fmi->files    = vfsbuf.f_files;
		fmi->ffree    = vfsbuf.f_ffree;
		fmi->favail   = GET_FAVAIL(vfsbuf);
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
	free(fmi);
}

#endif /* BSD */
