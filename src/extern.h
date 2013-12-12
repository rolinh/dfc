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

#ifndef H_EXTERN
#define H_EXTERN
/*
 * extern.h
 *
 * Globals and dfc version declaration
 */

#ifndef PACKAGE
#define	PACKAGE	"dfc"
#endif /* ndef PACKAGE */

#ifndef VERSION
#define	VERSION	"(unknown version)"
#endif /* ndef VERSION */

/* too ugly to use gettext in each string that needs translation... */
#ifdef NLS_ENABLED
#define _(STRING) gettext(STRING)
#else
#define _(STRING) STRING
#endif /* def NLS_ENABLED */

/* color defines */
#define BLACK	30
#define RED	31
#define GREEN	32
#define YELLOW	33
#define BLUE	34
#define MAGENTA 35
#define CYAN	36
#define WHITE	37

/* possible lengths of the graph bar */
#define GRAPHBAR_SHORT 22
#define GRAPHBAR_WIDE 52

/* html color code length (type of #FF0000, etc.) without the # */
#define HTMLCOLORCODELENGTH 6


struct conf {
	int chead;	/* color used for the header */
	int clow;	/* color when usage low */
	int cmedium;	/* color when usage medium */
	int chigh;	/* color when usage high */

	int gmedium;	/* starting value from which usage is considered medium */
	int ghigh;	/* starting value from which usage is considered high */

	char gsymbol;	/* symbol used to draw the graph */

	const char *hcheadbg;	/* background color for header and footer (html) */
	const char *hcheadfg;	/* font color used in header and footer (html) */
	const char *hccellbg;	/* background color for cells (html) */
	const char *hccellfg;	/* font color for cells (html) */
	const char *hchoverbg;	/* background color on hover (html) */
	const char *hchoverfg;	/* font color on hover (html) */
	const char *hclow;	/* color when usage is low (html) */
	const char *hcmedium;	/* color when usage is medium (html) */
	const char *hchigh;	/* color when usage is high (html) */

	char csvsep;	/* separator used for csv export */
};

struct maxwidths {
	size_t fsname;
	size_t fstype;
	size_t bar;
	size_t perctused;
	size_t used;
	size_t avail;
	size_t total;
	size_t nbinodes;
	size_t avinodes;
	size_t mountdir;
	size_t mountopt;
};

/*
 * These two variables are used when we don't need to alloc some mem for the
 * fsmntinfo struct. So we can know that and don't free the pointers
 */
extern char g_unknown_str[];
extern char g_none_str[];

/* struct to store specific configuration from config file */
extern struct conf cnf;

/* struct to store maximum required widths (useful only in text export mode) */
extern struct maxwidths max;

/* set flags for options */
extern int aflag, bflag, cflag, dflag, eflag, fflag, hflag, iflag, lflag, mflag,
    nflag, oflag, pflag, qflag, sflag, tflag, uflag, vflag, wflag;
extern int Tflag, Wflag;

/* flag that determines which unit is in use (Ko, Mo, etc.) */
extern char unitflag;

#endif /* ndef EXTERN_H */
