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

#include <stdlib.h>
#include <string.h>

#include "services.h"

#ifdef __linux__

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
typecmp(const void *e1, const void *e2)
{
	const char *s1 = *(const char **)e1;
	const char *s2 = *(const char **)e2;
	return strcmp(s1, s2);
}

#endif
