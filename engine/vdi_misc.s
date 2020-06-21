*****
* fVDI miscellaneous functions
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

always_clip_r	equ	1		; Always clip rectangles?
always_clip_p	equ	1		; Always clip points?
always_clip_l	equ	1		; Always clip lines?

	.include	"vdi.inc"
	.include	"macros.inc"

	xdef	clip_rect,clip_point,setup_blit,setup_plot,clip_line


	text

* clip_rect - Internal function
*
* Clips coordinates according to currect clip settings
* Todo: Always clip against screen area?
* In:   d1,d2   x1,y1
*       d3,d4   x2,y2
* Out:  d1,d2   clipped x1,y1
*       d3,d4   clipped x2,y2
*       flags   set for BLO if no area left
clip_rect:
  ifeq always_clip_r
	tst.w	vwk_clip_on(a0)
	beq	.no_clip
  endc
.do_clip:
	cmp.w	vwk_clip_rectangle_x1(a0),d1	; left
	bge	.left_ok
	move.w	vwk_clip_rectangle_x1(a0),d1
.left_ok:
	cmp.w	vwk_clip_rectangle_y1(a0),d2	; top
	bge	.top_ok
	move.w	vwk_clip_rectangle_y1(a0),d2
.top_ok:
	cmp.w	vwk_clip_rectangle_x2(a0),d3	; right
	ble	.right_ok
	move.w	vwk_clip_rectangle_x2(a0),d3
.right_ok:
	cmp.w	vwk_clip_rectangle_y2(a0),d4	; bottom
	ble	.bottom_ok
	move.w	vwk_clip_rectangle_y2(a0),d4
.bottom_ok:
	cmp.w	d1,d3
	blt	.end
	cmp.w	d2,d4
	blt	.end
.end:
	rts
.no_clip:
	rts


* clip_point - Internal function
*
* 'Clips' coordinates according to currect clip settings
* Todo: Always clip against screen area?
* In:   d1,d2   x,y
* Out:  d1,d2   'clipped' x,y
*       flags   set for BLO if no area left
clip_point:
  ifeq always_clip_p
	tst.w	vwk_clip_on(a0)
	beq	.clip_done
  endc
	cmp.w	vwk_clip_rectangle_x1(a0),d1	; left
	blt	.outside
	cmp.w	vwk_clip_rectangle_x2(a0),d1	; right
	bgt	.outside
	cmp.w	vwk_clip_rectangle_y1(a0),d2	; top
	blt	.outside
	cmp.w	vwk_clip_rectangle_y2(a0),d2	; bottom
	bgt	.outside
.clip_done:
	rts					; LE for not clipped!
.outside:
	move	#0,ccr				; Set GT for clipped!
	rts


* setup_blit - Internal function
*
* Sets up pointers to pixel blit functions
* In:	a0	VDI struct
*	4(a7)	drawing mode
* Out:	a1	blit function
*	a3	set pixel function
*	a4	get pixel function
setup_blit:
	move.l	vwk_real_address(a0),a1
	move.l	wk_r_set_pixel(a1),a3
	move.l	wk_r_get_pixel(a1),a4
	lea	blit_routines,a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	move.l	(a1),a1
	rts

* blit_0 - Internal function
*
* D1 = 0 (clear destination block)
* In:	a0	-> VDI struct, destination MFDB, VDI struct, source MFDB
*	d1	Source x (high), destination x (low)
*	d2	Source y (high), destination y (low)
*	a3	set pixel function
*	a4	get pixel function
blit_0:
	moveq	#0,d0
	jsr	(a3)
	rts

* blit_1 - Internal function
*
* D1 = S and D
blit_1:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	and.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_2 - Internal function
*
* D1 = S and (not D)
blit_2:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	not.w	d0
	and.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_3 - Internal function
*
* D1 = S (replace mode)
blit_3:
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	jsr	(a3)
	rts

* blit_4 - Internal function
*
* D1 = (not S) and D (erase mode)
blit_4:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	not.w	d7
	jsr	(a4)
	and.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_5 - Internal function
*
* D1 = D (no operation)
blit_5:
	rts

* blit_6 - Internal function
*
* D1 = S xor D (XOR mode)
blit_6:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	eor.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_7 - Internal function
*
* D1 = S or D (transparent mode)
blit_7:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	or.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_8 - Internal function
*
* D1 = not (S or D)
blit_8:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	or.w	d7,d0
	not.w	d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_9 - Internal function
*
* D1 = not (S xor D)
blit_9:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	eor.w	d7,d0
	not.w	d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_a - Internal function
*
* D1 = not D
blit_a:
	jsr	(a4)
	not.w	d0
	jsr	(a3)
	rts	

* blit_b - Internal function
*
* D1 = S or (not D)
blit_b:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	not.w	d0
	or.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_c - Internal function
*
* D1 = not S
blit_c:
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	not.w	d0
	jsr	(a3)
	rts

* blit_d - Internal function
*
* D1 = (not S) or D (reverse transparent mode)
blit_d:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	not.w	d7
	jsr	(a4)
	or.w	d7,d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_e - Internal function
*
* D1 = not (S and D)
blit_e:
	move.l	d7,-(a7)
	swap	d1
	swap	d2
	addq.l	#8,a0
	jsr	(a4)
	subq.l	#8,a0
	swap	d1
	swap	d2
	move.w	d0,d7
	jsr	(a4)
	and.w	d7,d0
	not.w	d0
	jsr	(a3)
	move.l	(a7)+,d7
	rts	

* blit_f - Internal function
*
* D1 = 1 (fill destination block)
blit_f:
	moveq	#-1,d0
	jsr	(a3)
	rts
 

* setup_plot - Internal function
*
* Sets up pointers to pixel draw functions
* In:	a0	VDI struct
*	4(a7)	drawing mode
* Out:	a1	draw function
*	a3	set pixel function
*	a4	get pixel function
setup_plot:
	move.l	vwk_real_address(a0),a1
	move.l	wk_r_set_pixel(a1),a3
	move.l	wk_r_get_pixel(a1),a4
	lea	mode_routines,a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	add.w	4(a7),a1
	move.l	-4(a1),a1
	rts

* p_replace - Internal function
*
* Draws in replace mode
* In:	a0	-> VDI struct, destination MFDB
*	d0	background.w and foreground.w colour
*	d1	x
*	d2	y
*	a3	set pixel function
*	a4	get pixel function
*	carry	current mask bit
p_replace:
	bcc	.background
	jsr	(a3)
	rts
.background:
	swap	d0
	jsr	(a3)
	swap	d0
	rts

* p_transp - Internal function
*
* Draws in transparent mode
p_transp:
	bcc	.nothing
	jsr	(a3)
.nothing:
	rts

* p_xor - Internal function
*
* Draws in xor mode
* I don't think this does the right thing!
p_xor:
	lbcc	.nothing,1
	move.l	d0,-(a7)
;	move.w	d0,-(a7)
	jsr	(a4)
;	eor.w	d0,(a7)		; No convenient addressing mode!
;	move.w	(a7)+,d0
	not.w	d0		; Is this right instead perhaps?
	jsr	(a3)
	move.l	(a7)+,d0
 label .nothing,1
	rts

* p_revtransp - Internal function
*
* Draws in reverse transparent mode
p_revtransp:
	lbcs	.nothing,1
	jsr	(a3)
 label .nothing,1
	rts


* clip_line - Internal function
*
* Clips line coordinates
* Todo:	?
* In:	a0	VDI struct
*	d1-d2	x1,y1
*	d3-d4	x2,y2
* Out:	d1-d4	clipped coordinates
clip_line:
	movem.l		d0/d5-d7,-(a7)
  ifeq always_clip_l
	tst.w		vwk_clip_on(a0)
	beq		.end_clip
  endc

	moveq		#0,d0		; Coordinate flip flag
	move.w		vwk_clip_rectangle_x1(a0),d6
	move.w		vwk_clip_rectangle_y1(a0),d7
	sub.w		d6,d1		; Change to coordinates
	sub.w		d7,d2		;  relative to the clip rectangle
	sub.w		d6,d3
	sub.w		d7,d4

;	move.w		clip_h(pc),d7
	move.w		vwk_clip_rectangle_y2(a0),d7
	sub.w		vwk_clip_rectangle_y1(a0),d7	; d7 - max y-coordinate of clip rectangle
;	addq.w		#1,d7		; d7 - height of clip rectangle
	cmp.w		d2,d4
	bge.s		.sort_y1y2	; Make sure max y-coordinate for <d3,d4>  (Was 'bpl')
	exg		d1,d3
	exg		d2,d4
	not.w		d0		; Mark as flipped

.sort_y1y2:
	cmp.w		d7,d2
	bgt		.error		; All below screen?  (Was 'bpl', with d7++)
	move.w		d4,d6
	blt		.error		; All above screen?  (Was 'bmi')
	sub.w		d2,d6		; d6 = dy (pos)
	move.w		d3,d5
	sub.w		d1,d5		; d5 = dx
	bne.s		.no_vertical

	tst.w		d2		; Clip vertical
	bge.s		.y1in		; Line to top/bottom  (Was 'bpl')
	moveq		#0,d2
.y1in:	cmp.w		d7,d4
	ble.s		.vertical_done	; (Was 'bmi', with d7++)
	move.w		d7,d4
	bra.s		.vertical_done

.no_vertical:
	tst.w		d2
	bge.s		.y1_inside	; (Was 'bpl')
	muls.w		d5,d2		; dx * (y1 - tc)
	divs.w		d6,d2		; dx * (y1 - tc) / (y2 - y1)
	sub.w		d2,d1		; x1' = x1 - (dx * (y1 - tc)) / (y2 - y1)
	moveq		#0,d2		; y1' = ty

.y1_inside:
	sub.w		d4,d7
	bge.s		.y2_inside	; (Was 'bpl', with d7++ (which was probably wrong))
; Is a d7++ needed now when it isn't there above?
	muls.w		d7,d5		; dx * (bc - y2)
	divs.w		d6,d5		; dx * (bc - y2) / (y2 - y1)
	add.w		d5,d3		; x2' = x2 + (dx * (bc - y2)) / (y2 - y1)
	add.w		d7,d4		; y2' = by

.y2_inside:
.vertical_done:
;	move.w		clip_w(pc),d7
	move.w		vwk_clip_rectangle_x2(a0),d7
	sub.w		vwk_clip_rectangle_x1(a0),d7	; d7 - max x-coordinate of clip rectangle
;	addq.w		#1,d7		; d7 - width of clip rectangle
	cmp.w		d1,d3
	bge.s		.sort_x1x2	; Make sure max x-coordinate for <d3,d4>  (Was 'bpl')
	exg		d1,d3
	exg		d2,d4
	not.w		d0		; Mark as flipped

.sort_x1x2:
	cmp.w		d7,d1
	bgt		.error		; All right of screen?  (Was 'bpl', with d7++)
	move.w		d3,d5
	blt		.error		; All left of screen?   (Was 'bmi')
	sub.w		d1,d5		; d5 = dx (pos)
	move.w		d4,d6
	sub.w		d2,d6		; d6 = dy
	bne.s		.no_horizontal

	tst.w		d1		; Clip horizontal
	bge.s		.x1in		; Line to left/right  (Was 'bpl')
	moveq		#0,d1
.x1in:	cmp.w		d7,d3
	ble.s		.horizontal_done	; (Was 'bmi', with d7++)
	move.w		d7,d3
	bra.s		.horizontal_done

.no_horizontal:
	tst.w		d1
	bge.s		.x1_inside	; (Was 'bpl')
	muls.w		d6,d1		; dy * (x1 - lc)
	divs.w		d5,d1		; dy * (x1 - lc) / (x2 - x1)
	sub.w		d1,d2		; y1' = y1 - (dy * (x1 - lc)) / (x2 - x1)
	moveq		#0,d1		; x1' = lc

.x1_inside:
	sub.w		d3,d7
	bge.s		.x2_inside	; (Was 'bpl', with d7++ (which was probably wrong))
; Is a d7++ needed now when it isn't there above?
	muls.w		d7,d6		; dy * (rc - x2)
	divs.w		d5,d6		; dy * (rc - x2) / (x2 - x1)
	add.w		d6,d4		; y2' = y2 + (dx * (bc - y2)) / (y2 - y1)
	add.w		d7,d3		; x2' = rc

.x2_inside:
.horizontal_done:
	move.w		vwk_clip_rectangle_x1(a0),d6
	move.w		vwk_clip_rectangle_y1(a0),d7
	add.w		d6,d1		; Change back to real coordinates
	add.w		d7,d2
	add.w		d6,d3
	add.w		d7,d4

	tst.w		d0
	beq		.end_clip
	exg		d1,d3		; Flip back again if needed
	exg		d2,d4
.end_clip:
	movem.l		(a7)+,d0/d5-d7
	rts

.error:
	movem.l		(a7)+,d0/d5-d7
	move.w		#2,-(a7)	; Return with the overflow flag set
	rtr


	data

mode_routines:
	dc.l	p_replace,p_transp,p_xor,p_revtransp

blit_routines:
	dc.l	blit_0,blit_1,blit_2,blit_3,blit_4,blit_5,blit_6,blit_7
	dc.l	blit_8,blit_9,blit_a,blit_b,blit_c,blit_d,blit_e,blit_f

	end
