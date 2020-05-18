/*
 * firebee.h - FireBee specific functions
 * This is part of the FireBee driver for fVDI
 * Glued by Vincent Riviere
 * Stirred, mixed and shaken by Markus Fröschle
 */

/*
 * Copyright (C) 2016, Jason S. McMullan <jason.mcmullan@gmail.com>
 * All rights reserved.
 *
 * Licensed under the MIT License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _FIREBEE_H_
#define _FIREBEE_H_

#include "fvdi.h"
#include <stdlib.h>
#include <stdbool.h>

#define FIREBEE_VRAM_PHYS_OFFSET    0x00000000      /* physical offset FPGA RAM to ST RAM start (0x0) */

#define FBEE_VIDEO_PLL_CONFIG   (volatile unsigned long *) 0xff000600
#define FBEE_VIDEO_PLL_CLK      (volatile unsigned long *) 0xff000604
#define FBEE_VIDEO_PLL_RECONFIG (volatile unsigned long *) 0xff000800

#define FBEE_VIDEO_ON       0x00000001
#define FBEE_VDAC_ON        0x00000002
#define FBEE_COLOR_24       0x00000004
#define FBEE_COLOR_16       0x00000008
#define FBEE_COLOR8         0x00000010
#define FBEE_COLOR1         0x00000020
#define FBEE_FALCON_SHIFT   0x00000040
#define FBEE_ST_SHIFT_MODE  0x00000080
#define FBEE_CLK_25         0x00000000
#define FBEE_CLK_33         0x00000100
#define FBEE_CLK_PLL        0x00000200

#define FBEE_SYNC           0x00008000
#define FBEE_VCKE           0x00010000
#define FBEE_VCS            0x00020000
#define FBEE_REFRESH_ON     0x00040000
#define FBEE_CONFIG_ON      0x00080000
#define FBEE_FIFO_ON        0x01000000
#define FBEE_BORDER_ON      0x02000000

#define GMB_DOUBLECLOCK         0       /* Clock is doubled after selection */
#define GMB_INTERLACE           1       /* Mode is interlaced */
#define GMB_DOUBLESCAN          2       /* Scanlines are doubled */
#define GMB_HPOLARITY           3       /* Horizontal Polarity */
#define GMB_VPOLARITY           4       /* Vertical Polarity */
#define GMB_COMPATVIDEO         5       /* Compatible Video */
#define GMB_DOUBLEVERTICAL      6       /* Doubled Vertical */

#define GMF_DOUBLECLOCK         (1UL << GMB_DOUBLECLOCK)
#define GMF_INTERLACE           (1UL << GMB_INTERLACE)
#define GMF_DOUBLESCAN          (1UL << GMB_DOUBLESCAN)
#define GMF_HPOLARITY           (1UL << GMB_HPOLARITY)
#define GMF_VPOLARITY           (1UL << GMB_VPOLARITY)
#define GMF_COMPATVIDEO         (1UL << GMB_COMPATVIDEO)
#define GMF_DOUBLEVERTICAL      (1UL << GMB_DOUBLEVERTICAL)

/* from firebee.c */
void fbee_set_clock(int clock);
void fbee_set_screen(void *addr);
void fbee_set_panning(unsigned short *mem);

long CDECL c_write_pixel(Virtual *vwk, MFDB *mfdb, long x, long y, long colour);
long CDECL c_read_pixel(Virtual *vwk, MFDB *mfdb, long x, long y);
long CDECL c_line_draw(Virtual *vwk, long x1, long y1, long x2, long y2, long pattern, long colour, long mode);
long CDECL c_expand_area(Virtual *vwk, MFDB *src, long src_x, long src_y, MFDB *dst, long dst_x, long dst_y, long w, long h, long operation, long colour);
long CDECL c_fill_area(Virtual *vwk, long x, long y, long w, long h, short *pattern, long colour, long mode, long interior_style);
long CDECL c_fill_polygon(Virtual *vwk, short points[], long n, short index[], long moves, short *pattern, long colour, long mode, long interior_style);
long CDECL c_blit_area(Virtual *vwk, MFDB *src, long src_x, long src_y, MFDB *dst, long dst_x, long dst_y, long w, long h, long operation);
long CDECL c_text_area(Virtual *vwk, short *text, long length, long dst_x, long dst_y, short *offsets);
long CDECL c_mouse_draw(Workstation *wk, long x, long y, Mouse *mouse);

long CDECL c_get_colour(Virtual *vwk, long colour);
void CDECL c_set_colours(Virtual *vwk, long start, long entries, unsigned short *requested, Colour palette[]);

#endif /* _FIREBEE_H_ */
