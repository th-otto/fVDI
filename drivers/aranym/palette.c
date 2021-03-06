/*
 *      16 bit colour index handling
 */
#include "fvdi.h"
#include "relocate.h"
#include "driver.h"
#include "aranym.h"


#define ECLIPSE 0
#define NOVA 0		/* 1 - byte swap 16 bit colour value (NOVA etc) */

#undef NORMAL_NAME

#ifdef NORMAL_NAME
long CDECL c_get_colour(Virtual *vwk, long colour)
#else
long CDECL GET1NAME(Virtual *vwk, long colour)
#endif
{
	Colour *local_palette, *global_palette;
	Colour *fore_pal, *back_pal;
	unsigned long tc_word, tc_word2;
	PIXEL *pp;

	local_palette = vwk->palette;
	if (local_palette && !((long) local_palette & 1))	/* Complete local palette? */
		fore_pal = back_pal = local_palette;
	else
	{
	    /* Global or only negative local */
		local_palette = (Colour *) ((long) local_palette & 0xfffffffeL);
		global_palette = vwk->real_address->screen.palette.colours;
		if (local_palette && ((short) colour < 0))
			fore_pal = local_palette;
		else
			fore_pal = global_palette;
		if (local_palette && ((colour >> 16) < 0))
			back_pal = local_palette;
		else
			back_pal = global_palette;
	}

	pp = (PIXEL *) &fore_pal[(short) colour].real;
	tc_word = *pp;
	pp = (PIXEL *) &back_pal[colour >> 16].real;
	tc_word2 = *pp;

#if NOVA
	switch (sizeof(PIXEL))
	{
	case 2:
		tc_word = ((tc_word & 0x000000ffL) << 8) | ((tc_word & 0x0000ff00L) >> 8);
		tc_word2 = ((tc_word2 & 0x000000ffL) << 8) | ((tc_word2 & 0x0000ff00L) >> 8);
		break;
	default:
		tc_word = ((tc_word & 0x000000ffL) << 24) | ((tc_word & 0x0000ff00L) << 8) |
		          ((tc_word & 0x00ff0000L) >> 8) | ((tc_word & 0xff000000L) >> 24);
		break;
	}
#endif

	if (sizeof(PIXEL) > 2)
		return tc_word;
	else
		return (tc_word2 << 16) | tc_word;
}


#ifdef NORMAL_NAME
void CDECL c_get_colours(Virtual *vwk, long colour, unsigned long *foreground, unsigned long *background)
#else
void CDECL GETNAME(Virtual *vwk, long colour, unsigned long *foreground, unsigned long *background)
#endif
{
	Colour *local_palette, *global_palette;
	Colour *fore_pal, *back_pal;
	unsigned long tc_word;
	PIXEL *pp;

	local_palette = vwk->palette;
	if (local_palette && !((long) local_palette & 1))	/* Complete local palette? */
		fore_pal = back_pal = local_palette;
	else
	{
	    /* Global or only negative local */
		local_palette = (Colour *) ((long) local_palette & 0xfffffffeL);
		global_palette = vwk->real_address->screen.palette.colours;
		if (local_palette && ((short) colour < 0))
			fore_pal = local_palette;
		else
			fore_pal = global_palette;
		if (local_palette && ((colour >> 16) < 0))
			back_pal = local_palette;
		else
			back_pal = global_palette;
	}

	pp = (PIXEL *) &fore_pal[(short) colour].real;
	tc_word = *pp;

#if NOVA
	switch (sizeof(PIXEL))
	{
	case 2:
		tc_word = ((tc_word & 0x000000ffL) << 8) | ((tc_word & 0x0000ff00L) >> 8);
		break;
	default:
		tc_word = ((tc_word & 0x000000ffL) << 24) | ((tc_word & 0x0000ff00L) << 8) |
		          ((tc_word & 0x00ff0000L) >> 8) | ((tc_word & 0xff000000L) >> 24);
		break;
	}
#endif

	*foreground = tc_word;
	pp = (PIXEL *) &back_pal[colour >> 16].real;
	tc_word = *pp;

#if NOVA
	switch (sizeof(PIXEL))
	{
	case 2:
		tc_word = ((tc_word & 0x000000ffL) << 8) | ((tc_word & 0x0000ff00L) >> 8);
		break;
	default:
		tc_word = ((tc_word & 0x000000ffL) << 24) | ((tc_word & 0x0000ff00L) << 8) |
		          ((tc_word & 0x00ff0000L) >> 8) | ((tc_word & 0xff000000L) >> 24);
		break;
	}
#endif

	*background = tc_word;
}


#ifdef NORMAL_NAME
void CDECL c_set_colours(Virtual *vwk, long start, long entries, unsigned short *requested, Colour palette[])
#else
void CDECL SETNAME(Virtual *vwk, long start, long entries, unsigned short *requested, Colour palette[])
#endif
{
	unsigned long colour;
	unsigned short component;
	unsigned long tc_word;
	int i;
	PIXEL *pp;
	
	(void) vwk;
	if (((long) requested) & 1)
	{
	    /* New entries? */
		requested = (unsigned short *) (((long) requested) & 0xfffffffeL);
		for (i = 0; i < entries; i++)
		{
			requested++;				/* First word is reserved */
			component = *requested++ >> 8;
			palette[start + i].vdi.red = (component * 1000L) / 255;
			palette[start + i].hw.red = component;
			component = *requested++ >> 8;
			palette[start + i].vdi.green = (component * 1000L) / 255;
			palette[start + i].hw.green = component;
			component = *requested++ >> 8;
			palette[start + i].vdi.blue = (component * 1000L) / 255;
			palette[start + i].hw.blue = component;

			if (debug > 1)
			{
				PRINTF(("[%ld] %d,%d,%d  $%x,$%x,$%x\n", start + i,
					palette[start + i].vdi.red & 0xffff,
					palette[start + i].vdi.green & 0xffff,
					palette[start + i].vdi.blue & 0xffff,
					palette[start + i].hw.red & 0xffff,
					palette[start + i].hw.green & 0xffff,
					palette[start + i].hw.blue & 0xffff));
			}

			/* Would likely be better to have a different mode for this */
			c_get_hw_colour(start + i,
					palette[start + i].vdi.red,
					palette[start + i].vdi.green,
					palette[start + i].vdi.blue,
					&tc_word);

			pp = (PIXEL *)&palette[start + i].real;
			*pp = (PIXEL) tc_word;
		}
	} else
	{
		for (i = 0; i < entries; i++)
		{
			component = *requested++;
			palette[start + i].vdi.red = component;
			palette[start + i].hw.red = component;	/* Not at all correct */
			colour = (component * ((1L << red_bits) - 1) + 500L) / 1000;
			palette[start + i].hw.red = (colour * 1000 + (1L << (red_bits - 1))) / ((1L << red_bits) - 1);
			component = *requested++;
			palette[start + i].vdi.green = component;
			palette[start + i].hw.green = component;	/* Not at all correct */
			colour = (component * ((1L << green_bits) - 1) + 500L) / 1000;
			palette[start + i].hw.green = (colour * 1000 + (1L << (green_bits - 1))) / ((1L << green_bits) - 1);
			component = *requested++;
			palette[start + i].vdi.blue = component;
			palette[start + i].hw.blue = component;	/* Not at all correct */
			colour = (component * ((1L << blue_bits) - 1) + 500L) / 1000;
			palette[start + i].hw.blue = (colour * 1000 + (1L << (blue_bits - 1))) / ((1L << blue_bits) - 1);

			if (debug > 1)
			{
				PRINTF(("[%ld] %d,%d,%d  $%x,$%x,$%x\n", start + i,
					palette[start + i].vdi.red & 0xffff,
					palette[start + i].vdi.green & 0xffff,
					palette[start + i].vdi.blue & 0xffff,
					palette[start + i].hw.red & 0xffff,
					palette[start + i].hw.green & 0xffff,
					palette[start + i].hw.blue & 0xffff));
			}

			c_get_hw_colour(start + i,
					palette[start + i].vdi.red,
					palette[start + i].vdi.green,
					palette[start + i].vdi.blue,
					&tc_word);

			pp = (PIXEL *)&palette[start + i].real;
			*pp  = (PIXEL) tc_word;
		}
	}
}
