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

#ifndef H_UTIL
#define H_UTIL
/*
 * util.h
 *
 * Util functions
 */
#include <inttypes.h>

#include "extern.h"
#include "list.h"

/* function declaration */
int imax(int a, int b);
char * strtrim(char *str);
char * shortenstr(char *str, int len);
char * sanitizestr(const char *str);
int getttywidth(void);
int humanize(double *n);
int humanize_i(uint64_t *n);
void print_unit(int i, int mode);
double cvrt(double n);
int fstypefilter(char *type, char *filter, int nm);
int fsnamefilter(char *fsname, char *filter, int nm);
int is_remote(char *fstype);
int cmp(struct fsmntinfo *a, struct fsmntinfo *b);
struct fsmntinfo * msort(struct fsmntinfo *fmi);
void auto_adjust(struct list lst, int width);
int req_width(struct list lst);
char * fetchdate(void);
char * colortostr(int color);
int colortoint(char *col);
int chk_html_colorcode(char *color);

#endif /* ndef UTIL_H */
