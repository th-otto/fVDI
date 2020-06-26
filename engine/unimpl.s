*****
* fVDI unimplemented functions
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?


*****
*
* What about:
* v_font   Lattice manual p260
* v_offset Lattice manual p267
*
*****
	.include	"vdi.inc"
	.include	"macros.inc"

	xref	redirect
	xref	_no_vex

	xdef	nothing
	xdef	v_clrwk,v_updwk
	xdef	vrq_locator,vrq_valuator,vrq_choice,vsin_mode
	xdef	v_contourfill
	xdef	vqin_mode
	xdef	v_cellarray,vq_cellarray
	xdef	vst_name,vst_width
	xdef	v_getoutline,vst_scratch
	xdef	vst_error,v_savecache
	xdef	v_loadcache,v_flushcache,vst_setsize,vqt_cachesize
	xdef	vqt_get_table
	xdef	v_set_app_buff
	xdef	vq_tabstatus,v_hardcopy,v_rmcur,v_form_adv
	xdef	v_output_window,v_clear_disp_list,v_bit_image
	xdef	start_unimpl,end_unimpl
	xdef	special_5,special_11
	xdef	v_kill_outline


	text

start_unimpl:

* xxxx - Standard Trap function
* Unimplemented traps
* Todo: Implement them!

* No known function
nothing:
special_5:
special_11:
	bra	redirect

* Physical workstation manipulation
v_clrwk:
v_updwk:
	done_return

* Strange mouse/keyboard functions
vrq_locator:
vrq_valuator:
vrq_choice:
;vrq_string:
vqin_mode:
	move.l	intout(a1),a2
	clr.w	(a2)
	bra	redirect

vsin_mode:			; An experiment
	move.l	intin(a1),a0
	move.w	2(a0),d1	; Mode
	move.w	0(a0),a0	; Device
	add.l	#setting,a0
	cmp.b	0(a0),d1
	beq	.done
	move.b	d1,0(a0)
	bra	redirect
.done:
	done_return


	data
setting:
	dc.b	-1,-1,-1,-1,-1,0


	text

vq_cellarray:
	bra	redirect

v_kill_outline:
	done_return
	
vst_name:
vst_width:
v_getoutline:
vst_scratch:
vst_error:
;vq_devinfo:
v_savecache:
v_loadcache:
v_flushcache:
vst_setsize:
vqt_cachesize:
vqt_get_table:
v_set_app_buff:
	done_return

vq_tabstatus:
v_hardcopy:
v_rmcur:
v_form_adv:
v_output_window:
v_clear_disp_list:
v_bit_image:
	bra	redirect

v_contourfill:
v_cellarray:

  ifne 0
v_rbox:
v_rfbox:
	done_return
  endc

end_unimpl:

	end
