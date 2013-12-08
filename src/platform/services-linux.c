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
 * services-linux.c
 *
 * Linux implemention of services.
 */
#ifdef __linux__

#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

#include <mntent.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>

#include "extern.h"
#include "services.h"
#include "util.h"

int
is_mnt_ignore(const struct fsmntinfo *fs)
{
	/* if the size is zero, it is most likely a fs that we want to ignore */
	if (fs->blocks == 0)
		return 1;

	/* treat tmpfs/devtmpfs/... as a special case */
	if (fs->type && strstr(fs->type, "tmpfs"))
		return 0;

	return is_pseudofs(fs->type);
}

int
is_remote(const struct fsmntinfo *fs)
{
	const char remote_fs[] = "afs cifs coda fuse.sshfs mfs "
            "ncpfs ftpfs nfs smbfs sshfs";

	if (strstr(remote_fs, fs->type))
		return 1;

	return 0;
}

/*
 * Comparison function needed in is_pseudofs for bsearch call.
 */
static int
typecmp(const void *e1, const void *e2)
{
	const char *s1 = *(const char **)e1;
	const char *s2 = *(const char **)e2;
	return strcmp(s1, s2);
}

int
is_pseudofs(const char *type)
{
	/* keep sorted for binary search */
	static const char *pseudofs[] = {
		"anon_inodefs",
		"autofs",
		"bdev",
		"binfmt_misc",
		"cgroup",
		"configfs",
		"cpuset",
		"debugfs",
		"devfs",
		"devpts",
		"devtmpfs",
		"dlmfs",
		"fuse.gvfs-fuse-daemon",
		"fusectl",
		"hugetlbfs",
		"mqueue",
		"nfsd",
		"none",
		"pipefs",
		"proc",
		"pstore",
		"ramfs",
		"rootfs",
		"rpc_pipefs",
		"securityfs",
		"sockfs",
		"spufs",
		"sysfs",
		"tmpfs"
	};

	if (!type)
		return -1;

	if (bsearch(&type, pseudofs, sizeof(pseudofs) / sizeof(pseudofs[0]),
		sizeof(char*), typecmp) == NULL) {
		return 0;
	}

	return 1;
}

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

void
fetch_info(struct list *lst)
{
	struct fsmntinfo *fmi;
	FILE *mtab;
	struct mntent *entbuf;
	struct statvfs vfsbuf;
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
			/* display a warning when a FS cannot be stated */
			(void)fprintf(stderr, _("WARNING: %s was skipped "
				"because it could not be stated"),
				entbuf->mnt_dir);
			perror(" ");
		} else {
			/* infos from getmntent */
			if (Wflag) { /* Wflag to avoid name truncation */
				if ((fmi->fsname = strdup(entbuf->mnt_fsname))
						== NULL) {
					/* g_unknown_str is def. in extern.h(.in) */
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup(entbuf->mnt_dir))
						== NULL) {
					fmi->dir = g_unknown_str;
				}
			} else {
				if ((fmi->fsname = strdup(shortenstr(
					entbuf->mnt_fsname,
					STRMAXLEN))) == NULL) {
					fmi->fsname = g_unknown_str;
				}
				if ((fmi->dir = strdup(shortenstr(entbuf->mnt_dir,
							STRMAXLEN))) == NULL) {
					fmi->dir = g_unknown_str;
				}
			}
			if ((fmi->type = strdup(shortenstr(entbuf->mnt_type,
							12))) == NULL) {
				fmi->type = g_unknown_str;
			}
			if ((fmi->opts = strdup(entbuf->mnt_opts)) == NULL) {
				fmi->opts = g_none_str;
			}

			/* infos from statvfs */
			fmi->bsize    = vfsbuf.f_bsize;
			fmi->frsize   = vfsbuf.f_frsize;
			fmi->blocks   = vfsbuf.f_blocks;
			fmi->bfree    = vfsbuf.f_bfree;
			fmi->bavail   = vfsbuf.f_bavail;
			fmi->files    = vfsbuf.f_files;
			fmi->ffree    = vfsbuf.f_ffree;
			fmi->favail   = vfsbuf.f_favail;
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
	}
	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");
	free(fmi);
}

#endif /* __linux__ */
