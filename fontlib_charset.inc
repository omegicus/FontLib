cp866toUpper:
; convert cp866 character in al to uppercase
	cmp	al, 'a'
	jb	.ret
	cmp	al, 'z'
	jbe	@f
	cmp	al, 0xA0
	jb	.ret
	cmp	al, 0xB0
	jb	@f
	cmp	al, 0xE0
	jb	.ret
	cmp	al, 0xF0
	jb	.rus
	cmp	al, 0xF7
	ja	.ret
	and	eax, -2
.ret:
	ret

@@:
	sub	eax, 32
	ret

.rus:
	sub	eax, 0xE0-0x90
	ret

utf16toUpper:
; convert UTF-16 character in ax to uppercase
	cmp	ax, 'a'
	jb	.ret
	cmp	ax, 'z'
	jbe	@f
	cmp	ax, 430h
	jb	.ret
	cmp	ax, 450h
	jb	@f
	cmp	ax, 460h
	jnc	.ret
	sub	eax, 80
.ret:
	ret

@@:
	sub	eax, 32
	ret

uni2ansi_char:
; convert UNICODE character in ax to ANSI character in al using cp866 encoding
	cmp	ax, 0x80
	jb	.ret
	cmp	ax, 0xB6
	jz	.B6
	cmp	ax, 0x400
	jb	.unk
	cmp	ax, 0x410
	jb	@f
	cmp	ax, 0x440
	jb	.rus1
	cmp	ax, 0x450
	jb	.rus2
	cmp	ax, 0x460
	jb	@f
.unk:
	mov	al, '_'
.ret:
	ret

.B6:
	mov	al, 20
	ret

.rus1:	; 0x410-0x43F -> 0x80-0xAF
	add	al, 0x70
	ret

.rus2:	; 0x440-0x44F -> 0xE0-0xEF
	add	al, 0xA0
	ret

@@:
	push	ecx edi
	mov	ecx, 8
	mov	edi, .table
	repnz scasb
	mov	ah, cl
	pop	edi ecx
	jnz	.unk
	mov	al, 0xF7
	sub	al, ah
	ret

.table	db  1, 51h, 4, 54h, 7, 57h, 0Eh, 5Eh

ansi2uni_char:
; convert ANSI character in al to UNICODE character in ax, using cp866 encoding
	movzx	eax, al
	cmp	al, 0x80
	jb	@f	; 0x00-0x7F - trivial map
	cmp	al, 0xB0
	jb	.rus	; 0x80-0xAF -> 0x410-0x43F
	cmp	al, 0xE0
	jb	.unk
	cmp	al, 0xF0
	jb	.rus2	; 0xE0-0xEF -> 0x440-0x44F
	cmp	al, 0xF8
	jnc	.unk
	mov	al, [eax+uni2ansi_char.table-0xF0]
	add	ax, 400h
	ret

@@:
	cmp	al, 20
	jnz	.ret
	mov	al, 0xB6
.ret:
	ret

.rus:
	add	ax, 0x410-0x80
	ret

.rus2:
	add	ax, 0x440-0xE0
	ret

.unk:
	mov	al, '_'
	ret

cp866toUTF8_string:
;   in:
; esi -> cp866 string (could be zero terminated)
; edi -> buffer for UTF-8 string
; ecx = buffer size (signed)
	lodsb
	call	ansi2uni_char
	push	eax
	call	UTF16to8
	pop	eax
	js	@f
	test	eax, eax
	jnz	cp866toUTF8_string
@@:
	ret

; SF=1 -> counter
; ZF=1 -> zero char

UTF16to8_string:
;   in:
; esi -> UTF-16 string (could be zero terminated)
; edi -> buffer for UTF-8 string
; ecx = buffer size (signed)
	xor	eax, eax
@@:
	lodsw
	push	eax
	call	UTF16to8
	pop	eax
	js	@f
	test	eax, eax
	jnz	@b
@@:
	ret

UTF16to8:
;   in:
; eax = UTF-16 char
; edi -> buffer for UTF-8 char (increasing)
; ecx = byte counter (decreasing)
	dec	ecx
	js	.ret
	cmp	eax, 80h
	jnc	@f
	stosb
	test	eax, eax    ; SF=0
.ret:
	ret

@@:
	dec	ecx
	js	.ret
	cmp	eax, 800h
	jnc	@f
	shl	eax, 2
	shr	al, 2
	or	eax, 1100000010000000b
	xchg	al, ah
	stosw
	ret

@@:
	dec	ecx
	js	.ret
	shl	eax, 4
	shr	ax, 2
	shr	al, 2
	or	eax, 111000001000000010000000b
	bswap	eax
	shr	eax, 8
	stosb
	shr	eax, 8
	stosw
	ret


utf8to16:
; in: esi -> UTF-8 char (increasing)
; out: ax = UTF-16 char
	lodsb
	test	al, al
	jns	.got
	shl	al, 2
	jnc	utf8to16
@@:
	shl	ax, 8
	lodsb
	test	al, al
	jns	.got
	shl	al, 2
	jc	@b
	shr	ah, 2
	shl	ax, 3
	jnc	@f
	shl	eax, 3
	lodsb
	test	al, al
	jns	.got
	shl	al, 2
	jc	@b
	shr	eax, 2
	ret

@@:
	shr	ax, 5
	ret

.got:
	xor	ah, ah
	ret


align 4
translate8:
	mov	byte al, [ecx]
	cmp	al, 0
	je	_translate8_end
	cmp	al, 127
	jb	_translate8_copy
	and	eax, 127
	mov	ebx, [charset]
	add	ebx, eax	
	mov	byte al, [ebx]
	_translate8_copy:
		mov	byte [edx], al
		inc	edx
		inc	ecx
		jmp	translate8
	_translate8_end:
		mov	byte [edx], 0
		ret

;  (const char text_in[], char text_out[])
align 4
dos2win:
	push	ecx
	push	edx
	mov	ecx, [esp+04]
	mov	edx, [esp+08]
	pushad
	mov	ebx, d2w
	mov	[charset], ebx
	call	translate8
	popad
	pop	edx
	pop	ecx
	ret	4*2

;  (const char text_in[], char text_out[])
align 4
win2dos:
	push	ecx
	push	edx
	mov	ecx, [esp+04]
	mov	edx, [esp+08]
	pushad
	mov	ebx, w2d
	mov	[charset], ebx
	call	translate8
	popad
	pop	edx
	pop	ecx
	ret	4*2

; void __stdcall koi2dos (const char text_in[], char text_out[])
align 4
koi2dos:
	push	ecx
	push	edx
	mov	ecx, [esp+04]
	mov	edx, [esp+08]
	pushad
	mov	ebx, k2d
	mov	[charset], ebx
	call	translate8
	popad
	pop	edx
	pop	ecx
	ret	4*2


align 4
cp1251_to_866_char: ; al
       @@: cmp	al,0xc0
	   jb  .1
	   cmp	al,0xef
	   ja  .3
	   xor	al,0x40
	   jmp .2
       .1: cmp	al,0xa8
	   jnz .3
	   mov	al,0xf0
	   jmp .2
       .3: cmp	al,0xb8
	   jnz .4
	   mov	al,0xf1
	   jmp .2
       .4: cmp	al,0xf0
	   jb  .2
	   xor	al,0x10
       .2: ret