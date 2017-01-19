/*
 * Copyright (c) 2012-2017, Robin Hahling
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
 * list.c
 *
 * Manipulate list
 */
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "extern.h"

/*
 * Initializes a queue structure
 * @lst: queue pointer
 */
void
init_queue(struct list *lst)
{
	lst->head  = NULL;
	lst->tail   = NULL;
}

/*
 * Checks if a queue is empty
 * @lst: queue
 * Returns:
 *	--> 1 (true) if the queue is empty
 *	--> 0 if not
 */
int
is_empty(struct list lst)
{
	if (lst.head == NULL)
		return 1;
	else
		return 0;
}

/*
 * Enqueues an element into a queue
 * @lst: queue pointer
 * @elt: element
 * Returns:
 *	--> -1 on error
 *	-->  0 on success
 */
int
enqueue(struct list *lst, struct fsmntinfo fmi)
{
	struct fsmntinfo *new_fmi = malloc(sizeof(struct fsmntinfo));

	if (new_fmi == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		return -1;
	}

	/* initialize the new element to be inserted in the queue */
	*new_fmi = fmi;

	if (is_empty((*lst)))
		lst->head = new_fmi;
	else
		lst->tail->next = new_fmi;

	lst->tail = new_fmi;

	return 0;
}

/*
 * Inits an fsmntinfo to some defaults values
 * Returns:
 *	--> fsmntinfo that has been initialized
 */
struct fsmntinfo
fmi_init(void)
{
	struct fsmntinfo fmi;

	fmi.fsname  = g_unknown_str;
	fmi.fstype  = g_unknown_str;
	fmi.mntdir  = g_unknown_str;
	fmi.mntopts = g_none_str;

	fmi.perctused = 0.0;
	fmi.total     = 0.0;
	fmi.avail     = 0.0;
	fmi.used      = 0.0;

	fmi.flags  = 0;
	fmi.bsize  = 0;
	fmi.frsize = 0;
	fmi.blocks = 0;
	fmi.bfree  = 0;
	fmi.bavail = 0;
	fmi.files  = 0;
	fmi.ffree  = 0;
	fmi.favail = 0;

	fmi.next   = NULL;

	return fmi;
}

struct fsmntinfo
*delete_struct_and_get_next(struct fsmntinfo *p)
{
	struct fsmntinfo *next;
	if(p == NULL) /* sanity check, we never know */
		return NULL;

	next = p->next;

	if(p->fsname != g_unknown_str) /* we malloc'd a string */
		free(p->fsname);
	if(p->fstype != g_unknown_str)
		free(p->fstype);
	if(p->mntdir != g_unknown_str)
		free(p->mntdir);
	if(p->mntopts != g_none_str)
		free(p->mntopts);

	free(p);

	return next;
}
