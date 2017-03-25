format PE GUI 4.0
entry start
include 	    '../fasm/include/win32a.inc'

FL_ANTIALIAS = 01000000000000000000000000000000b


cdXPos		    EQU  10
cdYPos		    EQU  10

cdXSize 	    EQU  640
cdYSize 	    EQU  480

texturesize_X	    EQU  640
texturesize_Y	    EQU  480

const_80	    equ 100	; 80 ; darkness parameter
const_502	    equ texturesize_Y+22	; 502
const_260	    equ texturesize_Y/2 + 20	; 260
const_262	    equ const_260 + 2		; 262

; 64x64:
cx_9		    equ     9	; 8 - for 64x64
cx_3		    equ     3	; 2 - for 64x64
TEX_SIZE	    equ     128*128*4	    ;64*64*4	    ;
ICON_SIZE_X	    equ     128     ;64
ICON_SIZE_Y	    equ     128     ;64
Floors_Height	    equ     32000

cdIdTimer	    EQU  1
CP_866	    = 0
CP_1251     = 1
CP_UTF8     = 2


section '.text' code readable executable
  start:


mov edi,  screen_buffer
mov ecx,  texturesize_X * texturesize_Y * 3
mov eax,  0xFFFFFFFF
cld
rep stosb

stdcall  screen_settextxy, 10, 10, testt, 0x000000, 0xFF00FF, font_plain,   0x00080008, CP_866,  0
stdcall  screen_settextxy, 10, 30, testt, 0x000000, 0xFF00FF, font_slim,    0x00080008, CP_866,  0
stdcall  screen_settextxy, 10, 50, testt, 0x000000, 0xFF00FF, font_type,    0x00080008, CP_866,  0
stdcall  screen_settextxy, 10, 70, testt, 0x000000, 0xFF00FF, font_freebsd, 0x00080008, CP_1251, 0

stdcall  screen_settextxy, 70, 10,  testt, 0x000000, 0xFF00FF, font_plain,   0x00080008, CP_866,  FL_ANTIALIAS
stdcall  screen_settextxy, 70, 30, testt, 0x000000, 0xFF00FF, font_slim,    0x00080008, CP_866,  FL_ANTIALIAS
stdcall  screen_settextxy, 70, 50, testt, 0x000000, 0xFF00FF, font_type,    0x00080008, CP_866,  FL_ANTIALIAS
stdcall  screen_settextxy, 70, 70, testt, 0x000000, 0xFF00FF, font_freebsd, 0x00080008, CP_1251, FL_ANTIALIAS



    invoke    GetModuleHandle,	  NULL
    mov       [wc.hInstance],	  eax
    mov       [wc.lpfnWndProc],   WndProc
    mov       [wc.lpszClassName], NombreClase
    invoke    GetStockObject,	  0;BLACK_BRUSH
    mov       [wc.hbrBackground], 0
    stdcall   WinMain, [wc.hInstance], NULL, NULL, SW_SHOWNORMAL
    invoke    ExitProcess, [wMsg.wParam]


align 4
demo_flush:

    xor      edx, edx
    mov      edi, dword [pMainDIB]
    mov      esi, screen_buffer
    mov      ecx, texturesize_X * texturesize_Y

align 4
@@: lodsw
    stosw
    lodsb
    stosb
    xor      al, al
    stosb
    loop     @b


ret



proc WinMain uses ebx esi edi, hInst, hPrevInst, CmdLine, CmdShow
    invoke    RegisterClass,wc
    test      eax,eax
    jz	     .error
    invoke    CreateWindowEx,NULL,NombreClase,msgTitulo,0x20000+0x80000+0x10000000,cdXPos, cdYPos, cdXSize, cdYSize,  NULL,NULL,[wc.hInstance],NULL
    test      eax,eax
    jz	     .error
 .msg_loop:
    invoke    GetMessage,wMsg,NULL,0,0
    cmp       eax,1
    jb	     .end_loop
    jne      .msg_loop
    invoke    TranslateMessage,wMsg
    invoke    DispatchMessage,wMsg
    jmp      .msg_loop
 .error:
    invoke    MessageBox,NULL,MsgError,NULL,MB_ICONERROR+MB_OK
 .end_loop:
    MOV       EAX, [wMsg.wParam]
    ret
endp








proc WndProc uses ebx esi edi,hWnd,uMsg,wParam,lParam
    mov       eax, [hWnd]
    mov       dword[window], eax
    ;  Propуsito: Procesa los mensajes provenientes de las ventanas
    ;  Entrada	: hwnd,wmsg,wparam,lparam
    ;  Salida	: Ninguna
    ;  Destruye : Ninguna
    mov       eax, [uMsg]
    cmp       eax, WM_ERASEBKGND
    jz	      wmEraseBackground
    cmp       eax, WM_PAINT
    jz	      wmPaint
    cmp       eax, WM_DESTROY
    jz	      wmDestroy
    cmp       eax, WM_CLOSE
    jz	      wmClose
    cmp       eax, WM_TIMER
    jz	      wmTimer
    cmp       eax, WM_CREATE
    jz	      wmCreate
    cmp       eax, WM_KEYDOWN
    je	      wmKeyDown
    cmp       eax, WM_MOUSEMOVE
    je	      wmMouseMove
    wmDefault:
	invoke	  DefWindowProcA, [hWnd], [uMsg], [wParam], [lParam]
	jmp	  wmFin2

    wmMouseMove:
	; wParam MK_LBUTTON==0x0001 LButton down
	;	 MK_MBUTTON==0x0010
	;	 MK_RBUTTON==0x0002
	; lParam low-order word specifies the x-coordinate
	;	 high-order word specifies the y-coordinate
	; The coordinate is relative to the upper-left corner of the client area.

     ;	 mov   eax, dword[lParam]
     ;	 shr   eax, 16
     ;	 mov   dword[miceY], eax
     ;
     ;	 mov   eax, dword[lParam]
     ;	 and   eax, 0x0000FFFF
     ;	 mov   dword[miceX], eax
     ;
     ;	 mov   ebx, 1
     ;	 xor   eax, eax
     ;	 call  demo_events
	jmp   wmFin


    wmKeyDown:
	cmp	  [wParam], VK_ESCAPE	; 1Bh
	je	  wmDestroy

     ;	 mov	   eax,  [lParam]
     ;	 rol	   eax, 16
     ;	 and	   eax, 0x000000FF
     ;	 xor	   ebx, ebx		 ; No mice events
     ;	 call	   demo_events


	jmp	  wmFin
    wmCreate:
	invoke	  GetTickCount
	mov	  [vdSeed], eax
	invoke	  GetDC, [hWnd]
	mov	  [hdc], eax



	; Crea un bъfer dib para el PintaObjeto. pMainDIB es un puntero a йl
	invoke	  CreateCompatibleDC, [hdc]
	mov	  [bufDIBDC], eax
	invoke	  CreateDIBSection, [hdc], LCDBITMAPINFO, 0, pMainDIB, NULL, NULL
	mov	  [hMainDIB], eax
	invoke	  SelectObject, [bufDIBDC], [hMainDIB]	 ; Select bitmap into DC
	mov	  [hOldDIB], eax
	; Create BMP buffer:
	invoke	  CreateCompatibleDC, [hdc]
	mov	  [hBackDC], eax
	invoke	  CreateCompatibleBitmap, [hdc], cdXSize, cdYSize
	mov	  [bufBMP], eax
	invoke	  SelectObject, [hBackDC], [bufBMP]
	mov	  [hOldBmp], eax
	invoke	  ReleaseDC, [hWnd], [hdc]   ; Libera device context







      ;  call	   demo_setup
	invoke	  SetTimer, [hWnd], cdIdTimer, 20, NULL
	jmp	  wmFin
    wmTimer:

      ;  call	   demo_timer

	inc	  dword [vdMovimiento]
	invoke	  InvalidateRect, [hWnd], NULL, NULL
	jmp	  wmFin
    wmEraseBackground:
	mov	  eax, 1   ; Con esto no se borra el fondo
	jmp	  wmFin
    wmPaint:
	invoke	  BeginPaint,[hWnd], ps
	mov	  [hdc], eax

	call	  demo_flush

	; Copia el bufer DIB en el bufer BMP
	invoke	  BitBlt, [hBackDC], 0, 0, cdXSize, cdYSize, [bufDIBDC], 0, 0, SRCCOPY
	; Copia el bufer BMP en la pantalla principal
	invoke	  BitBlt, [hdc], 0, 0, cdXSize, cdYSize, [hBackDC], 0, 0, SRCCOPY
	invoke	  EndPaint,[hWnd], ps
	jmp	  wmFin
	;
    wmClose:
	; Lo destruye
	invoke	  KillTimer,	[hWnd], cdIdTimer
	invoke	  SelectObject, [hBackDC], [hOldBmp]
	invoke	  DeleteObject, [bufBMP]
	invoke	  DeleteDC,	[hBackDC]
	invoke	  SelectObject, [bufDIBDC], [hOldDIB]
	invoke	  DeleteDC,	[bufDIBDC]
	invoke	  DeleteObject, [hMainDIB]
	invoke	  DestroyWindow,[hWnd]
	jmp	  wmFin
    wmDestroy:
	invoke	  PostQuitMessage, 0
    wmFin:
	xor	eax, eax
    wmFin2:
    ret
endp





align 4
Random:
	stdcall  WRandomInit
	; Получить случайное число в интервале min...max (включая границы)
	stdcall  WIRandom,1,eax
ret

;---------------------------------------------
; Park Miller random number algorithm 
; Получить случайное число 0 ... 99999 
; stdcall WRandom 
; на выходе EAX - случайное число  
;--------------------------------------------- 
proc	WRandom 
	push	edx ecx 
	mov	eax,[random_seed] 
	xor	edx,edx 
	mov	ecx,127773 
	div	ecx 
	mov	ecx,eax 
	mov	eax,16807 
	mul	edx 
	mov	edx,ecx 
	mov	ecx,eax 
	mov	eax,2836 
	mul	edx 
	sub	ecx,eax 
	xor	edx,edx 
	mov	eax,ecx 
	mov	[random_seed],ecx 
	mov	ecx,100000 
	div	ecx 
	mov	eax,edx 
	pop	ecx edx 
	ret 
endp 
  
;--------------------------------------------- 
; Получить случайное число в нужном интервале 
; Требуется процедура WRandom 
; stdcall WIRandom,min,max 
; на выходе EAX - случайное число    
;--------------------------------------------- 
proc	WIRandom rmin:dword,rmax:dword 
	push	edx ecx 
	mov	ecx,[rmax] 
	sub	ecx,[rmin] 
	inc	ecx 
	stdcall WRandom 
	xor	edx,edx 
	div	ecx 
	mov	eax,edx 
	add	eax,[rmin] 
	pop	ecx edx 
	ret 
endp 
  
;--------------------------------------------- 
; Инициализация генератора случайных чисел 
; stdcall WRandomInit  
;--------------------------------------------- 
proc	WRandomInit 
	push	eax edx 
	rdtsc 
	xor	eax,edx 
	mov	[random_seed],eax 
	pop	edx eax 
	ret 
endp


;align 4
;IO_SaveToFile:  ; esi=data, ecx=size, eax=filename
;	 push	 esi ecx
;	 invoke  CreateFile, eax, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
;	 pop	 ecx esi
;	 cmp	 eax, -1
;	 je	.exit
;	 mov	 [hFile], eax
;    @@: invoke  WriteFile,    [hFile], ESI, ECX, ESP, 0
;	 invoke  CloseHandle,  [hFile]
;  .exit:
;ret







;------------------------------------------------------------------------------
putpixel:
; eax = x
; ebx = y


     imul    ebx, texturesize_X*3;[BytesPerScanLine]	 ; ebx = y * y multiplier
     lea     edi, [eax+eax*2]  ; edi = x*3
;     mov     eax, [esp+32-8+4]
     mov     word[screen_buffer+ebx+edi], cx
     shr     eax, 16
     mov     byte[screen_buffer+ebx+edi+2], cl
ret


include 'fontlib_charset.inc'
include 'fontlib.inc'


section '.data' data readable writeable
  include 'fontlib_data.inc'








  random_seed dd 0
  NombreClase	   TCHAR    'FONT', 0
  msgTitulo	   TCHAR    'FONT', 0
  MsgError	   TCHAR    'errrorka', 0
  wc		   WNDCLASS 0,WndProc,0,0,NULL,NULL,NULL,NULL,NULL,NombreClase
  ;
  LCDBITMAPINFO    BITMAPINFOHEADER sizeof.BITMAPINFOHEADER,\  ; biSize 	
				    cdXSize,\		       ; biWidth
				    -1*cdYSize,\	       ; biHeight
				    1,\ 		       ; biPlanes	
				    32,\		       ; biBitCount
				    0,\ 		       ; biCompression	
				    cdXSize*cdYSize*3,\    ;0	 ; biSizeImage
				    0, \		       ; biXPelsPerMeter
				    0, \		       ; biYPelsPerMeter
				    0, \		       ; biClrUsed	
				    0			       ; biClrImportant 

  ;
  vdMovimiento	       dd    0



testt db 'SysSetup',0
testt_Sz = $ - testt



align 4
screen_buffer:
       rb texturesize_X*texturesize_Y*3 + 32; 32=Reserved









section '.bss' data readable writeable
  wMsg		   MSG
  ps		   PAINTSTRUCT
  hdc		   rd	    1
  CommandLine	   rd	    1
  bufDIBDC	   rd	    1  ; HDC
  bufBMP	   rd	    1  ; HBITMAP
  hOldBmp	   rd	    1
  ;
  hDC		   rd	    1
  pMainDIB	   rd	    1
  hMainDIB	   rd	    1
  hLabelDC	   rd	    1
  hOldDIB	   rd	    1
  hBackDC	   rd	    1
  ;
  vdSeed	   rd	    1
  vmBase	   rb	    cdXSize * cdYSize
  vdTmp 	   rd	    4
  vdTmp2	   rd	    4
  ;
  miceX 	   rd	    1
  miceY 	   rd	    1
  miceOldX	   rd	    1
  miceOldY	   rd	    1
  ;
  window	   rd	    1

section '.idata' import data readable writable
  library kernel32,'KERNEL32.DLL',\    ; Importamos las bibliotecas
	  user32,  'USER32.DLL',\      ; Importamos las bibliotecas
	  gdi32,   'GDI32.DLL'	       ; Importamos las bibliotecas

  include '../fasm/include/api/kernel32.inc'	       ; KERNEL32 API calls
  include '../fasm/include/api/user32.inc'	       ; USER32 API calls
  include '../fasm/include/api/gdi32.inc'	       ; USER32 API calls

