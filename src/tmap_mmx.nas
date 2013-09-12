; File generated with A2I 0.4 (by Pascal Lacroix)
; a2i -o test.nas src\tmap_asm.s
; Warning: Use at your own risks, no warranties
;;-----------------------------------------------------------------------------
;;
;; Copyright (C) 1998-2000 by DOSDOOM.
;;
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License
;; as published by the Free Software Foundation; either version 2
;; of the License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; DESCRIPTION:
;;      Assembler optimised rendering code for software mode.
;;      Draw wall columns.
;;
;;-----------------------------------------------------------------------------


[BITS 32]

%ifdef LINUX
%macro cextern 1
[extern %1]
%endmacro

%macro cglobal 1
[global %1]
%endmacro

%define CODE_SEG .data
%define CODE_WSEG .data
%else
%macro cextern 1
%define %1 _%1
[extern %1]
%endmacro

%macro cglobal 1
%define %1 _%1
[global %1]
%endmacro

%define CODE_SEG .data
%define CODE_WSEG .text write
%endif


;; externs
;; columns
cextern dc_colormap
cextern dc_x
cextern dc_yl
cextern dc_yh
cextern dc_iscale
cextern dc_texturemid
cextern dc_source
cextern ylookup
cextern columnofs
cextern centery ;;?
cextern dc_transmap ;;DrawTranslucentColumn
cextern colormaps ;;DrawShadeColumn

[SECTION .data]

;;.align        4
loopcount       dd      0
pixelcount      dd      0
tystep          dd      0 ;;Special?
;;.align        8
mmxcount        dd      0
mmxcomm         dd      0

[SECTION CODE_SEG]

;----------------------------------------------------------------------------
; void  MMX_PatchRowBytes (int rowbytes);
;----------------------------------------------------------------------------
cglobal MMX_PatchRowBytes
;       align   16
MMX_PatchRowBytes:
        mov     eax,[esp+4]
        mov     [p1+2],eax
        mov     [p2+2],eax
        mov     [p3+2],eax
        mov     [p4+2],eax
        mov     [p5+2],eax
        mov     [p6+2],eax
        mov     [p7+2],eax
        mov     [p8+2],eax
        mov     [p9+2],eax
        mov     [pa+2],eax
        mov     [pb+2],eax
        mov     [pc+2],eax
        mov     [pd+2],eax
        mov     [pe+2],eax
        mov     [pf+2],eax
        mov     [pg+2],eax
        mov     [ph+2],eax
        mov     [pi+2],eax
        ret

[SECTION CODE_WSEG]

cglobal R_DrawColumn_8_NOMMX
;       align   16
R_DrawColumn_8_NOMMX:
	push ebp
	push esi
	push edi
	push ebx
	mov edx, [dc_yl]
	mov eax, [dc_yh]
	sub eax, edx
	lea ebx, [eax+1]
	test ebx, ebx
	jle near rdc8ndone
	mov eax, [dc_x]
	mov edi, [ylookup]
	mov esi, [ylookup + edx*+4]
	mov edi, [columnofs]
	add esi, [columnofs + eax*+4]
	mov edi, [dc_iscale]
	mov eax, edx
	imul eax, edi
	mov ecx, [dc_texturemid]
	add ecx, eax
	mov ebp, [dc_source]
	xor edx, edx
p1:sub esi, 0x12345678

GLOBAL rdc8nwidth1
; (label at line 50)
rdc8nwidth1:
	align 4, nop
; (label at line 52)
rdc8nloop:
	mov eax, ecx
	shr eax, +16
	add ecx, edi
	and eax, +127
p2:add esi, 0x12345678

GLOBAL rdc8nwidth2
; (label at line 59)
rdc8nwidth2:
	mov dl, [eax + ebp]
	mov eax, [dc_colormap]
	mov al, [dc_colormap + edx]
	mov [esi], al
	dec ebx
	jne near rdc8nloop
; (label at line 66)
rdc8ndone:
	pop ebx
	pop edi
	pop esi
	pop ebp
	ret
;/
;/ Optimised specifically for P54C/P55C (aka Pentium with/without MMX)
;/ By ES 1998/08/01
;/

cglobal R_DrawColumn_8_Pentium
;       align   16
R_DrawColumn_8_Pentium:
	push ebp
	push ebx
	push esi
	push edi
	mov eax, [dc_yl]
;/ Top pixel
	mov ebx, [dc_yh]
;/ Bottom pixel
	mov edi, [ylookup]
	mov ecx, [ylookup + ebx*+4]
;/ ebx=number of pixels-1
	sub ebx, eax
;/ no pixel to draw, done
	jl near rdc8pdone
	jnz near rdc8pmany
	mov edx, [dc_x]
;/ Special case: only one pixel
	mov edi, [columnofs]
	add ecx, [columnofs + edx*+4]
;/ dest pixel at (%ecx)
	mov esi, [dc_iscale]
	imul eax, esi
	mov edi, [dc_texturemid]
;/ texture index in edi
	add edi, eax
	mov edx, [dc_colormap]
	shr edi, +16
	mov ebp, [dc_source]
	and edi, +127
	mov dl, [edi + ebp]
;/ read texture pixel
	mov al, [edx]
;/ lookup for light
	mov [ecx+0], al
;/ write it
;/ done!
	jmp rdc8pdone
	align 4, nop
;/ draw >1 pixel
; (label at line 107)
rdc8pmany:
	mov edx, [dc_x]
	mov edi, [columnofs]
	mov edx, [columnofs + edx*+4]
	lea edi, [edx + ecx]
p3:add edi, 0x12345678
;/ edi = two pixels above bottom

GLOBAL rdc8pwidth5
;/ DeadBeef = -2*SCREENWIDTH
; (label at line 113)
rdc8pwidth5:
	mov edx, [dc_iscale]
;/ edx = fracstep
	imul eax, edx
;/ fixme: Should get 7.25 fix as input
	shl edx, +9
	mov ecx, [dc_texturemid]
;/ ecx = frac
	add ecx, eax
	mov eax, [dc_colormap]
;/ eax = lighting/special effects LUT
	shl ecx, +9
	mov esi, [dc_source]
;/ esi = source ptr
p4:imul ebx, 0x12345678 ;* parse error at line 123 *
;/ ebx = negative offset to pixel

GLOBAL rdc8pwidth6
;/ DeadBeef = -SCREENWIDTH
; (label at line 125)
rdc8pwidth6:
;/ Begin the calculation of the two first pixels
	lea ebp, [ecx + edx]
	shr ecx, +25
	mov al, [esi + ecx]
	lea ecx, [edx + ebp]
	shr ebp, +25
	mov dl, [eax]
;/ The main loop
; (label at line 136)
rdc8ploop:
	mov al, [esi + ebp]
;/ load 1
	lea ebp, [ecx + edx]
;/ calc frac 3
;/ shift frac 2
	shr ecx, +25
	add edi, ebx
p5:add edi, 0x12345678
	mov [edi], dl
p6:sub edi, 0x12345678
	sub edi, ebx
;/ store 0

GLOBAL rdc8pwidth1
;/ DeadBeef = 2*SCREENWIDTH
; (label at line 143)
rdc8pwidth1:
	mov al, [eax]
;/ lookup 1
	add edi, ebx
p7:add edi, 0x12345678
	mov [edi], al
p8:sub edi, 0x12345678
	sub edi, ebx
;/ store 1

GLOBAL rdc8pwidth2
;/ DeadBeef = 3*SCREENWIDTH
; (label at line 149)
rdc8pwidth2:
	mov al, [esi + ecx]
;/ load 2
	lea ecx, [ebp + edx]
;/ calc frac 4
;/ shift frac 3
	shr ebp, +25
	mov dl, [eax]
;/ lookup 2
;/ counter
p9:add ebx, 0x12345678

GLOBAL rdc8pwidth3
;/ DeadBeef = 2*SCREENWIDTH
; (label at line 159)
rdc8pwidth3:
;/ loop
	jl near rdc8ploop
;/ End of loop. Write extra pixel or just exit.
	jnz near rdc8pdone
	add edi, ebx
pa:add edi, 0x12345678
	mov [edi], dl
pb:sub edi, 0x12345678
	sub edi, ebx
;/ Write odd pixel

GLOBAL rdc8pwidth4
;/ DeadBeef = 2*SCREENWIDTH
; (label at line 166)
rdc8pwidth4:
; (label at line 168)
rdc8pdone:
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret
;/
;/ MMX asm version, optimised for K6
;/ By ES 1998/07/05
;/

cglobal R_DrawColumn_8_K6_MMX
;       align   16
R_DrawColumn_8_K6_MMX:
	push ebp
	push ebx
	push esi
	push edi
;/ Push 8 or 12, so that (%esp) gets aligned by 8
	mov eax, esp
	and eax, +7
	add eax, +8
;/ Temp storage in mmxcomm: (%esp) is used instead
	mov [mmxcomm], eax
	sub esp, eax
	mov edx, [dc_yl]
;/ Top pixel
	mov ebx, [dc_yh]
;/ Bottom pixel
	mov edi, [ylookup]
	mov ecx, [ylookup + ebx*+4]
;/ ebx=number of pixels-1
	sub ebx, edx
;/ no pixel to draw, done
	jnz near rdc8mdone ;* parse error at line 199 *

GLOBAL rdc8moffs1
; (label at line 201)
rdc8moffs1:
	jnz near rdc8mmany
	mov eax, [dc_x]
;/ Special case: only one pixel
	mov edi, [columnofs]
	add ecx, [columnofs + eax*+4]
;/ dest pixel at (%ecx)
	mov esi, [dc_iscale]
	imul edx, esi
	mov edi, [dc_texturemid]
;/ texture index in edi
	add edi, edx
	mov edx, [dc_colormap]
	shr edi, +16
	mov ebp, [dc_source]
	and edi, +127
	mov dl, [edi + ebp]
;/ read texture pixel
	mov al, [edx]
;/ lookup for light
	mov [ecx+0], al
;/ write it
;/ done!
	jmp rdc8mdone ;* parse error at line 217 *

GLOBAL rdc8moffs2
; (label at line 219)
rdc8moffs2:
	align 4, nop
;/ draw >1 pixel
; (label at line 221)
rdc8mmany:
	mov eax, [dc_x]
	mov edi, [columnofs]
	mov eax, [columnofs + eax*+4]
	lea esi, [eax + ecx]
pc:add esi, 0x12345678
;/ esi = two pixels above bottom

GLOBAL rdc8mwidth3
;/ DeadBeef = -2*SCREENWIDTH
; (label at line 227)
rdc8mwidth3:
	mov ecx, [dc_iscale]
;/ ecx = fracstep
	imul edx, ecx
;/ fixme: Should get 7.25 fix as input
	shl ecx, +9
	mov eax, [dc_texturemid]
;/ eax = frac
	add eax, edx
	mov edx, [dc_colormap]
;/ edx = lighting/special effects LUT
	shl eax, +9
	lea edi, [ecx + ecx]
	mov ebp, [dc_source]
;/ ebp = source ptr
	mov [esp+0], edi
;/ Start moving frac and fracstep to MMX regs
pd:imul ebx, 0x12345678 ;* parse error at line 239 *
;/ ebx = negative offset to pixel

GLOBAL rdc8mwidth5
;/ DeadBeef = -SCREENWIDTH
; (label at line 241)
rdc8mwidth5:
	mov [esp+4], edi
	lea edi, [eax + ecx]
	movq mm1, [esp]	;* parse error at line 245 *
;/ fracstep:fracstep in mm1
	mov [esp], eax
	shr eax, +25
	mov [esp+4], edi
	mov eax, [ebp + eax]
	movq mm0, [esp] ;* parse error at line 250 *
;/ frac:frac in mm0
	paddd mm0, mm1 ;* parse error at line 252 *
	shr edi, +25
	movq mm2, mm0 ;* parse error at line 254 *
;/ texture index in mm2

GLOBAL rdc8mloop
;/ The main loop
; (label at line 260)
rdc8mloop:
	movq mm2, mm0 ;* parse error at line 261 *
;/ move 4-5 to temp reg
;/ read 1
	mov edi, [ebp + edi]
	psrld mm2, 25 ;* parse error at line 264 *
;/ shift 4-5
	mov cl, [edx + eax]
;/ lookup 0
	mov eax, [esp]
;/ load 2
;/ counter
pe:add ebx, 0x12345678

GLOBAL rdc8mwidth2
;/ DeadBeef = 2*SCREENWIDTH
; (label at line 270)
rdc8mwidth2:
	mov [esi + ebx], cl
;/ write 0
	mov ch, [edx + edi]
;/ lookup 1
	add esi, ebx
pf:add esi, 0x12345678
	mov [esi], ch
pg:sub esi, 0x12345678
	sub esi, ebx
;/ write 1

GLOBAL rdc8mwidth1
;/ DeadBeef = SCREENWIDTH
; (label at line 277)
rdc8mwidth1:
	mov edi, [esp+4]
;/ load 3
	paddd mm0, mm1;* parse error at line 280 *
;/ frac 6-7
;/ lookup 2
	mov eax, [ebp + eax]
	movq [esp], mm2 ;* parse error at line 283 *
;/ store texture index 4-5
	jl near rdc8mloop
	jnz near rdc8mno_odd
	mov cl, [edx + eax]
;/ write the last odd pixel
ph:add esi, 0x12345678
	mov [esi], cl
pi:sub esi, 0x12345678

GLOBAL rdc8mwidth4
;/ DeadBeef = 2*SCREENWIDTH
; (label at line 290)
rdc8mwidth4:
; (label at line 291)
rdc8mno_odd:

GLOBAL rdc8mdone
; (label at line 294)
rdc8mdone:
	emms ;* parse error at line 295 *
	add esp, [mmxcomm]
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret
;/ Need some extra space to align run-time

GLOBAL R_DrawColumn_8_K6_MMX_end
; (label at line 306)
R_DrawColumn_8_K6_MMX_end:
	nop
	nop
	nop
	nop
