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

#ifndef DFC_H
#define DFC_H
/*
 * dfc.h
 *
 * header file for dfc.c
 */

#include "list.h"

#define VERSION "2.4.0"

/* function declaration */
static void usage(int status);
void fetch_info(struct list *lst);
void disp(struct list *lst, char *fsfilter);
void disp_header(struct list *lst);
void disp_sum(struct list *lst, double stot, double utot, double ftot,
		double ifitot, double ifatot);
void disp_bar(double perct);
void disp_at(double n, double perct);
void disp_perct(double perct);
void change_color(double perct);
void reset_color(void);
double cvrt(double n);
void humanize(double n, double perct);
int imax(int a, int b);
char* trk(char *str);
char* shortenstr(char *str, int len);
unsigned int getttywidth(void);

#endif /* ndef DFC_H */
