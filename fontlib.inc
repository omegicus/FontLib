TRANSPARENT =  0x00FF00FF

CP_866	    = 0
CP_1251     = 1
CP_UTF8     = 2

al_color = 0x00D2D2D2
sl_color = 0x00444444

FL_ANTIALIAS = 01000000000000000000000000000000b
FL_ANTIALIAS_BT = 30

align 4 	  ; 4  8  12	16   20 				  24	28    32	flags
screen_settextxy: ; x, y, text, col, background (0x00FF00FF=transparent), font, size, codepage, 36
; col: SxRRGGBB, s=0 raw, 1=anti-aliasing , 2=subpixel
	;STEXTW = 8
	mov	ebp, esp	      ; save pointer

	movzx	eax,  word[ebp + 28]
	mov	dword[STEXTH], eax
	movzx	eax,  word[ebp + 30]
	mov	dword[STEXTW], eax

	mov	esi, dword[ebp + 12]

  .next_char:
	lodsb
	or	al,  al
	jz     .exit
	cmp	al, ' '
	je     .space
	cmp	al,  0x0A
	jne	@f

    @@:

	cmp	dword[ebp + 32], CP_1251
	je	@f
	call	cp1251_to_866_char		 ; -> al
    @@: and	eax, 0x000000FF


	cmp	dword[ebp + 28], 0x00080008
	je     .shl3
	cmp	dword[ebp + 28], 0x00080010
	je     .shl4
	jmp    .exit
     .shl3:
	shl	eax, 3				 ; *8
	jmp    .fntptr
     .shl4:
	shl	eax, 4				 ; *8
	jmp    .fntptr


     .fntptr:
	mov	edi, dword[esp + 24]		    ; FONT 8x8 (2048 bytes)
	add	edi, eax

	xor	ecx, ecx
  .next_row:
	mov	 al, byte[edi + ecx]
	mov	edx, [STEXTW]
  .next_col:
	dec	edx
	bt	eax, edx
	jnc    .antialias
	;
	pushad
	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	mov	ecx, dword[ebp + 16]
	call	putpixel	; EAX=X EBX=Y ECX=Color
	popad
	jmp    .antialias
	;
    .skip:
	inc	dword[ebp + 04]
	cmp	edx,  0
	jne    .next_col

	mov	eax, [STEXTW]
	sub	dword[ebp + 04], eax;STEXTW

	mov	eax, [STEXTH]
	inc	dword[ebp + 08]
	inc	ecx
	cmp	ecx, eax;[STEXTW]
	jne    .next_row

	mov	eax, [STEXTW]
	dec	eax
	add	dword[ebp + 04], eax;STEXTW-1	 ; add X

	mov	eax, [STEXTH]
	sub	dword[ebp + 08], eax;STEXTW    ; sub Y
	jmp    .next_char


    .antialias:

	bt	dword[ebp + 36], FL_ANTIALIAS_BT
	jnc    .skip

      ;  cmp	 dword[ebp + 20], TRANSPARENT
      ;  je	.transparent

      ;  pushad
      ;  mov	 eax, dword[ebp + 04]
      ;  mov	 ebx, dword[ebp + 08]
      ;  mov	 ecx, dword[ebp + 16] ; col
      ;  call	 putpixel	 ; EAX=X EBX=Y ECX=Color
      ;  popad

   .transparent:
	; ANTIALIASING:
	or	ecx, ecx
	jz     .skip
	pushad
	mov	byte[aa_sq + 0], 0
	mov	byte[aa_sq + 1], 0
	mov	byte[aa_sq + 2], 0
	mov	byte[aa_sq + 3], 0

	bt	eax, edx
	jnc	@f
	mov	byte[aa_sq + 2], 0xFF

    @@: dec	edx
	bt	eax, edx
	jnc	@f
	mov	byte[aa_sq + 3], 0xFF

    @@: inc	edx
	mov	 al, byte[edi + ecx - 1]
	bt	eax, edx
	jnc	@f
	mov	byte[aa_sq + 0], 0xFF

    @@: dec	edx
	bt	eax, edx
	jnc	@f
	mov	byte[aa_sq + 1], 0xFF

    @@: inc	edx
	mov	 al, byte[edi + ecx]

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	call	aa_putpixel
	popad

	jmp    .skip

    .space:
	add	dword[ebp + 04], 3;    ; add X
	jmp    .next_char

 .exit:
ret 4*9



aa_putpixel: ; EAX=X, EBX=Y	aa_sq=SquareArray
	cmp	byte [aa_sq + 0], 0x0
	jne    .invertcheck
	cmp	byte [aa_sq + 3], 0x0
	jne    .invertcheck
	cmp	byte [aa_sq + 1], 0xFF
	jne    .invertcheck
	cmp	byte [aa_sq + 2], 0xFF
	jne    .invertcheck

	pushad
	mov	eax, dword[ebp + 04]					 ; 0 1	0 1
	mov	ebx, dword[ebp + 08]					 ;*2 3	1 0
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad

  .invertcheck:
	cmp	byte [aa_sq + 2], 0x0
	jne    .ncheck_2
	cmp	byte [aa_sq + 1], 0x0
	jne    .ncheck_2
	cmp	byte [aa_sq + 0], 0xFF
	jne    .ncheck_2
	cmp	byte [aa_sq + 3], 0xFF
	jne    .ncheck_2
	pushad
	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad



  .ncheck_2:

	cmp	byte [aa_sq + 0], 0xFF
	jne    .ncheck_3
	cmp	byte [aa_sq + 3], 0xFF
	jne    .ncheck_3
	cmp	byte [aa_sq + 1], 0xFF
	jne    .ncheck_3
	cmp	byte [aa_sq + 2], 0x00
	jne    .ncheck_3

	pushad
	mov	eax, dword[ebp + 04]					 ; 0 1	1 1
	mov	ebx, dword[ebp + 08]					 ;*2 3	0 1
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad



  .ncheck_3:

	cmp	byte [aa_sq + 0], 0x00
	jne    .ncheck_4
	cmp	byte [aa_sq + 3], 0xFF
	jne    .ncheck_4
	cmp	byte [aa_sq + 1], 0xFF
	jne    .ncheck_4
	cmp	byte [aa_sq + 2], 0xFF
	jne    .ncheck_4

	pushad
	mov	eax, dword[ebp + 04]					 ; 0 1	0 1
	mov	ebx, dword[ebp + 08]					 ;*2 3	1 1
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad


  .ncheck_4:

	cmp	byte [aa_sq + 0], 0xFF
	jne    .skip
	cmp	byte [aa_sq + 3], 0xFF
	jne    .skip
	cmp	byte [aa_sq + 1], 0x00
	jne    .skip
	cmp	byte [aa_sq + 2], 0xFF
	jne    .skip

	pushad
	mov	eax, dword[ebp + 04]					 ; 0 1	1 0
	mov	ebx, dword[ebp + 08]					 ;*2 3	1 1
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad

  .skip:
ret

aa_putpixel2: ; EAX=X, EBX=Y	 aa_sq=SquareArray
	cmp	byte [aa_sq + 3], 0x0
	jne    .invertcheck
	cmp	byte [aa_sq + 1], 0xFF
	jne    .invertcheck
	cmp	byte [aa_sq + 2], 0xFF
	jne    .invertcheck
	pushad
	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	inc	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

      ;  mov	 eax, dword[ebp + 04]
      ;  inc	 eax
      ;  mov	 ebx, dword[ebp + 08]
      ;  mov	 ecx, sl_color
      ;  call	 putpixel	 ; EAX=X EBX=Y ECX=Color
      ;
      ;  mov	 eax, dword[ebp + 04]
      ;  mov	 ebx, dword[ebp + 08]
      ;  inc	 ebx
      ;  mov	 ecx, sl_color
      ;  call	 putpixel	 ; EAX=X EBX=Y ECX=Color

	popad

  .invertcheck:
	cmp	byte [aa_sq + 1], 0x0
	jne    .skip
	cmp	byte [aa_sq + 0], 0xFF
	jne    .skip
	cmp	byte [aa_sq + 3], 0xFF
	jne    .skip
	pushad
	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, al_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	inc	eax
	mov	ebx, dword[ebp + 08]
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	mov	eax, dword[ebp + 04]
	mov	ebx, dword[ebp + 08]
	dec	ebx
	mov	ecx, sl_color
	call	putpixel	; EAX=X EBX=Y ECX=Color

	popad


  .skip:
ret
