/*
 * fVDI default drawing function code
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * The Bresenham line drawing variation with perfect clipping is
 * based on code from the Graphics Gems series of books.
 */

#include "fvdi.h"
#include "function.h"
#include "relocate.h"
#include "utility.h"

void CDECL retry_line(Virtual *vwk, DrvLine *pars);
void CDECL vr_transfer_bits(Virtual *vwk, GCBITMAP *src_bm, GCBITMAP *dst_bm, RECT16 *src_rect, RECT16 *dst_rect, long mode);

void call_draw_line(Virtual *vwk, DrvLine *line);

#if 0
#define LEFT            1
#define RIGHT           2
#define BOTTOM          4
#define TOP             8

#define SWAP(x, y)      { int _t = x; x = y; y = _t; }

#define OUTCODE(x, y, outcode, type)                                     \
{                                                                        \
  if (x < clip[0]) {                                                     \
    outcode = LEFT;                                                      \
    type = 1;                                                            \
  } else if (x > clip[2]) {                                              \
    outcode = RIGHT;                                                     \
    type = 1;                                                            \
  } else                                                                 \
    outcode = type = 0;                                                  \
  if (y < clip[3]) {                                                     \
    outcode |= BOTTOM;                                                   \
    type++;                                                              \
  } else if (y > clip[1]) {                                              \
    outcode |= TOP;                                                      \
    type++;                                                              \
  }                                                                      \
}

#define CLIP(a1, a2, b1, da, da2, db2, as, bs, sa, sb,                   \
             amin, AMIN, amax, AMAX, bmin, BMIN, bmax, BMAX)             \
{                                                                        \
  long ca, cb;                                                           \
  if (out1) {                                                            \
    if (out1 & AMIN) {                                                   \
      ca = db2 * (amin - a1);                                            \
      as = amin;                                                         \
    } else if (out1 & AMAX) {                                            \
      ca = db2 * (a1 - amax);                                            \
      as = amax;                                                         \
    }                                                                    \
    if (out1 & BMIN) {                                                   \
      cb = da2 * (bmin - b1);                                            \
      bs = bmin;                                                         \
    } else if (out1 & BMAX) {                                            \
      cb = da2 * (b1 - bmax);                                            \
      bs = bmax;                                                         \
    }                                                                    \
    if (type1 == 2)                                                      \
      out1 &= (ca + da < cb + !dir) ? ~(AMIN | AMAX) : ~(BMAX | BMIN);   \
    if (out1 & (AMIN | AMAX)) {                                          \
      cb = (ca + da - !dir) / da2;                                       \
      if (sb >= 0) {                                                     \
        if ((bs = b1 + cb) > bmax)                                       \
          return ptr;                                                    \
      } else {                                                           \
        if ((bs = b1 - cb) < bmin)                                       \
          return ptr;                                                    \
      }                                                                  \
      r += ca - da2 * cb;                                                \
    } else {                                                             \
      ca = (cb - da + db2 - dir) / db2;                                  \
      if (sa >= 0) {                                                     \
        if ((as = a1 + ca) > amax)                                       \
          return ptr;                                                    \
      } else {                                                           \
        if ((as = a1 - ca) < amin)                                       \
          return ptr;                                                    \
      }                                                                  \
      r += db2 * ca - cb;                                                \
    }                                                                    \
  } else {                                                               \
    as = a1;                                                             \
    bs = b1;                                                             \
  }                                                                      \
  alt = 0;                                                               \
  if (out2) {                                                            \
    if (type2 == 2) {                                                    \
      ca = db2 * ((out2 & AMIN) ? a1 - amin : amax - a1);                \
      cb = da2 * ((out2 & BMIN) ? b1 - bmin : bmax - b1);                \
      out2 &= (cb + da < ca + dir) ? ~(AMIN | AMAX) : ~(BMIN | BMAX);    \
    }                                                                    \
    if (out2 & (AMIN | AMAX))                                            \
      n = (out2 & AMIN) ? as - amin : amax - as;                         \
    else {                                                               \
      n = (out2 & BMIN) ? bs - bmin : bmax - bs;                         \
      alt = 1;                                                           \
    }                                                                    \
  } else                                                                 \
    n = (a2 >= as) ? a2 - as : as - a2;                                  \
}


short *clip_line(short x1, short y1, short x2, short y2,
                 short clip[], short draw_last, short *ptr, short *total, short *first, short *drawn)
/*
 *      If dir = 0, round towards (x1, y1)
 *      If dir = 1, round towards (x2, y2)
 */
{
    short dir = 0;

    short adx,
     ady,
     adx2,
     ady2,
     sx,
     sy;

    short out1,
     out2,
     type1,
     type2;

    short r,
     xs,
     ys,
     n,
     alt;

    OUTCODE(x1, y1, out1, type1);
    OUTCODE(x2, y2, out2, type2);
    if (out1 & out2)
        return ptr;

#if 0
    /* Swap coordinates in some cases for efficiency */
    if ((type1 != 0 && type2 == 0) || (type1 == 2 && type2 == 1))
    {
        SWAP(out1, out2);
        SWAP(type1, type2);
        SWAP(x1, x2);
        SWAP(y1, y2);
        dir ^= 1;
    }
#endif

    xs = x1;
    ys = y1;
    sx = 1;
    adx = x2 - x1;
    if (adx < 0)
    {
        adx = -adx;
        sx = -1;
    }
    sy = 1;
    ady = y2 - y1;
    if (ady < 0)
    {
        ady = -ady;
        sy = -1;
    }
    adx2 = 2 * adx;
    ady2 = 2 * ady;

    *first = *drawn = 0;

    if (adx >= ady)
    {
        /*
         *      Line is semi-horizontal
         */
        *total = adx + 1;
        r = ady2 - adx - !dir;
        CLIP(x1, x2, y1, adx, adx2, ady2, xs, ys, sx, sy, clip[0], LEFT, clip[2], RIGHT, clip[3], BOTTOM, clip[1], TOP);
        r -= ady2;
        *first = xs;
        if (alt)
        {
            for (;; xs += sx)
            {                           /* Alternate Bresenham */
                *ptr++ = xs;
                *ptr++ = ys;
                r += ady2;
                if (r >= 0)
                {
                    if (--n < 0)
                        break;
                    r -= adx2;
                    ys += sy;
                }
            }
        } else
        {
            do
            {                           /* Standard Bresenham */
                *ptr++ = xs;
                *ptr++ = ys;
                r += ady2;
                if (r >= 0)
                {
                    r -= adx2;
                    ys += sy;
                }
                xs += sx;
            } while (n--);
        }
        *drawn = ABS(ptr[-2] - *first + 1);
        *first = ABS(*first - x1);
    } else
    {
        /*
         *      Line is semi-vertical
         */
        *total = ady + 1;
        r = adx2 - ady - !dir;
        CLIP(y1, y2, x1, ady, ady2, adx2, ys, xs, sy, sx, clip[3], BOTTOM, clip[1], TOP, clip[0], LEFT, clip[2], RIGHT);
        r -= adx2;
        *first = ys;
        if (alt)
        {
            for (;; ys += sy)
            {                           /* Alternate Bresenham */
                *ptr++ = xs;
                *ptr++ = ys;
                r += adx2;
                if (r >= 0)
                {
                    if (--n < 0)
                        break;
                    r -= ady2;
                    xs += sx;
                }
            }
        } else
        {
            do
            {                           /* Standard Bresenham */
                *ptr++ = xs;
                *ptr++ = ys;
                r += adx2;
                if (r >= 0)
                {
                    r -= ady2;
                    xs += sx;
                }
                ys += sy;
            } while (n--);
        }
        *drawn = ABS(ptr[-1] - *first + 1);
        *first = ABS(*first - y1);
    }

    if (!draw_last && (ptr[-2] == x2) && (ptr[-1] == y2))
    {
        ptr -= 2;
        *drawn -= 1;
        *total -= 1;
    }

    return ptr;
}


static void polymline(short table[], int length, short index[], int moves, short clip[], short *points)
{
    short n;
    short x1, y1, x2, y2, init_x, init_y;
    short movepnt;
    short *ptr, *oldptr;
    short draw_last;
    short total, first, drawn;

    ptr = points;
    oldptr = points;

    x1 = *table++;
    y1 = *table++;

    if (moves)
    {
        init_x = x1;
        init_y = y1;
    } else
    {
        init_x = index[0];
        init_y = index[1];
    }

    movepnt = -1;
    if (index)
    {
        moves--;
        if (index[moves] == -4)
            moves--;
        if (index[moves] == -2)
            moves--;
        if (moves >= 0)
            movepnt = (index[moves] + 4) / 2;
    }

    for (n = 1; n < length; n++)
    {
        x2 = *table++;
        y2 = *table++;
        if (n == movepnt)
        {
            if (--moves >= 0)
                movepnt = (index[moves] + 4) / 2;
            else
                movepnt = -1;           /* Never again equal to n */
            init_x = x1 = x2;
            init_y = y1 = y2;
            continue;
        }

        if (((n == length - 1) || (n == movepnt - 1)) && ((x2 != init_x) || (y2 != init_y)))
            draw_last = 1;              /* Draw last pixel in the line */
        else
            draw_last = 0;              /* Do not draw last pixel in the line */

        newptr = clip_line(x1, y1, x2, y2, clip, draw_last, ptr, &total, &first, &drawn);

        if (first)
        {
            /* Beginning was clipped, so draw previous part */
            draw...if (total - first != drawn)
            {
                /* End was clipped, so draw this part */
                draw...rotate_left(pattern, (previous + first) % 16);
                pattern = rotate_left(pattern, (previous + total) % 16);
                oldptr = ptr = points;
            } else
            {
                pattern = rotate_left(pattern, (previous + first) % 16);
                oldptr = ptr;
                ptr = newptr;
            }
        } else if (total - first != drawn)
        {
            /* End was clipped, so draw previous and this part */
            draw...;
            pattern = rotate_left(pattern, (previous + total) % 16);
            oldptr = ptr = points;
        }

        if (not enough space left for a max length line)
        {
            draw current points pattern = rotate_left(pattern, all % 16);

            oldptr = ptr = points;
        }


        x1 = x2;
        y1 = y2;
    }
}


/*
 * Pixel by pixel line routine
 * In:   a0      VDI struct (odd address marks table operation)
 *       d0      line colour
 *       d1      x1 or table address
 *       d2      y1 or table length (high) and type (0 - coordinate pairs, 1 - pairs+moves)
 *       d3      x2 or move point count
 *       d4      y2 or move index address
 *       d5      pattern
 *       d6      mode
 */
int CDECL default_line(Virtual *vwk, long x1, long y1, long x2, long y2, long pattern, long colour, long mode)
{
    ulong foreground, background;
    ulong pat;
    short *table, length, *index, moves;
    short ltab[4];

    index = 0;
    moves = 0;
    if ((long) vwk & 1)
    {
        if ((unsigned long) (y1 & 0xffff) > 1)
            return -1;                  /* Don't know about this kind of table operation */
        table = (short *) x1;
        length = (y1 >> 16) & 0xffff;
        if ((y1 & 0xffff) == 1)
        {
            index = (short *) y2;
            moves = x2 & 0xffff;
        }
        vwk = (Virtual *) ((long) vwk - 1);
        x1 = table[0];
        y1 = table[1];
        x2 = table[2];
        y2 = table[3];
    } else
    {
        table = ltab;
        length = 2;
        table[0] = x1;
        table[1] = y1;
        table[2] = x2;
        table[3] = y2;
    }

    get_colours_r(vwk, colour, &foreground, &background);

    if (moves)
        polymline(table, length, index, moves, (short *) &vwk->clip.rectangle, points);
    else
    {
        short connected = (x1 == table[(length - 1) * 2]) && (y1 == table[(length - 1) * 2 + 1]);

        index = table;                  /* To check for connection in polymline */

        table += 4;
        for (--length; length > 0; length--)
        {
            short skip_last = connected || (length != 1);

            if (x1 == x2)
            {
                if (y1 < y2)
                {
                    short y_max = y2 - skip_last;

                    if (y1 < vwk->clip.rectangle.y1)
                        y1 = vwk->clip.rectangle.y1;
                    if (y_max > vwk->clip.rectangle.y2)
                        y_max = vwk->clip.rectangle.y2;
                    for (; y1 <= y_max; y1++)
                    {
                        *ptr++ = x1;
                        *ptr++ = y1;
                    }
                } else
                {
                    short y_min = y2 + skip_last;

                    if (y1 > vwk->clip.rectangle.y2)
                        y1 = vwk->clip.rectangle.y2;
                    if (y_min < vwk->clip.rectangle.y1)
                        y_min = vwk->clip.rectangle.y1;
                    for (; y1 >= y_min; y1--)
                    {
                        *ptr++ = x1;
                        *ptr++ = y1;
                    }
                }
            } else if (y1 == y2)
            {
                if (x1 < x2)
                {
                    short x_max = x2 - skip_last;

                    if (x1 < vwk->clip.rectangle.x1)
                        x1 = vwk->clip.rectangle.x1;
                    if (x_max > vwk->clip.rectangle.x2)
                        x_max = vwk->clip.rectangle.x2;
                    for (; x1 <= x_max; x1++)
                    {
                        *ptr++ = x1;
                        *ptr++ = y1;
                    }
                } else
                {
                    short x_min = x2 + skip_last;

                    if (x1 > vwk->clip.rectangle.x2)
                        x1 = vwk->clip.rectangle.x2;
                    if (x_min < vwk->clip.rectangle.x1)
                        x_min = vwk->clip.rectangle.x1;
                    for (; x1 >= x_min; x1--)
                    {
                        *ptr++ = x1;
                        *ptr++ = y1;
                    }
                }
            } else
            {
                length++;
                table -= 4;
                break;
            }
            x1 = x2;
            y1 = y2;
            x2 = *table++;
            y2 = *table++;
        }
        if (length)
        {
            polymline(table, length, index, 0, (short *) &vwk->clip.rectangle, points);
        }
    }

    return 1;
}



/*
 * Fill a multiple bitplane area using a monochrome pattern
 * c_fill_area(Virtual *vwk, long x, long y, long w, long h, short *pattern, long colour, long mode, long interior_style)
 * In:  a0  VDI struct (odd address marks table operation)
 *  d0  colours
 *  d1  x1 destination or table address
 *  d2  y1    - " -    or table length (high) and type (0 - y/x1/x2 spans)
 *  d3-d4   x2,y2 destination
 *  d5  pattern address
 *  d6  mode
 *  d7  interior/style
 */
long CDECL c_fill_area(Virtual *vwk, long x, long y, long w, long h,
                       short *pattern, long colour, long mode, long interior_style)
{
    ulong foreground,
     background;

    short *table;

    table = 0;
    if ((long) vwk & 1)
    {
        if ((y & 0xffff) != 0)
            return -1;                  /* Don't know about this kind of table operation */
        table = (short *) x;
        h = (y >> 16) & 0xffff;
        vwk = (Virtual *) ((long) vwk - 1);
    }

    get_colours_r(vwk, colour, &foreground, &background);

    if ((interior_style >> 16) >= 2 && (interior_style & 0xffffL) != 8)
    {
        /* Pattern */
        ulong pat0,
         pat1;

        pat0 =
            (((long) pattern[3] & 0x00ff) << 24) | (((long) pattern[2] & 0x00ff) << 16) | (((long) pattern[1] & 0x00ff)
                                                                                           << 8) | ((pattern[0] &
                                                                                                     0x00ff));
        pat1 =
            (((long) pattern[7] & 0x00ff) << 24) | (((long) pattern[6] & 0x00ff) << 16) | (((long) pattern[5] & 0x00ff)
                                                                                           << 8) | ((pattern[4] &
                                                                                                     0x00ff));

        write_raw_pattern(register_base, pat0, pat1);   /* Six register writes */
#if 0
    } else if ((interior_style >> 16) == 0)
    {
        /* Setup pixel data path */
        reg_w(register_base, DP_SRC, FRGD_SRC_BKGD_CLR);
#endif
    } else
    {
        /* Setup pixel data path */
        reg_w(register_base, DP_SRC, FRGD_SRC_FRGD_CLR);
    }

    /* Perform rectangle fill. */
    if (!table)
    {
        dst_yx(x, y);
        dst_hw(w, h);
    } else
    {
        for (h = h - 1; h >= 0; h--)
        {
            y = *table++;
            x = *table++;
            w = *table++ - x + 1;
#ifdef CHECK_FIFO
            while ((swap16(reg_r(register_base, FIFO_STAT) & 0xFFFF)) > ((unsigned int) (0x8000 >> 2))) ;
#endif
            dst_yx(x, y);
            dst_hw(w, 1);
        }
    }

    return 1;
}


retry_set_pixel(Virtual *vwk, MFDB *mfdb, long x, long y, long colour, long pattern, long mode)
{
    short *table;

    short length;

    if ((long) vwk & 1)
    {
        if ((unsigned long) (y & 0xffff) > 1)
            return -1;                  /* Don't know about this kind of table operation */
        table = (short *) x;
        length = (y >> 16) & 0xffff;
        vwk = (Virtual *) ((long) vwk - 1);
    } else
        return;

    if ((y & 0xffff) == 0)
    {
        for (--length; length >= 0; length--)
        {
            x = *table++;
            y = *table++;
            c_set_pixel(vwk, mfdb, x, y, colour);
        }
    } else
    {
        ushort pat;

        ulong foreground,
         background;

        get_colours_r(vwk, colour, &foreground, &background);
        pat = pattern;
        for (--length; length >= 0; length--)
        {
            x = *table++;
            y = *table++;
            switch (mode)
            {
            case 1:
                if (pat & 0x8000)
                    c_set_pixel(vwk, mfdb, x, y, foreground);
                else
                    c_set_pixel(vwk, mfdb, x, y, background);
                break;
            case 2:
                if (pat & 0x8000)
                    c_set_pixel(vwk, mfdb, x, y, foreground);
                break;
            case 3:
                if (pat & 0x8000)
                    c_set_pixel(vwk, mfdb, x, y, ~c_get_pixel(vwk, mfdb, x, y));
                break;
            case 4:
                if (!(pat & 0x8000))
                    c_set_pixel(vwk, mfdb, x, y, foreground);
                break;
            }
            pat = (pat << 1) || (pat >> 15);
        }
    }
}
#endif


void CDECL retry_line(Virtual *vwk, DrvLine *pars)
{
    short *table, length, *index, moves;
    short n;
    short init_x, init_y, x2, y2;
    short movepnt;
    DrvLine line;

    table = index = 0;
    length = moves = 0;
    if ((long) vwk & 1)
    {
        if ((unsigned long) (pars->y1 & 0xffff) > 1)
            return;                     /* Don't know about this kind of table operation */
        table = (short *) pars->x1;
        length = (pars->y1 >> 16) & 0xffff;
        if ((pars->y1 & 0xffff) == 1)
        {
            index = (short *) pars->y2;
            moves = pars->x2 & 0xffff;
        }
        vwk = (Virtual *) ((long) vwk - 1);
    } else
        return;

    /* Clear high words of coordinate unions? */
    line.pattern = pars->pattern;
    line.colour = pars->colour;
    line.mode = pars->mode;

    init_x = line.x1 = *table++;
    init_y = line.y1 = *table++;

    movepnt = -1;
    if (index)
    {
        moves--;
        if (index[moves] == -4)
            moves--;
        if (index[moves] == -2)
            moves--;
        if (moves >= 0)
            movepnt = (index[moves] + 4) / 2;
    }

    for (n = 1; n < length; n++)
    {
        x2 = *table++;
        y2 = *table++;
        if (n == movepnt)
        {
            if (--moves >= 0)
                movepnt = (index[moves] + 4) / 2;
            else
                movepnt = -1;           /* Never again equal to n */
            init_x = line.x1 = x2;
            init_y = line.y1 = y2;
            continue;
        }

        if (((n == length - 1) || (n == movepnt - 1)) && ((x2 != init_x) || (y2 != init_y)))
            line.draw_last = 1;         /* Draw last pixel in the line */
        else
            line.draw_last = 0;         /* Do not draw last pixel in the line */

#if 0
        c_draw_line(vwk, x1, y1, x2, y2, pattern, colour, mode, draw_last);
#else
        line.x2 = x2;
        line.y2 = y2;
        call_draw_line(vwk, &line);
#endif

        line.x1 = x2;
        line.y1 = y2;
    }
}


/* This is in a rather adhoc state at the moment, unfortunately.
 * But it does seem to work, for what it does, and is not complicated.
 */
void CDECL vr_transfer_bits(Virtual *vwk, GCBITMAP *src_bm, GCBITMAP *dst_bm, RECT16 *src_rect, RECT16 *dst_rect, long mode)
{
    int error = 0;

    do
    {
        if ((src_rect->x2 - src_rect->x1 != dst_rect->x2 - dst_rect->x1) ||
            (src_rect->y2 - src_rect->y1 != dst_rect->y2 - dst_rect->y1))
        {
            PUTS("No support yet for scaling\n");
            error = 1;
            break;
        }

        if (!src_bm)
        {
            PUTS("No support yet for screen->memory/screen\n");
            error = 1;
            break;
        }

        if (dst_bm)
        {
            if (src_bm->px_format != dst_bm->px_format)
            {
                if (src_bm->px_format == 0x01020101 && dst_bm->px_format == 0x01020808)
                {
                    int x, y;

                    for (y = src_rect->y1; y <= src_rect->y2; y++)
                    {
                        long *src;
                        unsigned char *dst;
                        unsigned long v, mask;

                        src = (long *)(src_bm->addr + src_bm->width * y + (src_rect->x1 / 32) * 4);
                        mask = 1 << (31 - src_rect->x1 % 32);
                        dst = dst_bm->addr + dst_bm->width * (dst_rect->y1 - src_rect->y1 + y) + dst_rect->x1;
                        v = *src++;
                        if (mode == 33)
                        {
                            for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            {
                                if (v & mask)
                                    *dst = ~0;
                                dst++;
                                mask = (mask >> 1) | (mask << 31);
                                if ((long)mask < 0)
                                    v = *src++;
                            }
                        } else
                        {
                            for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            {
                                if (v & mask)
                                    *dst++ = ~0;
                                else
                                    *dst++ = 0;
                                mask = (mask >> 1) | (mask << 31);
                                if ((long)mask < 0)
                                    v = *src++;
                            }
                        }
                    }
                    mode = 0;  /* Just to skip error printout at the end */
                } else if (src_bm->px_format == 0x01020101 && dst_bm->px_format == 0x03421820)
                {
                    int x, y;

                    for (y = src_rect->y1; y <= src_rect->y2; y++)
                    {
                        long *src;
                        long *dst;
                        unsigned long v, mask;

                        src = (long *)(src_bm->addr + src_bm->width * y + (src_rect->x1 / 32) * 4);
                        mask = 1 << (31 - src_rect->x1 % 32);
                        dst = (long *)(dst_bm->addr + dst_bm->width * (dst_rect->y1 - src_rect->y1 + y)) + dst_rect->x1;
                        v = *src++;
                        if (mode == 33)
                        {
                            for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            {
                                if (v & mask)
                                    *dst = ~0;
                                dst++;
                                mask = (mask >> 1) | (mask << 31);
                                if ((long)mask < 0)
                                    v = *src++;
                            }
                        } else
                        {
                            for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            {
                                if (v & mask)
                                    *dst++ = ~0;
                                else
                                    *dst++ = 0;
                                mask = (mask >> 1) | (mask << 31);
                                if ((long) mask < 0)
                                    v = *src++;
                            }
                        }
                    }
                    mode = 0;  /* Just to skip error printout at the end */
                } else
                {
                    PUTS("No support yet for memory->memory between these different pixmap formats\n");
                    error = 1;
                    break;
                }
            } else
            {
                if (src_bm->px_format == 0x01020101)
                {
                    /* PX_PREF1 */
                    PUTS("No support yet for 1 bit memory->memory\n");
                    error = 1;
                    break;
                } else if (src_bm->px_format == 0x01020808)
                {
                    /* PX_PREF8 */
                    int x, y;

                    for (y = src_rect->y1; y <= src_rect->y2; y++)
                    {
                        unsigned char *src, *dst;

                        src = src_bm->addr + src_bm->width * y + src_rect->x1;
                        dst = dst_bm->addr + dst_bm->width * (dst_rect->y1 - src_rect->y1 + y) + dst_rect->x1;
                        for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            *dst++ = *src++;
                    }
                } else if (src_bm->px_format == 0x03421820)
                {
                    /* PX_PREF32 */
                    int x, y;

                    for (y = src_rect->y1; y <= src_rect->y2; y++)
                    {
                        long *src, *dst;

                        src = (long *)(src_bm->addr + src_bm->width * y) + src_rect->x1;
                        dst = (long *)(dst_bm->addr + dst_bm->width * (dst_rect->y1 - src_rect->y1 + y)) + dst_rect->x1;
                        for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                            *dst++ = *src++;
                    }
                } else
                {
                    PRINTF(("Unsupported pixel format ($%lx) for memory->memory\n", src_bm->px_format));
                    error = 1;
                    break;
                }
            }
        } else
        {
            if (src_bm->px_format == 0x01020101)
            {
                /* PX_PREF1 */
                int x, y, i;
                char *block;
                long *palette;
                MFDB mfdb;
                short coords[8];

#if 0
                if (!src_bm->ctab)
                {
                    PUTS("Need a colour table for 1 bit->TC\n");
                    error = 1;
                    break;
                }
                if (src_bm->ctab->color_space != 1)
                {
                    PUTS("Need an RGB colour table for 1 bit->TC\n");
                    error = 1;
                    break;
                }
#endif

                if ((block = (char *)allocate_block(0)) == 0)
                {
                    PUTS("Could not allocate memory block\n");
                    error = 1;
                    break;
                }
#if 0
                palette = (long *)block;
                for (i = 0; i < src_bm->ctab->no_colors; i++)
                {
                    palette[i] = ((long)(src_bm->ctab->colors[i].rgb.red & 0xff) << 16) |
                                 ((long)(src_bm->ctab->colors[i].rgb.green & 0xff) << 8) |
                                 ((long)(src_bm->ctab->colors[i].rgb.blue & 0xff));
                }
#endif

                mfdb.address = (short *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
                mfdb.width = src_rect->x2 - src_rect->x1 + 1;
                mfdb.height = 1;
                mfdb.wdwidth = (mfdb.width + 15) / 16;
                mfdb.standard = 0;
                mfdb.bitplanes = 32;

                coords[0] = 0;
                coords[1] = 0;
                coords[2] = src_rect->x2 - src_rect->x1 + 1;
                coords[3] = 0;

                coords[4] = dst_rect->x1;
                coords[5] = dst_rect->y1;
                coords[6] = dst_rect->x2;
                coords[7] = dst_rect->y1;

                for (y = src_rect->y1; y <= src_rect->y2; y++)
                {
                    long *src, *dst;
                    unsigned long v, mask;

                    src = (long *)(src_bm->addr + src_bm->width * y) + (src_rect->x1 / 32);
                    mask = 1 << (31 - src_rect->x1 % 32);
                    dst = (long *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
                    v = *src++;
                    if (mode == 33)
                    {
                        short coords2[8];

                        for (i = 0; i < 4; i++)
                        {
                            coords2[i] = coords[i + 4];
                            coords2[i + 4] = coords[i];
                        }
                        lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords2, 0, &mfdb, 0);
                        for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                        {
                            if (v & mask)
                                *dst = ~0;
                            dst++;
                            mask = (mask >> 1) | (mask << 31);
                            if ((long)mask < 0)
                                v = *src++;
                        }
                    } else
                    {
                        for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                        {
                            if (v & mask)
                                *dst++ = ~0;
                            else
                                *dst++ = 0;
                            mask = (mask >> 1) | (mask << 31);
                            if ((long)mask < 0)
                                v = *src++;
                        }
                    }
                    lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
                    coords[5]++;
                    coords[7]++;
                }

                free_block(block);
                mode = 0;  /* Just to skip error printout at the end */
            } else if (src_bm->px_format == 0x01020808)
            {
                /* PX_PREF8 */
                char *block;
                long *palette;
                unsigned char *src;
                long *dst;
                MFDB mfdb;
                short coords[8];
                int x, y, i;

                if (!src_bm->ctab)
                {
                    PUTS("Need a colour table for 8 bit->TC\n");
                    error = 1;
                    break;
                }
                if (src_bm->ctab->color_space != 1)
                {
                    PUTS("Need an RGB colour table for 8 bit->TC\n");
                    error = 1;
                    break;
                }

                if ((block = (char *)allocate_block(0)) == NULL)
                {
                    PUTS("Could not allocate memory block\n");
                    error = 1;
                    break;
                }

                palette = (long *)block;
                for (i = 0; i < src_bm->ctab->no_colors; i++)
                {
                    palette[i] = ((long)(src_bm->ctab->colors[i].rgb.red & 0xff) << 16) |
                                 ((long)(src_bm->ctab->colors[i].rgb.green & 0xff) << 8) |
                                 ((long)(src_bm->ctab->colors[i].rgb.blue & 0xff));
                }

                mfdb.address = (short *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
                mfdb.width = src_rect->x2 - src_rect->x1 + 1;
                mfdb.height = 1;
                mfdb.wdwidth = (mfdb.width + 15) / 16;
                mfdb.standard = 0;
                mfdb.bitplanes = 32;

                coords[0] = 0;
                coords[1] = 0;
                coords[2] = src_rect->x2 - src_rect->x1 + 1;
                coords[3] = 0;

                coords[4] = dst_rect->x1;
                coords[5] = dst_rect->y1;
                coords[6] = dst_rect->x2;
                coords[7] = dst_rect->y1;

                for (y = src_rect->y1; y <= src_rect->y2; y++)
                {
                    src = src_bm->addr + src_bm->width * y + src_rect->x1;
                    dst = (long *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
                    for (x = src_rect->x2 - src_rect->x1; x >= 0; x--)
                        *dst++ = palette[*src++];
                    lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
                    coords[5]++;
                    coords[7]++;
                }

                free_block(block);
            } else if (src_bm->px_format == 0x03421820)
            {
                /* PX_PREF32 */
                MFDB mfdb;
                short coords[8];

                mfdb.address = (short *)src_bm->addr;
                mfdb.width = src_bm->xmax - src_bm->xmin;
                mfdb.height = src_bm->ymax - src_bm->ymin;
                mfdb.wdwidth = (mfdb.width + 15) / 16;
                mfdb.standard = 0;
                mfdb.bitplanes = 32;

                coords[0] = src_rect->x1;
                coords[1] = src_rect->y1;
                coords[2] = src_rect->x2;
                coords[3] = src_rect->y2;

                coords[4] = dst_rect->x1;
                coords[5] = dst_rect->y1;
                coords[6] = dst_rect->x2;
                coords[7] = dst_rect->y2;

                lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
            } else
            {
                PRINTF(("Unsupported source pixel format ($%lx) for !TC->TC\n", src_bm->px_format));
                error = 1;
                break;
            }
        }
    } while (0);

    if (error)
    {
#ifdef FVDI_DEBUG
        int i;

        PRINTF(("vr_transfer_bits mode %ld\n", mode));

        for (i = 0; i < 2; i++)
        {
            GCBITMAP *bm = (i == 0) ? src_bm : dst_bm;

            if (bm)
            {
                PRINTF(("%s $%08lx  Width: %ld  Bits: %ld  Format: $%lx",
                    i == 0 ? "SRC" : "DST",
                    (long) bm->addr,
                    bm->width,
                    bm->bits,
                    bm->px_format));
                if (bm->xmin || bm->ymin)
                {
                    PRINTF(("  Base: %ld,%ld", bm->xmin, bm->ymin));
                }
                PRINTF(("  Dim: %ld,%ld\n", bm->xmax - bm->xmin, bm->ymax - bm->ymin));

                PRINTF(("    C/ITAB: $%08lx%08lx/$", (long) bm->ctab, (long) bm->itab));

                if (bm->ctab)
                {
                    PRINTF(("  ID: $%lx  Space: %ld  Flags: $%lx  Colours: %ld", bm->ctab->map_id, bm->ctab->color_space, bm->ctab->flags, bm->ctab->no_colors));
                }

                PUTS("\n");
            }
        }

        PUTS("--------\n");
    } else if (mode != 0 && mode != 32)
    {
        PRINTF(("\nvr_transform_bits mode %ld\n\n", mode));
#endif
    }
}


#if 0
retry_fill(Virtual *vwk, long x, long y, long w, long h, short *pattern, long colour, long mode, long interior_style)
{
    short *table;

    short h;

    table = 0;
    if ((long) vwk & 1)
    {
        if ((y & 0xffff) != 0)
            return -1;                  /* Don't know about this kind of table operation */
        table = (short *) x;
        h = (y >> 16) & 0xffff;
        vwk = (Virtual *) ((long) vwk - 1);
    } else
        return;

    for (h = h - 1; h >= 0; h--)
    {
        y = *table++;
        x = *table++;
        w = *table++ - x + 1;
        c_fill_area(vwk, x, y, w, 1, pattern, colour, mode, interior_style);
    }
}
#endif
