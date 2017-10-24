#NO_APP
	.text
	.even
	.globl	_test
_test:
	link.w %fp,#0
	move.l 8(%fp),%a0
	move.w (%a0),_x
	move.w 4(%a0),_y
	unlk %fp
	rts
.comm _y,2
.comm _x,2
