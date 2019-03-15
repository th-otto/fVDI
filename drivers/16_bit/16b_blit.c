/* 
 * A 16 bit graphics blit routine, by Johan Klockars.
 *
 * This file is an example of how to write an
 * fVDI device driver routine in C.
 *
 * You are encouraged to use this file as a starting point
 * for other accelerated features, or even for supporting
 * other graphics modes. This file is therefore put in the
 * public domain. It's not copyrighted or under any sort
 * of license.
 */

#include "fvdi.h"
#include "driver.h"
#include "../bitplane/bitplane.h"

#define PIXEL		short
#define PIXEL_SIZE	sizeof(PIXEL)
#define PIXEL_32    long

#ifdef FVDI_DEBUG
static void debug_out(const char *text1, int w, int old_w, int h, int src_x, int src_y, int dst_x, int dst_y)
{
	PRINTF(("%s%d", w));
	if (old_w > 0) {
		PRINTF(("(%d))", old_w));
	}
	PRINTF((",%d from %d,%d to %d,%d\n", h, src_x, src_y, dst_x, dst_y));
}
#endif


#define DO_OP(v) \
	switch(operation) { \
	case 0: \
	default: \
		v = 0; \
		break; \
	case 1: \
		v = v ## s & v ## d; \
		break; \
	case 2: \
		v = v ## s & ~v ## d; \
		break; \
	case 3: \
		v = v ## s; \
		break; \
	case 4: \
		v = ~v ## s & v ## d; \
		break; \
	case 5: \
		v = v ## d; \
		break; \
	case 6: \
		v = v ## s ^ v ## d; \
		break; \
	case 7: \
		v = v ## s | v ## d; \
		break; \
	case 8: \
		v = ~(v ## s | v ## d); \
		break; \
	case 9: \
		v = ~(v ## s ^ v ## d); \
		break; \
	case 10: \
		v = ~v ## d; \
		break; \
	case 11: \
		v = v ## s | ~v ## d; \
		break; \
	case 12: \
		v = ~v ## s; \
		break; \
	case 13: \
		v = ~v ## s | v ## d; \
		break; \
	case 14: \
		v = ~(v ## s & v ## d); \
		break; \
	case 15: \
		v = -1; \
		break; \
	}


/*
 * Make it as easy as possible for the C compiler.
 * The current code is written to produce reasonable results with Lattice C.
 * (long integers, optimize: [x xx] time)
 * - One function for each operation -> more free registers
 * - 'int' is the default type
 * - some compilers aren't very smart when it comes to *, / and %
 * - some compilers can't deal well with *var++ constructs
 */

#ifdef BOTH

static void s_blit_copy(PIXEL *src_addr, int src_line_add,
	PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
	short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
	PIXEL v;
	PIXEL_32 v32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%2)+,%3\n" \
			" move.w %3,(%0)+\n" \
			" move.w %3,(%1)+\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%2)+,%3\n" \
			" move.l %3,(%0)+\n" \
			" move.l %3,(%1)+\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%1)+,(%0)+\n" \
			: "=a"(dst_addr), "=a"(src_addr) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%1)+,(%0)+\n" \
			: "=a"(dst_addr32), "=a"(src_addr32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void s_blit_or(PIXEL *src_addr, int src_line_add,
        PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
        short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
	PIXEL v;
	PIXEL_32 v32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%2)+,%3\n" \
			" or.w (%1),%3\n" \
			" move.w %3,(%0)+\n" \
			" move.w %3,(%1)+\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%2)+,%3\n" \
			" or.l (%1),%3\n" \
			" move.l %3,(%0)+\n" \
			" move.l %3,(%1)+\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%1)+,%2\n" \
			" or.w %2,(%0)+\n" \
			: "=a"(dst_addr), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%1)+,%2\n" \
			" or.l %2,(%0)+\n" \
			: "=a"(dst_addr32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void s_blit(PIXEL *src_addr, int src_line_add,
     PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
     short int w, short int h, short int operation)
{
	short int i, j;
	PIXEL v, vs, vd;
	PIXEL_32 v32, v32s, v32d;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;
	/* Tell gcc that this cannot happen (already checked in c_blit_area() below) */
	if (w <= 0 || h <= 0)
		unreachable();
	for (i = h - 1; i >= 0; i--)
	{
		if (w & 1)
		{
			vs = *src_addr++;
#ifdef BOTH
			vd = *dst_addr_fast;
#else
			vd = *dst_addr;
#endif
			DO_OP(v);
#ifdef BOTH
			*dst_addr_fast++ = v;
#endif
			*dst_addr++ = v;
		}
		src_addr32 = (PIXEL_32 *)src_addr;
		dst_addr32 = (PIXEL_32 *)dst_addr;
#ifdef BOTH
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast;
#endif
		for(j = (w >> 1) - 1; j >= 0; j--) {
			v32s = *src_addr32++;
#ifdef BOTH
			v32d = *dst_addr_fast32;
#else
			v32d = *dst_addr32;
#endif
			DO_OP(v32);
#ifdef BOTH
			*dst_addr_fast32++ = v32;
#endif
			*dst_addr32++ = v32;
		}
		src_addr = (PIXEL *)src_addr32;
		dst_addr = (PIXEL *)dst_addr32;
		src_addr += src_line_add;
		dst_addr += dst_line_add;
#ifdef BOTH
		dst_addr_fast = (PIXEL *)dst_addr_fast32;
		dst_addr_fast += dst_line_add;
#endif
	}
}


static void
s_pan_backwards_copy(PIXEL *src_addr, int src_line_add,
                     PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
                     short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
	PIXEL v;
	PIXEL_32 v32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%2),%3\n" \
			" move.w %3,-(%0)\n" \
			" move.w %3,-(%1)\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%2),%3\n" \
			" move.l %3,-(%0)\n" \
			" move.l %3,-(%1)\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%1),-(%0)\n" \
			: "=a"(dst_addr), "=a"(src_addr) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%1),-(%0)\n" \
			: "=a"(dst_addr32), "=a"(src_addr32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void
s_pan_backwards_or(PIXEL *src_addr, int src_line_add,
                   PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
                   short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
	PIXEL v;
	PIXEL_32 v32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%2),%3\n" \
			" or.w -(%1),%3\n" \
			" move.w %3,(%1)\n" \
			" move.w %3,-(%0)\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%2),%3\n" \
			" or.l -(%1),%3\n" \
			" move.l %3,-(%1)\n" \
			" move.l %3,-(%0)\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%1),%2\n" \
			" or.w %2,-(%0)\n" \
			: "=a"(dst_addr), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%1),%2\n" \
			" or.l %2,-(%0)\n" \
			: "=a"(dst_addr32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void
s_pan_backwards(PIXEL *src_addr, int src_line_add,
                PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
                short int w, short int h, short int operation)
{
	short int i, j;
	PIXEL v, vs, vd;
	PIXEL_32 v32, v32s, v32d;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif
	
	(void) dst_addr_fast;
	/* Tell gcc that this cannot happen (already checked in c_blit_area() below) */
	if (w <= 0 || h <= 0)
		unreachable();
	for (i = h - 1; i >= 0; i--)
	{
		if (w & 1)
		{
			vs = *--src_addr;
#ifdef BOTH
			vd = *--dst_addr_fast;
#else
			vd = *--dst_addr;
#endif
			DO_OP(v);
#ifdef BOTH
			*dst_addr_fast = v;
#endif
			*dst_addr = v;
		}
		src_addr32 = (PIXEL_32 *)src_addr;
		dst_addr32 = (PIXEL_32 *)dst_addr;
#ifdef BOTH
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast;
#endif
		for (j = (w >> 1) - 1; j >= 0; j--)
		{
			v32s = *--src_addr;
#ifdef BOTH
			v32d = *--dst_addr_fast32;
#else
			v32d = *--dst_addr32;
#endif
			DO_OP(v32);
#ifdef BOTH
			*dst_addr_fast32 = v32;
#endif
			*dst_addr32 = v32;
		}
		src_addr = (PIXEL *)src_addr32;
		dst_addr = (PIXEL *)dst_addr32;
		src_addr += src_line_add;
		dst_addr += dst_line_add;
#ifdef BOTH
		dst_addr_fast = (PIXEL *)dst_addr_fast32;
		dst_addr_fast += dst_line_add;
#endif
	}
}

#define BOTH_WAS_ON
#endif
#undef BOTH

/*
 * The functions below are exact copies of those above.
 * The '#undef BOTH' makes sure that this works as it should
 * when no shadow buffer is available
 */

static void blit_copy(PIXEL *src_addr, int src_line_add,
	PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
	short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
	PIXEL v;
	PIXEL_32 v32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%2)+,%3\n" \
			" move.w %3,(%0)+\n" \
			" move.w %3,(%1)+\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%2)+,%3\n" \
			" move.l %3,(%0)+\n" \
			" move.l %3,(%1)+\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%1)+,(%0)+\n" \
			: "=a"(dst_addr), "=a"(src_addr) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%1)+,(%0)+\n" \
			: "=a"(dst_addr32), "=a"(src_addr32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void blit_or(PIXEL *src_addr, int src_line_add,
        PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
        short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
	PIXEL v;
	PIXEL_32 v32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%2)+,%3\n" \
			" or.w (%1),%3\n" \
			" move.w %3,(%0)+\n" \
			" move.w %3,(%1)+\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%2)+,%3\n" \
			" or.l (%1),%3\n" \
			" move.l %3,(%0)+\n" \
			" move.l %3,(%1)+\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w (%1)+,%2\n" \
			" or.w %2,(%0)+\n" \
			: "=a"(dst_addr), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l (%1)+,%2\n" \
			" or.l %2,(%0)+\n" \
			: "=a"(dst_addr32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void blit_16b(PIXEL *src_addr, int src_line_add,
     PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
     short int w, short int h, short int operation)
{
	short int i, j;
	PIXEL v, vs, vd;
	PIXEL_32 v32, v32s, v32d;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;
	/* Tell gcc that this cannot happen (already checked in c_blit_area() below) */
	if (w <= 0 || h <= 0)
		unreachable();
	for (i = h - 1; i >= 0; i--)
	{
		if (w & 1)
		{
			vs = *src_addr++;
#ifdef BOTH
			vd = *dst_addr_fast;
#else
			vd = *dst_addr;
#endif
			DO_OP(v);
#ifdef BOTH
			*dst_addr_fast++ = v;
#endif
			*dst_addr++ = v;
		}
		src_addr32 = (PIXEL_32 *)src_addr;
		dst_addr32 = (PIXEL_32 *)dst_addr;
#ifdef BOTH
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast;
#endif
		for(j = (w >> 1) - 1; j >= 0; j--) {
			v32s = *src_addr32++;
#ifdef BOTH
			v32d = *dst_addr_fast32;
#else
			v32d = *dst_addr32;
#endif
			DO_OP(v32);
#ifdef BOTH
			*dst_addr_fast32++ = v32;
#endif
			*dst_addr32++ = v32;
		}
		src_addr = (PIXEL *)src_addr32;
		dst_addr = (PIXEL *)dst_addr32;
		src_addr += src_line_add;
		dst_addr += dst_line_add;
#ifdef BOTH
		dst_addr_fast = (PIXEL *)dst_addr_fast32;
		dst_addr_fast += dst_line_add;
#endif
	}
}


static void
pan_backwards_copy(PIXEL *src_addr, int src_line_add,
                   PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
                   short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
	PIXEL v;
	PIXEL_32 v32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%2),%3\n" \
			" move.w %3,-(%0)\n" \
			" move.w %3,-(%1)\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%2),%3\n" \
			" move.l %3,-(%0)\n" \
			" move.l %3,-(%1)\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%1),-(%0)\n" \
			: "=a"(dst_addr), "=a"(src_addr) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%1),-(%0)\n" \
			: "=a"(dst_addr32), "=a"(src_addr32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void
pan_backwards_or(PIXEL *src_addr, int src_line_add,
                 PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
                 short int w, short int h)
{
	short int x, y, x4, xR;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
	PIXEL v;
	PIXEL_32 v32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif

	(void) dst_addr_fast;

#ifdef BOTH
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%2),%3\n" \
			" or.w -(%1),%3\n" \
			" move.w %3,(%1)\n" \
			" move.w %3,-(%0)\n" \
			: "=a"(dst_addr), "=a"(dst_addr_fast), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(dst_addr_fast), "2"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%2),%3\n" \
			" or.l -(%1),%3\n" \
			" move.l %3,-(%1)\n" \
			" move.l %3,-(%0)\n" \
			: "=a"(dst_addr32), "=a"(dst_addr_fast32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(dst_addr_fast32), "2"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr; \
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32; \
		dst_addr_fast = (PIXEL *)dst_addr_fast32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add; \
		dst_addr_fast += dst_line_add
#else
#define COPY_PIXEL \
		__asm__ __volatile__( \
			" move.w -(%1),%2\n" \
			" or.w %2,-(%0)\n" \
			: "=a"(dst_addr), "=a"(src_addr), "=d"(v) \
			: "0"(dst_addr), "1"(src_addr) \
			: "cc", "memory")
#define COPY_2PIXEL \
		__asm__ __volatile__( \
			" move.l -(%1),%2\n" \
			" or.l %2,-(%0)\n" \
			: "=a"(dst_addr32), "=a"(src_addr32), "=d"(v32) \
			: "0"(dst_addr32), "1"(src_addr32) \
			: "cc", "memory")
#define SET_PIXEL32_ADDR \
		src_addr32 = (PIXEL_32 *)src_addr; \
		dst_addr32 = (PIXEL_32 *)dst_addr
#define SET_PIXEL_ADDR \
		src_addr = (PIXEL *)src_addr32; \
		dst_addr = (PIXEL *)dst_addr32
#define NEXTLINE \
		src_addr += src_line_add; \
		dst_addr += dst_line_add
#endif

	x = w;
	y = h;
	while (y--)
	{
		x4 = x >> 2;
		SET_PIXEL32_ADDR;
		while (x4--)
		{
			COPY_2PIXEL;
			COPY_2PIXEL;
		}
		SET_PIXEL_ADDR;
		xR = x & 3;
		while (xR--)
		{
			COPY_PIXEL;
		}
		NEXTLINE;
    }

#undef COPY_PIXEL
#undef COPY_2PIXEL
#undef SET_PIXEL32_ADDR
#undef SET_PIXEL_ADDR
#undef NEXTLINE
}


static void
pan_backwards(PIXEL *src_addr, int src_line_add,
              PIXEL *dst_addr, PIXEL *dst_addr_fast, int dst_line_add,
              short int w, short int h, short int operation)
{
	short int i, j;
	PIXEL v, vs, vd;
	PIXEL_32 v32, v32s, v32d;
	PIXEL_32 *src_addr32;
	PIXEL_32 *dst_addr32;
#ifdef BOTH
	PIXEL_32 *dst_addr_fast32;
#endif
	
	(void) dst_addr_fast;
	/* Tell gcc that this cannot happen (already checked in c_blit_area() below) */
	if (w <= 0 || h <= 0)
		unreachable();
	for (i = h - 1; i >= 0; i--)
	{
		if (w & 1)
		{
			vs = *--src_addr;
#ifdef BOTH
			vd = *--dst_addr_fast;
#else
			vd = *--dst_addr;
#endif
			DO_OP(v);
#ifdef BOTH
			*dst_addr_fast = v;
#endif
			*dst_addr = v;
		}
		src_addr32 = (PIXEL_32 *)src_addr;
		dst_addr32 = (PIXEL_32 *)dst_addr;
#ifdef BOTH
		dst_addr_fast32 = (PIXEL_32 *)dst_addr_fast;
#endif
		for (j = (w >> 1) - 1; j >= 0; j--)
		{
			v32s = *--src_addr;
#ifdef BOTH
			v32d = *--dst_addr_fast32;
#else
			v32d = *--dst_addr32;
#endif
			DO_OP(v32);
#ifdef BOTH
			*dst_addr_fast32 = v32;
#endif
			*dst_addr32 = v32;
		}
		src_addr = (PIXEL *)src_addr32;
		dst_addr = (PIXEL *)dst_addr32;
		src_addr += src_line_add;
		dst_addr += dst_line_add;
#ifdef BOTH
		dst_addr_fast = (PIXEL *)dst_addr_fast32;
		dst_addr_fast += dst_line_add;
#endif
	}
}


#ifdef BOTH_WAS_ON
#define BOTH
#endif

#undef DO_OP


long CDECL
c_blit_area(Virtual *vwk, MFDB *src, long src_x, long src_y,
            MFDB *dst, long dst_x, long dst_y,
            long w, long h, long operation)
{
	Workstation *wk;
	PIXEL *src_addr, *dst_addr, *dst_addr_fast;
	int src_wrap, dst_wrap;
	int src_line_add, dst_line_add;
	unsigned long src_pos, dst_pos;
	int to_screen;

	if (w <= 0 || h <= 0)
		return 1;

	wk = vwk->real_address;

	if (!src || !src->address || (src->address == wk->screen.mfdb.address)) {		/* From screen? */
		src_wrap = wk->screen.wrap;
		if (!(src_addr = wk->screen.shadow.address))
			src_addr = wk->screen.mfdb.address;
	} else {
		src_wrap = (long)src->wdwidth * 2 * src->bitplanes;
		src_addr = src->address;
	}
	src_pos = (short)src_y * (long)src_wrap + src_x * PIXEL_SIZE;
	src_line_add = src_wrap - w * PIXEL_SIZE;

	to_screen = 0;
	if (!dst || !dst->address || (dst->address == wk->screen.mfdb.address)) {		/* To screen? */
		dst_wrap = wk->screen.wrap;
		dst_addr = wk->screen.mfdb.address;
		to_screen = 1;
	} else {
		dst_wrap = (long)dst->wdwidth * 2 * dst->bitplanes;
		dst_addr = dst->address;
	}
	dst_pos = (short)dst_y * (long)dst_wrap + dst_x * PIXEL_SIZE;
	dst_line_add = dst_wrap - w * PIXEL_SIZE;

	if (src_y < dst_y) {
		src_pos += (short)(h - 1) * (long)src_wrap;
		src_line_add -= src_wrap * 2;
		dst_pos += (short)(h - 1) * (long)dst_wrap;
		dst_line_add -= dst_wrap * 2;
	}

	src_addr += src_pos / PIXEL_SIZE;
	dst_addr += dst_pos / PIXEL_SIZE;
	src_line_add /= PIXEL_SIZE;		/* Change into pixel count */
	dst_line_add /= PIXEL_SIZE;

	dst_addr_fast = wk->screen.shadow.address;	/* May not really be to screen at all, but... */

#ifdef FVDI_DEBUG
	if (debug > 1) {
		debug_out("Blitting: ", w, -1, h, src_x, src_y, dst_x, dst_y);
	}
#endif

#ifdef BOTH
	if (!to_screen || !dst_addr_fast) {
#endif
		if ((src_y == dst_y) && (src_x < dst_x)) {
			src_addr += w;		/* To take backward copy into account */
			dst_addr += w;
			src_line_add += 2 * w;
			dst_line_add += 2 * w;
			switch(operation) {
			case 3:
				pan_backwards_copy(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h);
				break;
			case 7:
				pan_backwards_or(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h);
				break;
			default:
				pan_backwards(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h, operation);
				break;
			}
		} else {
			switch(operation) {
			case 3:
				blit_copy(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h);
				break;
			case 7:
				blit_or(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h);
				break;
			default:
				blit_16b(src_addr, src_line_add, dst_addr, 0, dst_line_add, w, h, operation);
				break;
			}
		}
#ifdef BOTH
	} else {
		dst_addr_fast += dst_pos / PIXEL_SIZE;
		if ((src_y == dst_y) && (src_x < dst_x)) {
			src_addr += w;		/* To take backward copy into account */
			dst_addr += w;
			dst_addr_fast += w;
			src_line_add += 2 * w;
			dst_line_add += 2 * w;
			switch(operation) {
			case 3:
				s_pan_backwards_copy(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h);
				break;
			case 7:
				s_pan_backwards_or(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h);
				break;
			default:
				s_pan_backwards(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h, operation);
				break;
			}
		} else {
			switch(operation) {
			case 3:
				s_blit_copy(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h);
				break;
			case 7:
				s_blit_or(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h);
				break;
			default:
				s_blit(src_addr, src_line_add, dst_addr, dst_addr_fast, dst_line_add, w, h, operation);
				break;
			}
		}
	}
#else
	(void) to_screen;
	(void) dst_addr_fast;
#endif

	return 1;	/* Return as completed */
}
