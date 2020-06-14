*============================================*
*   System routines modified from BAD MOOD   *
*============================================*

	.include	"macros.inc"

	xref		_sub_call

	xdef		_appl_init,_appl_exit
	xdef		_vq_extnd,_vq_color
	xdef		_graf_handle
	xdef		_call_v_opnwk,_call_v_opnvwk,_call_v_clsvwk
	xdef		_scall_v_opnwk,_scall_v_clswk
	xdef		_get_sub_call
	xdef		_set_inout
	xdef		_vdi,_sub_vdi,_fvdi
	xdef		_vq_gdos
	xdef		_linea_fonts

	xdef		_control;
	xdef		_int_in,_pts_in,_addr_in
	xdef		_int_out,_pts_out,_addr_out

	text

* int ap_id = appl_init(void);
* 1 word to int_out
*
_appl_init:
	lea		_control,a0
	move.w		#10,(a0)+
	move.w		#0,(a0)+
	move.w		#1,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	bsr		_AES
	moveq		#0,d0
	move.w		int_out,d0
	rts

* int status = appl_exit(void);
* 1 word to int_out
*
_appl_exit:
	lea		_control,a0
	move.w		#19,(a0)+
	move.w		#0,(a0)+
	move.w		#1,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	clr.w		int_out
	bsr		_AES
	moveq		#0,d0
	move.w		int_out,d0
	rts

* handle = graf_handle();
*
_graf_handle:
	lea		_control,a0
	move.w		#77,(a0)+
	move.w		#0,(a0)+
	move.w		#5,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	bsr		_AES
	moveq		#0,d0
	move.w		int_out,d0
	rts


* handle = call_v_opnwk(long dev_id, short *int_out, short *pts_out);
*
_call_v_opnwk:
	lea		_control,a0
	move.w		#1,(a0)+
	move.w		#0,(a0)+
	addq.l		#2,a0
	move.w		#11,(a0)+
	lea		int_in,a0
	move.l		4(a7),d0
	move.w		d0,(a0)+
	moveq		#1,d0
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		#2,(a0)+
	move.l		8(a7),vdi_int_out_addr
	move.l		12(a7),vdi_pts_out_addr
	bsr		_VDI
	move.l		#int_out,vdi_int_out_addr
	move.l		#pts_out,vdi_pts_out_addr
	moveq		#0,d0
	move.w		_control+12,d0
	rts

* handle = scall_v_opnwk(long dev_id, short *int_out, short *pts_out);
*
_scall_v_opnwk:
	lea		_control,a0
	move.w		#1,(a0)+
	move.w		#0,(a0)+
	addq.l		#2,a0
	move.w		#11,(a0)+
	lea		int_in,a0
	move.l		4(a7),d0
	move.w		d0,(a0)+
	moveq		#1,d0
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		#2,(a0)+
	move.l		8(a7),vdi_int_out_addr
	move.l		12(a7),vdi_pts_out_addr
	bsr		_subVDI
	move.l		#int_out,vdi_int_out_addr
	move.l		#pts_out,vdi_pts_out_addr
	moveq		#0,d0
	move.w		_control+12,d0
	rts

* handle = scall_v_clswk(long handle);
*
_scall_v_clswk:
	lea		_control,a0
	move.w		#2,0(a0)
	move.w		#0,2(a0)
	move.w		#0,6(a0)
	move.l		4(a7),d0
	move.w		d0,12(a0)
	bsr		_subVDI
	moveq		#0,d0
	rts


* handle = call_v_opnvwk(long handle, short *int_out, short *pts_out);
*
_call_v_opnvwk:
	lea		_control,a0
	move.w		#100,(a0)+
	move.w		#0,(a0)+
	move.w		#6,(a0)+
	move.w		#11,(a0)+
	move.w		#45,(a0)+
	move.w		#0,(a0)+
	move.l		4(a7),d0
	move.w		d0,(a0)+
	lea		int_in,a0
	moveq		#1,d0
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		d0,(a0)+
	move.w		#2,(a0)+
	move.l		8(a7),vdi_int_out_addr
	move.l		12(a7),vdi_pts_out_addr
	bsr		_VDI
	move.l		#int_out,vdi_int_out_addr
	move.l		#pts_out,vdi_pts_out_addr
	moveq		#0,d0
	move.w		_control+12,d0
	rts

* void call_v_clsvwk(long handle);
*
_call_v_clsvwk:
	lea		_control,a0
	move.w		#101,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	move.l		4(a7),d0
	move.w		d0,(a0)+
	bsr		_VDI
	rts

* void vq_extnd(long handle, long info_flag, short *int_out, short *pts_out);
*
_vq_extnd:
	lea		_control,a0
	move.w		#102,(a0)+
	move.w		#0,(a0)+
	move.w		#6,(a0)+
	move.w		#1,(a0)+
	move.w		#45,(a0)+
	move.w		#0,(a0)+
	move.l		4(a7),d0
	move.w		d0,(a0)+
	move.l		8(a7),d0
	move.w		d0,int_in
	move.l		12(a7),vdi_int_out_addr
	move.l		16(a7),vdi_pts_out_addr
	bsr		_VDI
	move.l		#int_out,vdi_int_out_addr
	move.l		#pts_out,vdi_pts_out_addr
	rts

* void vq_color(long handle, long colour, long flag, short *int_out);
*
_vq_color:
	lea		_control,a0
	move.w		#26,(a0)+
	move.w		#0,(a0)+
	move.w		#0,(a0)+
	move.w		#2,(a0)+
	move.w		#4,(a0)+
	move.w		#0,(a0)+
	move.l		4(a7),d0
	move.w		d0,(a0)+
	move.l		8(a7),d0
	move.w		d0,int_in
	move.l		12(a7),d0
	move.w		d0,int_in+2
	move.l		16(a7),vdi_int_out_addr
	bsr		_VDI
	move.l		#int_out,vdi_int_out_addr
	rts

* void set_inout(short *int_in, short *pts_in, short *int_out, short *pts_out);
*
_set_inout:
	move.l		4(a7),d0
	lbne		.skip1,1
	move.l		#int_in,d0
 label .skip1,1
	move.l		d0,vdi_int_in_addr
	move.l		8(a7),d0
	lbne		.skip2,2
	move.l		#pts_in,d0
 label .skip2,2
	move.l		d0,vdi_pts_in_addr
	move.l		12(a7),d0
	lbne		.skip3,3
	move.l		#int_out,d0
 label .skip3,3
	move.l		d0,vdi_int_out_addr
	move.l		16(a7),d0
	lbne		.skip4,4
	move.l		#pts_out,d0
 label .skip4,4
	move.l		d0,vdi_pts_out_addr
	rts

* void vdi(long handle, long func, long pts, long ints);
*
_vdi:
	move.l		4(a7),d0
	move.w		d0,_control+12
	move.l		8(a7),d0
	move.w		d0,_control
	move.l		12(a7),d0
	move.w		d0,_control+2
	move.l		16(a7),d0
	move.w		d0,_control+4
	bsr		_VDI
	rts

* void sub_vdi(long handle, long func, long pts, long ints);
*
_sub_vdi:
	move.l		4(a7),d0
	move.w		d0,_control+12
	move.l		8(a7),d0
	move.w		d0,_control
	move.l		12(a7),d0
	move.w		d0,_control+2
	move.l		16(a7),d0
	move.w		d0,_control+4
	bsr		_subVDI
	rts

* void fvdi(long handle, long func, long pts, long ints);
*
_fvdi:
	move.l		4(a7),d0
	move.w		d0,_control+12
	move.l		8(a7),d0
	move.w		d0,_control
	move.l		12(a7),d0
	move.w		d0,_control+2
	move.l		16(a7),d0
	move.w		d0,_control+4
	bsr		_fVDI
	rts


_get_sub_call:
	movem.l		d2/a2,-(a7)	; Necessary?
	moveq		#-1,d0
	trap		#2
	movem.l		(a7)+,d2/a2
	rts


_vq_gdos:
	movem.l		d2/a2,-(a7)	; Necessary?
	moveq		#-2,d0
	trap		#2
	movem.l		(a7)+,d2/a2
	rts


_AES:
	movem.l		d2/a2,-(a7)	; Necessary?
	move.l		#aespb,d1
	move.w		#200,d0
	trap		#2	
	movem.l		(a7)+,d2/a2
	rts

_VDI:
	movem.l		d2/a2,-(a7)	; Necessary?
	move.l		#vdipb,d1
	moveq		#115,d0
	trap		#2	
	movem.l		(a7)+,d2/a2
	rts

_subVDI:
	movem.l		d2/a2,-(a7)	; Necessary?
	move.l		#vdipb,d1
	moveq		#115,d0
	move.l		_sub_call,a2
	jsr		(a2)
	movem.l		(a7)+,d2/a2
	rts

_fVDI:
	movem.l		d2/a2,-(a7)	; Necessary?
	move.l		#vdipb,d1
	move.l		#1969,d0
	trap		#2	
	movem.l		(a7)+,d2/a2
	rts

	data

vdipb:
vdi_control_addr:	dc.l	_control
vdi_int_in_addr:	dc.l	int_in
vdi_pts_in_addr:	dc.l	pts_in
vdi_int_out_addr:	dc.l	int_out
vdi_pts_out_addr:	dc.l	pts_out

aespb:
aes_control_addr:	dc.l	_control
aes_global_addr:	dc.l	global
aes_int_in_addr:	dc.l	int_in
aes_int_out_addr:	dc.l	int_out
aes_addr_in_addr:	dc.l	addr_in
aes_addr_out_addr:	dc.l	addr_out


	bss

_control:
opcode:			ds.w	1
sintin:			ds.w	1
sintout:		ds.w	1
saddrin:		ds.w	1
saddrout:		ds.w	1
			ds.w	6

global:
apversion:		ds.w	1
apcount:		ds.w	1
apid:			ds.w	1
apprivate:		ds.l	1
apptree:		ds.l	1
ap1resv:		ds.l	1	
ap2resv:		ds.l	1
ap3resv:		ds.l	1
ap4resv:		ds.l	1

int_in:
_int_in:		ds.w	16
pts_in:
_pts_in:		ds.w	8
int_out:
_int_out:		ds.w	4
pts_out:
_pts_out:		ds.w	4
addr_in:
_addr_in:		ds.l	4
addr_out:
_addr_out:		ds.l	4

	end
