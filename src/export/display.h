/*
 * Copyright (c) 2012-2016, Robin Hahling
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

#ifndef H_DISPLAY
#define H_DISPLAY

#include <sys/types.h>
#include <inttypes.h>

/* Structure to handle the display interface. */
struct display
{
	/* Some interfaces may need to initialize/deinitialize stuff */
	void (*init)         (void);
	void (*deinit)       (void);

	void (*print_header) (void);
	void (*print_sum)    (double, double, double, double, double);
	void (*print_bar)    (double);
	void (*print_used)   (double, double, int);
	void (*print_avail)  (double, double, int);
	void (*print_total)  (double, double, int);
	void (*print_fs)     (const char *);
	void (*print_type)   (const char *);
	void (*print_inodes) (uint64_t, uint64_t);
	void (*print_mount)  (const char *);
	void (*print_mopt)   (const char *);
	void (*print_perct)  (double);
	void (*print_ln_end) (void);
};

#endif /* ndef H_DISPLAY */
