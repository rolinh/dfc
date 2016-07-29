/*
 * Copyright (c) 2012-2016, Robin Hahling
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__) || defined(__GLIBC__)

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

#include <mntent.h>
#include <sys/statvfs.h>
#include <errno.h>

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
	if (fs->fstype && strstr(fs->fstype, "tmpfs"))
		return 0;

	return is_pseudofs(fs->fstype);
}

int
is_remote(const struct fsmntinfo *fs)
{
	return is_remotefs(fs->fstype);
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
		/* avoid stating remote fs because they may hang */
		if (lflag && is_remotefs(entbuf->mnt_type))
			continue;
		/* get infos from statvfs */
		if (statvfs(entbuf->mnt_dir, &vfsbuf) == -1) {
			/* show only "real" errors, not lack of permissions */
			if (errno == EACCES)
				continue;
			/* display a warning when a FS cannot be stated */
			(void)fprintf(stderr, _("WARNING: %s was skipped "
				"because it could not be stated"),
				entbuf->mnt_dir);
			perror(" ");
			continue;
		}
		/* infos from getmntent */
		if (Wflag) { /* Wflag to avoid name truncation */
			if ((fmi->fsname = strdup(entbuf->mnt_fsname)) == NULL)
				fmi->fsname = g_unknown_str;
			if ((fmi->mntdir = strdup(entbuf->mnt_dir)) == NULL)
				fmi->mntdir = g_unknown_str;
			if ((fmi->fstype = strdup(entbuf->mnt_type)) == NULL)
				fmi->fstype = g_unknown_str;
		} else {
			if ((fmi->fsname = strdup(shortenstr(
				entbuf->mnt_fsname, STRMAXLEN))) == NULL) {
				fmi->fsname = g_unknown_str;
			}
			if ((fmi->mntdir = strdup(shortenstr(
				entbuf->mnt_dir, STRMAXLEN))) == NULL) {
				fmi->mntdir = g_unknown_str;
			}
			if ((fmi->fstype = strdup(shortenstr(
				entbuf->mnt_type, STRMAXLEN))) == NULL) {
				fmi->fstype = g_unknown_str;
			}
		}

		if ((fmi->mntopts = strdup(entbuf->mnt_opts)) == NULL)
			fmi->mntopts = g_none_str;

		/* infos from statvfs */
		fmi->bsize    = vfsbuf.f_bsize;
		fmi->frsize   = vfsbuf.f_frsize;
		fmi->blocks   = vfsbuf.f_blocks;
		fmi->bfree    = vfsbuf.f_bfree;
		fmi->bavail   = vfsbuf.f_bavail;
		fmi->files    = vfsbuf.f_files;
		fmi->ffree    = vfsbuf.f_ffree;
		fmi->favail   = vfsbuf.f_favail;

		/* compute, available, % used, etc. */
		compute_fs_stats(fmi);

		/* pointer to the next element */
		fmi->next = NULL;

		/* enqueue the element into the queue */
		enqueue(lst, *fmi);
	}
	/* we need to close the mtab file now */
	if (fclose(mtab) == EOF)
		perror("Could not close mtab file ");
	free(fmi);
}

void
compute_fs_stats(struct fsmntinfo *fmi)
{
	fmi->total = (double)fmi->frsize * (double)fmi->blocks;
	fmi->avail = (double)fmi->frsize * (double)fmi->bavail;
	fmi->used  = (double)fmi->frsize * ((double)fmi->blocks - (double)fmi->bfree);
	if ((int)fmi->total == 0)
		fmi->perctused = 100.0;
	else
		fmi->perctused = 100.0 -
			((double)fmi->bavail / (double)fmi->blocks) * 100.0;
}

#endif /* __linux__ */
