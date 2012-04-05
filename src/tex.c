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
 * tex.c
 *
 * TeX display functions
 */
#include "tex.h"
#include "extern.h"

void
init_disp_tex(struct Display *disp)
{
    disp->print_header = tex_disp_header;
    disp->print_sum    = tex_disp_sum;
    disp->print_bar    = tex_disp_bar;
    disp->print_at     = tex_disp_at;
    disp->print_perct  = tex_disp_perct;
    disp->change_color = tex_change_color;
    disp->reset_color  = tex_reset_color;
}

void
tex_disp_header(struct list *lst)
{
    /* TODO. */
}

void
tex_disp_sum(struct list *lst, double stot, double utot, double ftot,
             double ifitot, double ifatot)
{
    /* TODO. */
}

void
tex_disp_bar(double perct)
{
    /* TODO. */
}

void
tex_disp_at(double n, double perct)
{
    /* TODO. */
}

void
tex_disp_perct(double perct)
{
    /* TODO. */
}

void
tex_change_color(double perct)
{
    /* TODO. */
}

void
tex_reset_color(void)
{
    /* TODO. */
}
