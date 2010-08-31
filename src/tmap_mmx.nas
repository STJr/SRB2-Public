;;-----------------------------------------------------------------------------
;;
;; Copyright (C) 1998-2000 by DOSDOOM.
;; Copyright (C) 2010 by Sonic Team Jr.
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
;;      Assembler optimised rendering code for software mode, using SIMD
;;      instructions (but not restricted to MMX as the filename might
;;      suggest).
;;      Draw wall columns.
;;
;;-----------------------------------------------------------------------------


[BITS 32]

%define FRACBITS 16

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


; The viddef_s structure. We only need the width field.
struc viddef_s
		resb 12
.width: resb 4
		resb 44
endstruc


;; externs
;; columns
cextern dc_colormap
cextern dc_x
cextern dc_yl
cextern dc_yh
cextern dc_iscale
cextern dc_texturemid
cextern dc_texheight
cextern dc_source
cextern ylookup
cextern columnofs
cextern dc_hires
cextern centery
cextern centeryfrac
cextern dc_transmap
cextern vid

cextern R_DrawColumn_8_ASM

[SECTION .data]


[SECTION CODE_SEG]

;;----------------------------------------------------------------------
;;
;; R_DrawColumn : 8bpp column drawer
;;
;; MMX (and a little SSE) column drawer.
;;
;;----------------------------------------------------------------------
;; eax = accumulator
;; ebx = colormap
;; ecx = count
;; edx = accumulator
;; esi = source
;; edi = dest
;; ebp = vid.width
;; mm0 = accumulator
;; mm1 = heightmask, twice
;; mm2 = 2 * fracstep, twice
;; mm3 = pair of consecutive fracs
;;----------------------------------------------------------------------


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cglobal R_DrawColumn_8_SSE
R_DrawColumn_8_SSE:
		push		ebp						;; preserve caller's stack frame pointer
		push		esi						;; preserve register variables
		push		edi
		push		ebx
		
;;
;; Our algorithm requires that the texture height be a power of two.
;; If not, fall back to the non-MMX drawer.
;;
.texheightcheck:
		mov			edx, [dc_texheight]
		sub			edx, 1					;; edx = heightmask
		test		edx, [dc_texheight]
		jnz			near .usenonMMX
		
		mov			ebp, edx				;; Keep a copy of heightmask in a
											;; GPR for the time being.

;;
;; Fill mm1 with heightmask
;;
		movd		mm1, edx				;; low dword = heightmask
		punpckldq	mm1, mm1				;; copy low dword to high dword

;;
;; dest = ylookup[dc_yl] + columnofs[dc_x];
;;
		mov			eax, [dc_yl]
		mov			edi, [ylookup+eax*4]
		mov			ebx, [dc_x]
		add			edi, [columnofs+ebx*4]	;; edi = dest
		
		
;;
;; pixelcount = yh - yl + 1
;;
		mov			ecx, [dc_yh]
		add			ecx, 1
		sub			ecx, eax				;; pixel count
		jle			near .done				;; nothing to scale
;;
;; fracstep = dc_iscale;
;;
		movd		mm2, [dc_iscale]		;; fracstep in low dword
		punpckldq	mm2, mm2				;; copy to high dword
		
		mov			ebx, [dc_colormap]
		mov			esi, [dc_source]
		
;;
;; frac = (dc_texturemid + FixedMul((dc_yl << FRACBITS) - centeryfrac, fracstep));
;;
											;; eax == dc_yl already
		shl			eax, FRACBITS
		sub			eax, [centeryfrac]
		imul		dword [dc_iscale]
		shrd		eax, edx, FRACBITS
		add			eax, [dc_texturemid]
		
;;
;; if (dc_hires) frac = 0;
;;
		test		byte [dc_hires], 0x01
		mov			edx, 0
		cmovnz		eax, edx


;;
;; Do mod-2 pixel.
;;
.mod2:
		test		ecx, 1
		jz			.pairprepare
		mov			edx, eax				;; edx = frac
		add			eax, [dc_iscale]		;; eax += fracstep
		sar			edx, FRACBITS
		and			edx, ebp				;; edx &= heightmask
		movzx		edx, byte [esi + edx]
		movzx		edx, byte [ebx + edx]
		mov			[edi], dl
		
		add			edi, [vid + viddef_s.width]
		sub			ecx, 1
		jz			.done

.pairprepare:		
;;
;; Prepare for the main loop.
;;
		movd		mm3, eax				;; Low dword = frac
		movq		mm4, mm3				;; Copy to intermediate register
		paddd		mm4, mm2				;; dwords of mm4 += fracstep
		punpckldq	mm3, mm4				;; Low dword = first frac, high = second
		pslld		mm2, 1					;; fracstep *= 2
				
;;
;; ebp = vid.width
;;
		mov			ebp, [vid + viddef_s.width]
		
		align		16
.pairloop:
		movq		mm0, mm3				;; 3B 1u.
		psrad		mm0, FRACBITS			;; 4B 1u.
		pand		mm0, mm1				;; 3B 1u. frac &= heightmask
		paddd		mm3, mm2				;; 3B 1u. frac += fracstep

		movd		eax, mm0				;; 3B 1u. Get first frac
;; IFETCH boundary	
		movzx		eax, byte [esi + eax]	;; 4B 1u. Texture map
		movzx		eax, byte [ebx + eax]	;; 4B 1u. Colormap
		
		pshufw		mm0, mm0, 0x0e			;; 4B 1u. low dword = high dword
		movd		edx, mm0				;; 3B 1u. Get second frac
;; IFETCH boundary
		mov			[edi], al				;; 2B 1(2)u. First pixel
		
		movzx		edx, byte [esi + edx]	;; 4B 1u. Texture map
		movzx		edx, byte [ebx + edx]	;; 4B 1u. Colormap
		mov			[edi + 1*ebp], dl		;; 3B 1(2)u. Second pixel

		lea			edi, [edi + 2*ebp]		;; 3B 1u. edi += 2 * vid.width
;; IFETCH boundary
		sub			ecx, 2					;; 3B 1u. count -= 2
		jnz			.pairloop				;; 2B 1u. if(count != 0) goto .pairloop


.done:				
;;
;; Clear MMX state, or else FPU operations will go badly awry.
;;
		emms
		
		pop			ebx
		pop			edi
		pop			esi
		pop			ebp
		ret

.usenonMMX:
		call		R_DrawColumn_8_ASM
		jmp			.done
