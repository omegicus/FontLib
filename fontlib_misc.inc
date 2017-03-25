; esi=edi supported (32bpp)
italic:
; esi -> source buffer
; edi -> result buffer
; ebx = height
; ecx = width
	shl	ecx,2
	shr	ebx,2
	mov	eax,ecx
	mul	ebx
	shl	eax,2
	add	esi,eax
	add	edi,eax
	dec	ecx
	sub	esi,8
	sub	edi,4
	push	ebx
	std
@@:
	push	ecx
	rep movsd
	pop	ecx
	sub	esi,4
	dec	ebx
	jnz	@b
	pop	ecx
	mov	eax,[edi+4]
	rep stosd
	cld
	ret


; vertical downscale
; white-black-gray only
; esi=edi supported (32bpp)
verScale:
; esi -> source buffer
; edi -> result buffer
; eax = width
; ebx = source height
; edx = result height
	push	ebp
	dec	eax
	shl	eax,2
	push	eax
	add	eax,4
	push	edx
	push	esi
	push	edi
	push	eax
	mov	ecx,edx
.scale:
	mov	al, [esi]
	add	esi,[esp]
	mul	cl
	neg	ecx
	add	ecx,ebx
	mov	ebp,eax
	mov	al, [esi]
@@:
	cmp	edx,ecx
	jnc	@f
	add	esi,[esp]
	mul	dl
	sub	ecx,edx
	add	ebp,eax
	mov	al, [esi]
	jmp	@b
@@:
	mul	cl
	add	eax,ebp
	div	bl
	mov	[edi],al
	mov	[edi+1],al
	mov	[edi+2],al
	add	edi,[esp]
	neg	ecx
	add	ecx,edx
	jnz	@f
	add	ecx,edx
	add	esi,[esp]
@@:
	dec	dword[esp+12]
	jnz	.scale
	mov	edi,[esp+4]
	mov	esi,[esp+8]
	mov	[esp+12],edx
	add	edi,[esp+16]
	add	esi,[esp+16]
	sub	dword[esp+16],4
	jnc	.scale
	add	esp,20
	pop	ebp
	ret


; horizontal downscale
; minimum — x3, maximum — x6
; white-black-gray only
; esi=edi supported
ClearType:
; esi -> source buffer (32bpp)
; edi -> result buffer (24bpp)
; eax = height
; edx = source width
; ebx = result width
	push	ebp
	lea	ebx,[ebx*2+ebx]
	imul	eax,ebx
	push	eax
	push	edi
	push	eax
	push	edx
	mov	ecx,ebx
.scale:
	movzx	eax,byte[esi]
	add	esi,4
	mul	ecx
	neg	ecx
	add	ecx,[esp]
	mov	ebp,eax
	movzx	eax,byte[esi]
	cmp	ebx,ecx
	jnc	@f
	add	esi,4
	mul	ebx
	sub	ecx,ebx
	add	ebp,eax
	movzx	eax,byte[esi]
@@:
	mul	ecx
	add	eax,ebp
	div	dword[esp]
	stosb
	neg	ecx
	add	ecx,ebx
	jnz	@f
	add	ecx,ebx
	add	esi,4
@@:
	dec	dword[esp+4]
	jnz	.scale
	pop	edi
	pop	edi
	mov	edi,[esp]
	mov	ecx,[esp+4]
	movzx	ebx,byte[edi]
	xor	eax,eax
	dec	ecx
.degradation:
	mov	al, [edi]
	shl	eax,1
	lea	eax,[eax*2+eax]
	lea	edx,[ebx*4+ebx]
	mov	bl, [edi+1]
	add	eax,edx
	lea	edx,[ebx*4+ebx]
	mov	bl, [edi]
	add	eax,edx
	shr	eax,4
	stosb
	dec	ecx
	jnz	.degradation
	pop	edi
	pop	ecx
.colRev:
	mov	al,[edi]
	xchg	al,[edi+2]
	mov	[edi],al
	add	edi,3
	sub	ecx,3
	jnz	.colRev
	pop	ebp
	ret


; apply color on font, put font on picture
; white font on black background only, smoothing allowed
putOnPicture:
; esi -> font buffer (24bpp)
; edi -> picture buffer
; ebx = font color
; ecx = width
; edx = height
; eax = picture buffer line gap in bytes
; ebp = picture buffer bytes per pixel
	push	edx
	push	eax
	push	ecx
	push	ebp
	xor	eax,eax
	rol	ebx,8
	mov	ebp,ecx
.start:
	cmp	byte[esi], 0
	jz	@f
	mov	al, [esi]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	mul	bh
	mov	edx,eax
	mov	al, [edi]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi],al
@@:
	cmp	byte[esi+1], 0
	jz	@f
	mov	al, [esi+1]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	rol	ebx,16
	mul	bl
	rol	ebx,16
	mov	edx,eax
	mov	al, [edi+1]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi+1],al
@@:
	cmp	byte[esi+2], 0
	jz	@f
	mov	al, [esi+2]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	rol	ebx,16
	mul	bh
	rol	ebx,16
	mov	edx,eax
	mov	al, [edi+2]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi+2],al
@@:
	add	esi,3
	add	edi,[esp]
	dec	ebp
	jnz	.start
	mov	ebp,[esp+4]
	add	edi,[esp+8]
	dec	dword[esp+12]
	jnz	.start
	add	esp,16
	ret

align 4
@EXPORT:
export	drawText,	'drawText', \
	countUTF8,	'cntUTF-8', \
	charsFit,	'charsFit', \
	stringWidth,	'strWidth'
