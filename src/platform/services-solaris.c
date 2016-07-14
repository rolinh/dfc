/*
 * Copyright (c) 2016, Robin Hahling
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
 * services-solaris.c
 *
 * Solaris implemention of services.
 */
#ifdef __sun

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef NLS_ENABLED
#include <locale.h>
#include <libintl.h>
#endif /* NLS_ENABLED */

#include <sys/mnttab.h>
#include <sys/statvfs.h>

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

	/* libc is dynamically mounted into /lib, treat it as a special case */
	if (fs->mntdir && (strncmp(fs->mntdir, "/lib/", 5) == 0)) {
		return 1;
	}

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
	FILE *mnttab;
	struct mnttab mnttabbuf;
	struct statvfs vfsbuf;
	int ret;

	/* init fsmntinfo */
	if ((fmi = malloc(sizeof(struct fsmntinfo))) == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	*fmi = fmi_init();

	/* open mnttab file */
	if ((mnttab = fopen("/etc/mnttab", "r")) == NULL) {
		perror("Error while opening mnttab file ");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	/* loop to get infos from all the mounted fs */
	while ((ret = getmntent(mnttab, &mnttabbuf)) == 0) {
		if (statvfs(mnttabbuf.mnt_mountp, &vfsbuf) == -1) {
			(void)fprintf(stderr, _("WARNING: %s was skipped "
				"because it could not be stated"),
				mnttabbuf.mnt_mountp);
			perror(" ");
			continue;
		}
		if (Wflag) { /* Wflag to avoid name truncation */
			if ((fmi->fsname = strdup(mnttabbuf.mnt_special)) == NULL) {
				fmi->fsname = g_unknown_str;
			}
			if ((fmi->mntdir = strdup(mnttabbuf.mnt_mountp))== NULL) {
				fmi->mntdir = g_unknown_str;
			}
		} else {
			if ((fmi->fsname = strdup(shortenstr(
				mnttabbuf.mnt_special,
				STRMAXLEN))) == NULL) {
				fmi->fsname = g_unknown_str;
			}
			if ((fmi->mntdir = strdup(shortenstr(mnttabbuf.mnt_mountp,
						STRMAXLEN))) == NULL) {
				fmi->mntdir = g_unknown_str;
			}
		}
		if ((fmi->fstype = strdup(shortenstr(mnttabbuf.mnt_fstype,
						12))) == NULL) {
			fmi->fstype = g_unknown_str;
		}
		if ((fmi->mntopts = strdup(mnttabbuf.mnt_mntopts)) == NULL) {
			fmi->mntopts = g_none_str;
		}

		fmi->bsize  = vfsbuf.f_bsize;
		fmi->frsize = vfsbuf.f_frsize;
		fmi->blocks = vfsbuf.f_blocks;
		fmi->bfree  = vfsbuf.f_bfree;
		fmi->bavail = vfsbuf.f_bavail;
		fmi->files  = vfsbuf.f_files;
		fmi->ffree  = vfsbuf.f_ffree;
		fmi->favail = vfsbuf.f_favail;

		compute_fs_stats(fmi);

		fmi->next = NULL;
		enqueue(lst, *fmi);

		update_maxwidth(fmi);
	}
	if (ret > 0) {
		(void)fprintf(stderr, "An error occured while reading the "
				"mnttab file\n");
	}

	/* we need to close the mnttab file now */
	if (fclose(mnttab) == EOF)
		perror("Could not close mnttab file ");
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

#endif /* __sun */
