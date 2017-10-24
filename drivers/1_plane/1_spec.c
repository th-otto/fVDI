/*
 * fVDI device driver specific setup
 *
 * $Id: 1_spec.c,v 1.4 2005-12-06 08:06:27 johan Exp $
 *
 * Copyright 1998-2002, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "relocate.h"

#include "driver.h"

#if 1
 #define FAST		/* Write in FastRAM buffer */
 #define BOTH		/* Write in both FastRAM and on screen */
#else
 #undef FAST
 #undef BOTH
#endif


char red[] = {6};
char green[] = {6};
char blue[] = {6};
char none[] = {0};

Mode mode[1] =
	{{1, CHECK_PREVIOUS, {red, green, blue, none, none, none}, 0, 0, 1, 1}};

extern Device device;

char driver_name[] = "Monochrome (shadow)";

void *write_pixel_r = write_pixel;
void *read_pixel_r  = read_pixel;
void *line_draw_r   = line_draw;
void *expand_area_r = expand_area;
void *fill_area_r   = fill_area;
void *fill_poly_r   = 0;
void *blit_area_r   = blit_area;
void *text_area_r   = text_area;
void *mouse_draw_r  = mouse_draw;
long CDECL (*get_colour_r)(Virtual *vwk, long colour) = get_colour;
void CDECL (*get_colours_r)(Virtual *vwk, long colour, long *foreground, long *background) = 0;
void CDECL (*set_colours_r)(Virtual *vwk, long start, long entries, unsigned short *requested, Colour palette[]) = set_colours;

long wk_extend = 0;

short accel_s = A_MOUSE | A_SET_PAL | A_GET_COL | A_SET_PIX | A_GET_PIX |
                A_TEXT | A_BLIT | A_FILL | A_EXPAND | A_LINE;
short accel_c = 0;

#if 0
short graphics_mode = CHECK_PREVIOUS;
#else
Mode *graphics_mode = &mode[0];
#endif

short colour_bits = 18;

int shadow = 0;

short debug = 0;

short depth = 0;


/*
 * Handle any driver specific parameters
 */
void check_token(char *token, const char **ptr)
{
	if (access->funcs.equal(token, "shadow"))
		shadow = 1;
}


/*
 * Do whatever setup work might be necessary on boot up
 * and which couldn't be done directly while loading.
 * Supplied is the default fVDI virtual workstation.
 */
long initialize(Virtual *vwk)
{
	Workstation *wk;
	char *buf;
	int fast_w_bytes;
#if 0
	int i;
#endif

	debug = access->funcs.misc(0, 1, 0);
	
	vwk = me->default_vwk;	/* This is what we're interested in */
	wk = vwk->real_address;

#ifdef FAST
	if (shadow) {
 #if 0			/* It's not clear that this is a good idea */
		fast_w_bytes = (wk->screen.wrap + 15) & 0xfffffff0;
 #else
		fast_w_bytes = wk->screen.wrap;
 #endif
		buf = (char *)access->funcs.malloc(fast_w_bytes * wk->screen.mfdb.height + 255, 1);
		if (buf) {
			wk->screen.shadow.buffer = buf;
			wk->screen.shadow.address = (void *)(((long)buf + 255) & 0xffffff00);
			wk->screen.shadow.wrap = fast_w_bytes;
		} else {
			access->funcs.error("Can't allocate FastRAM!", 0);
			wk->screen.shadow.buffer = 0;
			wk->screen.shadow.address = 0;
		}
 #ifndef BOTH
		wk->screen.mfdb.address = wk->screen.shadow.address;
 #endif
	}
#endif
	if (!wk->screen.shadow.address)
		driver_name[10] = 0;

        if (wk->screen.pixel.width > 0)        /* Starts out as screen width */
                wk->screen.pixel.width = (wk->screen.pixel.width * 1000L) / wk->screen.mfdb.width;
        else                                   /*   or fixed DPI (negative) */
                wk->screen.pixel.width = 25400 / -wk->screen.pixel.width;
        if (wk->screen.pixel.height > 0)        /* Starts out as screen height */
                wk->screen.pixel.height = (wk->screen.pixel.height * 1000L) / wk->screen.mfdb.height;
        else                                    /*   or fixed DPI (negative) */
                wk->screen.pixel.height = 25400 / -wk->screen.pixel.height;

	device.byte_width = wk->screen.wrap;
	device.address = wk->screen.mfdb.address;

	return 1;
}

/*
 *
 */
long setup(long type, long value)
{
	long ret;

	ret = -1;
	switch(type) {
#if 0
	case S_SCREEN:
		if (value != -1)
			old_screen = value;
		ret = old_screen;
		break;
	case S_AESBUF:
		if (value != -1)
			aes_buffer = value;
		ret = aes_buffer;
		break;
	case S_DOBLIT:
		if (value != -1)
			blit_fall_back = value;
		ret = blit_fall_back;
		break;
	case S_CACHEIMG:
		if (value != -1)
			cache_img = value;
		ret = cache_img;
		break;
#endif
	case Q_NAME:
		ret = (long)driver_name;
		break;
	}

	return ret;
}

/*
 * Initialize according to parameters (boot and sent).
 * Create new (or use old) Workstation and default Virtual.
 * Supplied is the default fVDI virtual workstation.
 */
Virtual *opnwk(Virtual *vwk)
{
	return 0;
}

/*
 * 'Deinitialize'
 */
void clswk(Virtual *vwk)
{
}
