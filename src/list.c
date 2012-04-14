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

/*
 * list.c
 *
 * Manipulate list
 */
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

/*
 * Initializes a queue structure
 * @lst: queue pointer
 */
void
init_queue(struct list *lst)
{
	lst->head = NULL;
	lst->tail = NULL;
	lst->fsmaxlen = -1;
	lst->dirmaxlen = -1;
	lst->typemaxlen = -1;
	lst->mntoptmaxlen = -1;
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
		/* NOTREACHED */
	else
		return 0;
		/* NOTREACHED */
}

/*
 * Enqueues an element into a queue
 * @lst: queue pointer
 * @elt: element
 * Returns:
 *	--> -1 on error
 *	-->  0 on sucess
 */
int
enqueue(struct list *lst, struct fsmntinfo fmi)
{
	struct fsmntinfo *new_fmi = malloc(sizeof(struct fsmntinfo));

	if (new_fmi == NULL) {
		(void)fputs("Error while allocating memory to fmi", stderr);
		return -1;
		/* NOTREACHED */
	}

	/* initialize the new element to be inserted in the queue */
	*new_fmi = fmi;

	if (is_empty((*lst)))
		lst->head = new_fmi;
	else
		lst->tail->next = new_fmi;

	lst->tail = new_fmi;

	return 0;
	/* NOTREACHED */
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

	fmi.fsname	= "unknown";
	fmi.dir		= "unknown";
	fmi.type	= "unknown";
	fmi.opts	= "none";

	fmi.bsize	= 0;
	fmi.frsize	= 0;
	fmi.blocks	= 0;
	fmi.bfree	= 0;
	fmi.bavail	= 0;
	fmi.files	= 0;
	fmi.ffree	= 0;
	fmi.favail	= 0;

	fmi.next	= NULL;

	return fmi;
	/* NOTREACHED */
}
