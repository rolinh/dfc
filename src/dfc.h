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

#ifndef H_DFC
#define H_DFC
/*
 * dfc.h
 *
 * header file for dfc.c
 */

#include "display.h"
#include "dotfile.h"
#include "export.h"
#include "extern.h"
#include "list.h"
#include "util.h"
#include "platform/services.h"

/* function declaration */
void usage(int status);
void fetch_info(struct list *lst);
void disp(struct list *lst, char *fsfilter, char *fsnfilter, struct display *sdisp);

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__) \
	|| defined(__DragonFly__)
/* avoid struct statfs declared inside parameter list warning */
struct statfs;
char * statfs_flags_to_str(struct statfs *s);
#elif defined(__NetBSD__)
struct statvfs;
char *statfs_flags_to_str(struct statvfs *s);
#endif /* __FreeBSD__ || __OpenBSD__ || __APPLE__ || __DragonFly__ || __NetBSD__ */

#endif /* ndef DFC_H */
