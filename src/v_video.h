// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief Gamma correction LUT

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"

//
// VIDEO
//

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern UINT8 *screens[5];

extern const UINT8 gammatable[5][256];
extern consvar_t cv_ticrate, cv_usegamma, cv_allcaps;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

// Set the current RGB palette lookup to use for palettized graphics
void V_SetPalette(INT32 palettenum);

void V_SetPaletteLump(const char *pal);

const char *R_GetPalname(UINT16 num);
const char *GetPalette(void);

extern RGBA_t *pLocalPalette;

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color) (pLocalPalette[color&0xFF])

// like V_DrawPatch, + using a colormap.
void V_DrawMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap);

// flags hacked in scrn (not supported by all functions (see src))
#define V_NOSCALESTART       0x00010000  // don't scale x, y, start coords
#define V_WRAPY              0x00020000  // Don't clamp texture on Y (for HW mode)
// V_SCALEPATCH isn't even used anywhere in the entire project! thus, I'm hijacking it for something else ~MattW_CFI
// #define V_SCALEPATCH         0x00040000  // scale patch
#define V_ALLOWLOWERCASE     0x00040000  // allow fonts that have lowercase letters to use them
#define V_NOSCALEPATCH       0x00080000  // don't scale patch
#define V_YELLOWMAP          0x00100000  // draw yellow (for v_drawstring)
#define V_SNAPTOTOP          0x00200000  // for centering
#define V_TRANSLUCENT        0x00400000  // TRANS50 applied
#define V_SNAPTOBOTTOM       0x00800000  // for centering
#define V_WRAPX              0x01000000  // Don't clamp texture on X (for HW mode)
#define V_WORDWRAP           0x02000000  // Word wrap
#define V_8020TRANS          0x04000000
#define V_GREENMAP           0x08000000
#define V_TOPLEFT            0x10000000
#define V_RETURN8            0x20000000
#define V_SNAPTOLEFT         0x40000000 // for centering
#define V_SNAPTORIGHT        0x80000000 // for centering

// default params: scale patch and scale start
void V_DrawScaledPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch);

// default params: scale patch and scale start
void V_DrawSmallScaledPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch);
void V_DrawSmallMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap);
void V_DrawSmallTranslucentMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap);
void V_DrawSmallTranslucentPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch);

// like V_DrawScaledPatch, plus translucency
void V_DrawTranslucentPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch);

void V_DrawPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch);

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(INT32 x, INT32 y, INT32 scrn, INT32 width, INT32 height, const UINT8 *src);

// draw a pic_t, SCALED
void V_DrawScaledPic (INT32 px1, INT32 py1, INT32 scrn, INT32 lumpnum);

// fill a box with a single color
void V_DrawFill(INT32 x, INT32 y, INT32 w, INT32 h, INT32 c);
// fill a box with a flat as a pattern
void V_DrawFlatFill(INT32 x, INT32 y, INT32 w, INT32 h, lumpnum_t flatnum);

// fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen(void);

void V_DrawFadeConsBack(INT32 px1, INT32 py1, INT32 px2, INT32 py2, INT32 pcolor);

// draw a single character
void V_DrawCharacter(INT32 x, INT32 y, INT32 c, boolean lowercaseallowed);

void V_DrawLevelTitle(INT32 x, INT32 y, INT32 option, const char *string);

// draw a string using the hu_font
void V_DrawString(INT32 x, INT32 y, INT32 option, const char *string);
void V_DrawCenteredString(INT32 x, INT32 y, INT32 option, const char *string);
void V_DrawRightAlignedString(INT32 x, INT32 y, INT32 option, const char *string);

// Draw a tiny number, yay.
void V_DrawTinyNum(INT32 x, INT32 y, INT32 c, INT32 num);

// Find string width from lt_font chars
INT32 V_LevelNameWidth(const char *string);
INT32 V_LevelNameHeight(const char *string);

void V_DrawCreditString(INT32 x, INT32 y, INT32 option, const char *string);
INT32 V_CreditStringWidth(const char *string);

// Find string width from hu_font chars
INT32 V_StringWidth(const char *string);

void V_DoPostProcessor(postimg_t type);

void V_DrawPatchFill(patch_t *pat);

void VID_BlitLinearScreen(const UINT8 *srcptr, UINT8 *destptr, INT32 width, INT32 height, size_t srcrowbytes,
	size_t destrowbytes);

#endif
