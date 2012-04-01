/*
 * Copyright (c) 2012, Robin Hahling
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1 Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  4 Neither the name of the author nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIST_H
#define LIST_H
/*
 * list.h
 *
 * list structure
 */

/*
 * structure needed to store informations about mounted fs
 * It should contain brut datas.
 * Later on, we would need to compute those infos:
 * unsigned long bused;	used blocks
 * double prct_used;	usage of fs in percent
 * double prct_free;	free space of fs in percent
 */
struct fsmntinfo {
	/* infos to get from getmntent(3) */
	char *fsname;	/* name of mounted file system */
	char *dir;	/* file system path prefix */
	char *type;	/* mount type */
	char *opts;	/* mount options (see mntent.h) */

	/* infos to get from statvfs(3) */
	unsigned long bsize;	/* file system block size */
	unsigned long frsize;	/* fragment size */
	unsigned long blocks;	/* size of fs in frsize unit */
	unsigned long bfree;	/* # of free blocks */
	unsigned long bavail;	/* # of available blocks */
	unsigned long files;	/* # of inodes */
	unsigned long ffree;	/* # of free inodes */
	unsigned long favail;	/* # of available inodes */

	/* pointer to the next element of the list */
	struct fsmntinfo *next;
};

/* list structure to store fsmntinfo */
struct list {
	struct fsmntinfo *head;
	struct fsmntinfo *tail;

	int fsmaxlen; /* should be the size of the longest fsname */
	int dirmaxlen; /* same for dir */
	int typemaxlen; /* same for type */
};

/* function declaration */
void init_queue(struct list *lst);
int is_empty(struct list lst);
int enqueue(struct list *lst, struct fsmntinfo elt);
struct fsmntinfo fmi_init(void);

#endif /* ndef LIST_H */
