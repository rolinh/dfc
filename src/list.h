/*
 * Copyright (c) 2012-2015, Robin Hahling
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

#ifndef H_LIST
#define H_LIST
/*
 * list.h
 *
 * list structure
 */

#include <sys/types.h>

/*
 * Structure to store information about mounted fs
 */
struct fsmntinfo {
	/* infos to get from getmntent(3) */
	char *fsname;	/* name of mounted file system */
	char *fstype;	/* mount type */
	char *mntdir;	/* file system path prefix */
	char *mntopts;	/* mount options (see mntent.h) */

	double perctused;   /* fs usage in % */
	double total;	    /* fs total size */
	double avail;	    /* fs available size */
	double used;	    /* fs used size */

	/* infos to get from statvfs(3) */
#if defined(__linux__)
	int		flags;	/* XXX: does not exist on Linux */
	unsigned long	bsize;	/* file system block size */
	unsigned long	frsize;	/* fragment size */
	fsblkcnt_t	blocks;	/* size of fs in frsize unit */
	fsblkcnt_t	bfree;	/* # of free blocks */
	fsblkcnt_t	bavail;	/* # of available blocks */
	fsfilcnt_t	files;	/* # of inodes */
	fsfilcnt_t	ffree;	/* # of free inodes */
	fsfilcnt_t	favail;	/* # of available inodes */
#endif /* __linux__ */
#if defined(__NetBSD__)
	unsigned long	flags;	/* mount exported flags */
	unsigned long	bsize;	/* file system block size */
	unsigned long	frsize;	/* fragment size */
	fsblkcnt_t	blocks;	/* size of fs in frsize unit */
	fsblkcnt_t	bfree;	/* # of free blocks */
	fsblkcnt_t	bavail;	/* # of available blocks */
	fsfilcnt_t	files;	/* # of inodes */
	fsfilcnt_t	ffree;	/* # of free inodes */
	fsfilcnt_t	favail;	/* # of available inodes */
#endif /* __NetBSD__ */
#if defined(__FreeBSD__)
	uint64_t	flags;	/* mount exported flags */
	uint64_t	bsize;	/* file system block size */
	unsigned long   frsize;	/* XXX: does not exist on FreeBSD */
	uint64_t	blocks;	/* size of fs in frsize unit */
	uint64_t	bfree;	/* # of free blocks */
	int64_t		bavail;	/* # of available blocks */
	uint64_t	files;	/* # of inodes */
	int64_t		ffree;	/* # of free nodes to non super-user */
	unsigned long	favail;	/* XXX: does not exist on FreeBSD */
#endif /* __FreeBSD__ */
#if defined(__OpenBSD__)
	u_int32_t	flags;	/* mount exported flags */
	u_int32_t	bsize;	/* file system block size */
	unsigned long	frsize;	/* XXX: does not exist on OpenBSD */
	u_int64_t	blocks;	/* size of fs in frsize unit */
	u_int64_t	bfree;	/* # of free blocks */
	int64_t		bavail;	/* # of available blocks */
	u_int64_t	files;	/* # of inodes */
	u_int64_t	ffree;	/* # of free inodes */
	int64_t		favail;	/* # of available inodes */
#endif /* __OpenBSD__ */
#if defined(__DragonFly__)
	int		flags;	/* mount exported flags */
	long		bsize;	/* file system block size */
	unsigned long	frsize;	/* XXX: does not exist on DragonFly */
	long		blocks;	/* size of fs in frsize unit */
	long		bfree;	/* # of free blocks */
	long		bavail;	/* # of available blocks */
	long		files;	/* # of inodes */
	long		ffree;	/* # of free inodes */
	unsigned long	favail;	/* XXX: does not exist on DragonFly */
#endif /* __DragonFly__ */
#if defined(__APPLE__)
	uint32_t	flags;	/* mount exported flags */
	uint32_t	bsize;	/* file system block size */
	unsigned long	frsize;	/* XXX: does not exist on OSX */
	uint64_t	blocks;	/* size of fs in frsize unit */
	uint64_t	bfree;	/* # of free blocks */
	uint64_t	bavail;	/* # of available blocks */
	uint64_t	files;	/* # of inodes */
	uint64_t	ffree;	/* # of free inodes */
	unsigned long	favail;	/* XXX: does not exist on OSX */
#endif /* __APPLE__ */

	/* pointer to the next element of the list */
	struct fsmntinfo *next;
};

/* list structure to store fsmntinfo */
struct list {
	struct fsmntinfo *head;
	struct fsmntinfo *tail;
};

/* function declaration */
void init_queue(struct list *lst);
int is_empty(struct list lst);
int enqueue(struct list *lst, struct fsmntinfo elt);
struct fsmntinfo fmi_init(void);
struct fsmntinfo *delete_struct_and_get_next(struct fsmntinfo *p);

#endif /* ndef LIST_H */
