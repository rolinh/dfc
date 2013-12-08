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
#include <unistd.h>
#include <err.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <string.h>

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */



#include "extern.h"
#include "services.h"
#include "util.h"

/* Hide differences between NetBSD and the other BSD. */
#if defined(__NetBSD__)
typedef struct statvfs statst;
#define GET_FLAGS(vfsbuf)  ((vfsbuf).f_flag)
#define GET_FRSIZE(vfsbuf) ((vfsbuf).f_frsize)
#define GET_FAVAIL(vfsbuf) ((vfsbuf).f_favail)
#else /* Other BSDs. */
typedef struct statfs statst;
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
	statst *entbuf;
	statst vfsbuf, **fs;
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

		/* infos from statvfs */
		fmi->flags    = GET_FLAGS(vfsbuf);
		fmi->bsize    = vfsbuf.f_bsize;
		fmi->frsize   = GET_FRSIZE(vfsbuf);
		fmi->blocks   = vfsbuf.f_blocks;
		fmi->bfree    = vfsbuf.f_bfree;
		fmi->bavail   = vfsbuf.f_bavail;
		fmi->files    = vfsbuf.f_files;
		fmi->ffree    = vfsbuf.f_ffree;
		fmi->favail   = GET_FAVAIL(vfsbuf);

		if ((fmi->opts = statfs_flags_to_str(fmi)) == NULL) {
			fmi->opts = g_none_str;
		}

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
statfs_flags_to_str(const struct fsmntinfo *fs)
{
	int i, n_flags;
	size_t bufsize = 128;
	char *buffer = malloc(bufsize);
	if (!buffer) {
		(void)fprintf(stderr,
				_("Could not retrieve mount flags for %s\n"),
				fs->fsname);
		return NULL;
	}
	buffer[0] = '\0';

	/* There is no MNT_RDWRITE flag, so we have to do this. */
	if (fs->flags & MNT_RDONLY) {
		if (strlcat(buffer, "ro", bufsize) >= bufsize)
			goto truncated;
	} else {
		if (strlcat(buffer, "rw", bufsize) >= bufsize)
			goto truncated;
	}

	/* Comparing flags to all possible flags. */
	n_flags = sizeof(possible_flags) / sizeof(possible_flags[0]);
	for (i = 0; i < n_flags; i++) {
		if (!(fs->flags & possible_flags[i].flag))
			continue;
		if (strlcat(buffer, ",", bufsize) >= bufsize)
			goto truncated;
		if (strlcat(buffer, possible_flags[i].str, bufsize) >= bufsize)
			goto truncated;
	}

	return buffer;

truncated:
       (void)fprintf(stderr, _("Truncating mount options for %s\n"),
			fs->fsname);
	return buffer;
}

#endif /* BSD */
