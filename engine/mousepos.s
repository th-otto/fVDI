/*
 * small test program. constantly write the mouse cursor
 * position to the top right corner of the screen.
 * known bugs:
 * - always uses an 8x16 font, no check on resolution is done.
 * - directly writes to the screen.
 * - planar screen organisation assumed, does not work on hicolor
 * - blindly installs an interrupt routine in vblqueue[1]
 */

GCURX = -0x25a
PLANES = 0
BYTES_LIN = -2

vblqueue = 0x456
v_bas_ad = 0x44e

init:
	movea.l    4(a7),a0
	move.l     a0,_BasPag
	lea.l      stacktop,a7
	move.l     12(a0),d0
	add.l      20(a0),d0
	add.l      28(a0),d0
	addi.l     #256,d0
	move.l     d0,d7
	move.l     d0,-(a7)
	move.l     a0,-(a7)
	clr.w      -(a7)
	move.w     #0x004A,-(a7)
	trap       #1
	lea.l      12(a7),a7

	.dc.w 0xa000
	movem.l    a0-a2,linea_vars
	move.l a0,a5

	pea init_intr(pc)
	move.w #38,-(a7)
	trap #14
	addq.l     #6,a7

	clr.w      -(a7)
	move.l     d7,-(a7)
	move.w     #49,-(a7) /* Ptermres */
	trap       #1

init_intr:
	move.l (vblqueue).w,a0
	lea    vbl_intr(pc),a1
	move.l a1,4(a0)
	rts

vbl_intr:
	move.l linea_vars(pc),a0
	lea.l  lastpos(pc),a1
	move.l GCURX(a0),d0
	cmp.l  (a1),d0
	beq vbl_intr_ex
	move.l d0,(a1)
	move.l (v_bas_ad).w,a1
	move.w BYTES_LIN(a0),d1
	move.w PLANES(a0),d2

	add.w  d1,a1
	move.w d2,d3
	add.w  d3,d3
    sub.w  d3,a1
    subq.w #1,d3
    subq.w #1,d2
    bsr    print_digit
    sub.w  d3,a1
    bsr    print_digit
    subq.w #1,a1
    bsr    print_digit
    sub.w  d3,a1
    bsr    print_digit
    subq.w #1,a1
    sub.w  d3,a1
    swap   d0
    bsr    print_digit
    subq.w #1,a1
    bsr    print_digit
    sub.w  d3,a1
    bsr    print_digit
    subq.w #1,a1
    bsr    print_digit
vbl_intr_ex:
	rts

print_digit:
	moveq  #0,d4
	move.w d0,d4
	divu   #10,d4
	move.w d4,d0
	swap   d4
	lsl.w  #4,d4
	lea    fonttab(pc,d4.w),a0
	moveq  #15,d4
	move.l a1,a3
charloop:
	move.l a3,a4
	move.w d2,d5
planeloop:
	move.b (a0),(a4)
	lea 2(a4),a4
	dbf d5,planeloop
	add.w d1,a3
	addq.w #1,a0
	dbf d4,charloop

	rts

fonttab:
	dc.b 0x00,0x00,0x3c,0x7e,0x66,0x66,0x66,0x6e,0x76,0x66,0x66,0x66,0x7e,0x3c,0x00,0x00
	dc.b 0x00,0x00,0x18,0x18,0x38,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x7e,0x7e,0x00,0x00
	dc.b 0x00,0x00,0x3c,0x7e,0x66,0x66,0x0c,0x0c,0x18,0x18,0x30,0x30,0x7e,0x7e,0x00,0x00
	dc.b 0x00,0x00,0x7e,0x7e,0x0c,0x0c,0x18,0x18,0x0c,0x0c,0x66,0x66,0x7e,0x3c,0x00,0x00
	dc.b 0x00,0x00,0x0c,0x0c,0x1c,0x1c,0x3c,0x3c,0x6c,0x6c,0x7e,0x7e,0x0c,0x0c,0x00,0x00
	dc.b 0x00,0x00,0x7e,0x7e,0x60,0x60,0x7c,0x7e,0x06,0x06,0x06,0x66,0x7e,0x3c,0x00,0x00
	dc.b 0x00,0x00,0x1c,0x3c,0x70,0x60,0x60,0x7c,0x7e,0x66,0x66,0x66,0x7e,0x3c,0x00,0x00
	dc.b 0x00,0x00,0x7e,0x7e,0x06,0x06,0x0c,0x0c,0x18,0x18,0x30,0x30,0x30,0x30,0x00,0x00
	dc.b 0x00,0x00,0x3c,0x7e,0x66,0x66,0x3c,0x3c,0x66,0x66,0x66,0x66,0x7e,0x3c,0x00,0x00
	dc.b 0x00,0x00,0x3c,0x7e,0x66,0x66,0x7e,0x3e,0x06,0x06,0x06,0x0e,0x3c,0x38,0x00,0x00

	.data
		
lastpos: dc.l -1


	.bss
_BasPag:            ds.l 1

/* return values from linea_init, must be in order */
linea_vars:         ds.l 1
linea_fonts:        ds.l 1
linea_fcts :        ds.l 1

stack: ds.b 256
stacktop: ds.l 1
