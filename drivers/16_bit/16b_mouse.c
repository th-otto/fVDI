/*
 * Bitplane mouse routine
 *
 * $Id: mouse.c,v 1.4 2006-05-26 07:56:20 johan Exp $
 *
 * Copyright 2006, Johan Klockars 
 * Copyright 2002 The EmuTOS development team
 * Copyright 1999 by Caldera, Inc. and Authors:
 * Copyright 1982 by Digital Research Inc.
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "driver.h"
#include "../bitplane/bitplane.h"

#define PIXEL		short
#define PIXEL_SIZE	sizeof(PIXEL)


static void set_mouse_shape(Mouse *mouse, unsigned short *masks)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		*masks++ = mouse->mask[i];
		*masks++ = mouse->data[i];
	}
}


long CDECL c_mouse_draw(Workstation *wk, long x, long y, Mouse *mouse)
{
	unsigned long state;
	long mouseparm = (long) mouse;
	static long old_colours = 0;
	static PIXEL foreground = 0xffff;
	static PIXEL background = 0;
	static unsigned long save_state = 0;

	static unsigned short mouse_data[16 * 2] = {
		0xffff, 0x0000, 0x7ffe, 0x3ffc, 0x3ffc, 0x1ff8, 0x1ff8, 0x0ff0,
		0x0ff0, 0x07e0, 0x07e0, 0x03c0, 0x03c0, 0x0180, 0x0180, 0x0000
	};
	static PIXEL saved[16 * 16];

	if (mouseparm > 7)
	{									/* New mouse shape */
		long *pp = (long *)&wk->mouse.colour;
		
		pp = (long *)&wk->mouse.colour;
		if (*pp != old_colours)
		{
			Colour *global_palette;
			PIXEL *realp;
			
			old_colours = *pp;
			/* c_get_colours(wk, *pp, &foreground, &background); */
			global_palette = wk->screen.palette.colours;
			realp = (PIXEL *)&global_palette[wk->mouse.colour.foreground].real;
			foreground = *realp;
			realp = (PIXEL *)&global_palette[wk->mouse.colour.background].real;
			background = *realp;
		}

		if (!fix_shape)
			set_mouse_shape(mouse, mouse_data);
		return 0;
	}

	/* Need to mask x since it contains old operation in high bits (use that?) */
	x &= 0xffffL;

	state = save_state;
	if (state && !no_restore && (mouseparm == 0 || mouseparm == 2 || mouseparm == 4))
	{									/* Move or Hide */
		PIXEL *dst;
		PIXEL *save_w;
		short i, w, h;
		unsigned long wrap;

		dst = (PIXEL *) ((long) wk->screen.mfdb.address + (state & 0x00ffffffL));

		w = (state >> 28) & 0x0f;
		h = (state >> 24) & 0x0f;
		save_w = saved;
		wrap = wk->screen.wrap - (w + 1) * PIXEL_SIZE;
		wrap /= PIXEL_SIZE;		/* Change into pixel count */

		do
		{
			i = w;
			do
			{
				*dst++ = *save_w++;
			} while (--i >= 0);
			dst += wrap;
		} while (--h >= 0);

		save_state = 0;
	}

	if (mouseparm == 0 || mouseparm == 3)
	{									/* Move or Show */
		PIXEL *dst;
		PIXEL *save_w;
		const unsigned short *mask_start;
		short w, h, shft;
		unsigned long wrap;

		x -= wk->mouse.hotspot.x;
		y -= wk->mouse.hotspot.y;
		w = 16;
		h = 16;

		mask_start = mouse_data;
		if (y < wk->screen.coordinates.min_y)
		{
			short ys;

			ys = wk->screen.coordinates.min_y - y;
			h -= ys;
			y = wk->screen.coordinates.min_y;
			mask_start += ys << 1;
		}
		if (y + h - 1 > wk->screen.coordinates.max_y)
		{
			h = wk->screen.coordinates.max_y - y + 1;
		}
		
		shft = 0;

		if (x < wk->screen.coordinates.min_x)
		{
			short xs;

			xs = wk->screen.coordinates.min_x - x;
			w -= xs;
			x = wk->screen.coordinates.min_x;
			shft = xs;
		}
		if (x + w - 1 > wk->screen.coordinates.max_x)
		{
			w = wk->screen.coordinates.max_x - x + 1;
		}

		if (w <= 0 || h <= 0)
		{
			save_state = 0;
			return 0;
		}
		
		wrap = wk->screen.wrap - w * PIXEL_SIZE;
		wrap /= PIXEL_SIZE;		/* Change into pixel count */
		dst = (PIXEL *) ((long) wk->screen.mfdb.address + y * (long) wk->screen.wrap + x * PIXEL_SIZE);

		w--;
		h--;
		state = 0;
		state |= (long) (w & 0x0f) << 28;
		state |= (long) (h & 0x0f) << 24;
		state |= (long) dst - (long) wk->screen.mfdb.address;
		save_state = state;

		save_w = saved;

		{
			unsigned short fg, bg;
			short i;
			
			do
			{
				bg = *mask_start++;
				bg <<= shft;
				fg = *mask_start++;
				fg <<= shft;
				i = w;
				do
				{
					*save_w++ = *dst;
					if (fg & 0x8000)
						*dst = foreground;
					else if (bg & 0x8000)
						*dst = background;
					dst++;
					bg <<= 1;
					fg <<= 1;
				} while (--i >= 0);
				dst += wrap;
			} while (--h >= 0);
		}
	}

	return 0;
}
