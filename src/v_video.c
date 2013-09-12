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
/// \brief Gamma correction LUT stuff
///
///	Functions to draw patches (by post) directly to screen.
///	Functions to blit a block to the screen.

#include "doomdef.h"
#include "r_local.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "r_draw.h"
#include "console.h"

#include "i_video.h" // rendermode
#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "doomstat.h"

#ifdef HWRENDER
#include "hardware/hw_glob.h"
#endif

// Each screen is [vid.width*vid.height];
UINT8 *screens[5];

static CV_PossibleValue_t ticrate_cons_t[] = {
	{0, "Off"}, {1, "Counter"}, {2, "Graph"}, {3, "Both"},
	{0, NULL}};
static CV_PossibleValue_t gamma_cons_t[] = {{0, "MIN"}, {4, "MAX"}, {0, NULL}};
static void CV_usegamma_OnChange(void);

consvar_t cv_ticrate = {"vid_ticrate", "Off", 0, ticrate_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_usegamma = {"gamma", "0", CV_SAVE|CV_CALL, gamma_cons_t, CV_usegamma_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_allcaps = {"allcaps", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

#ifdef HWRENDER
static void CV_Gammaxxx_ONChange(void);
// Saved hardware mode variables
// - You can change them in software,
// but they won't do anything.
static CV_PossibleValue_t grrenderquality_cons_t[] = {{1, "Speed"}, {2, "Quality"}, {3, "Full Quality"}, {0, NULL}};
static CV_PossibleValue_t grgamma_cons_t[] = {{1, "MIN"}, {255, "MAX"}, {0, NULL}};

consvar_t cv_grrenderquality = {"gr_renderdetail", "Quality", CV_SAVE, grrenderquality_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfovchange = {"gr_fovchange", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfog = {"gr_fog", "On", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfogcolor = {"gr_fogcolor", "AAAAAA", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grgammared = {"gr_gammared", "127", CV_SAVE|CV_CALL, grgamma_cons_t,
                           CV_Gammaxxx_ONChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grgammagreen = {"gr_gammagreen", "127", CV_SAVE|CV_CALL, grgamma_cons_t,
                             CV_Gammaxxx_ONChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grgammablue = {"gr_gammablue", "127", CV_SAVE|CV_CALL, grgamma_cons_t,
                            CV_Gammaxxx_ONChange, 0, NULL, NULL, 0, 0, NULL};
#endif

const UINT8 gammatable[5][256] =
{
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
	81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
	97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

	{2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
	32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
	56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
	78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
	99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
	115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
	130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
	146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
	161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
	175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
	190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
	205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
	219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
	233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
	247,248,249,250,251,252,252,253,254,255},

	{4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
	43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
	70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
	94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
	144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
	160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
	174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
	188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
	202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
	216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
	229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
	242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
	255},

	{8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
	57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
	86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
	108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
	125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
	141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
	155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
	169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
	183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
	195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
	207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
	219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
	231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
	242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
	253,253,254,254,255},

	{16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
	78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
	107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
	125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
	142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
	156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
	169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
	182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
	193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
	204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
	214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
	224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
	234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
	243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
	251,252,252,253,254,254,255,255}
};

// local copy of the palette for V_GetColor()
RGBA_t *pLocalPalette = NULL;

// keep a copy of the palette so that we can get the RGB value for a color index at any time.
static void LoadPalette(const char *lumpname)
{
	const UINT8 *usegamma = gammatable[cv_usegamma.value];
	lumpnum_t lumpnum = W_GetNumForName(lumpname);
	size_t i, palsize = W_LumpLength(lumpnum)/3;
	UINT8 *pal;

	Z_Free(pLocalPalette);

	pLocalPalette = Z_Malloc(sizeof (*pLocalPalette)*palsize, PU_STATIC, NULL);

	pal = W_CacheLumpNum(lumpnum, PU_CACHE);
	for (i = 0; i < palsize; i++)
	{
		pLocalPalette[i].s.red = usegamma[*pal++];
		pLocalPalette[i].s.green = usegamma[*pal++];
		pLocalPalette[i].s.blue = usegamma[*pal++];
		pLocalPalette[i].s.alpha = 0xFF;
	}
}

const char *R_GetPalname(UINT16 num)
{
	static char palname[9];
	char newpal[9] = "PLAYPAL";

	if (num <= 9999)
		snprintf(newpal, 8, "PAL%04u", num-1);

	strncpy(palname, newpal, 8);
	return palname;
}

const char *GetPalette(void)
{
	if (gamestate == GS_LEVEL)
		return R_GetPalname(mapheaderinfo[gamemap-1].palette);
	return "PLAYPAL";
}

static void LoadMapPalette(void)
{
	LoadPalette(GetPalette());
}

// -------------+
// V_SetPalette : Set the current palette to use for palettized graphics
//              :
// -------------+
void V_SetPalette(INT32 palettenum)
{
	if (!pLocalPalette)
		LoadMapPalette();

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
		HWR_SetPalette(&pLocalPalette[palettenum*256]);
#if defined (__unix__) || defined (UNIXCOMMON) || defined (SDL)
	else
#endif
#endif
	if (rendermode != render_none)
		I_SetPalette(&pLocalPalette[palettenum*256]);
}

void V_SetPaletteLump(const char *pal)
{
	LoadPalette(pal);
#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
		HWR_SetPalette(pLocalPalette);
#if defined (__unix__) || defined (UNIXCOMMON) || defined (SDL)
	else
#endif
#endif
	if (rendermode != render_none)
		I_SetPalette(pLocalPalette);
}

static void CV_usegamma_OnChange(void)
{
	// reload palette
	LoadMapPalette();
	V_SetPalette(0);
}

// change the palette directly to see the change
#ifdef HWRENDER
static void CV_Gammaxxx_ONChange(void)
{
	if (rendermode != render_soft && rendermode != render_none)
		V_SetPalette(0);
}
#endif


#if defined (__GNUC__) && defined (__i386__) && !defined (NOASM)
void VID_BlitLinearScreen_ASM(const UINT8 *srcptr, UINT8 *destptr, INT32 width, INT32 height, size_t srcrowbytes,
	size_t destrowbytes);
#define HAVE_VIDCOPY
#endif


// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// --------------------------------------------------------------------------
void VID_BlitLinearScreen(const UINT8 *srcptr, UINT8 *destptr, INT32 width, INT32 height, size_t srcrowbytes,
	size_t destrowbytes)
{
#ifdef HAVE_VIDCOPY
    VID_BlitLinearScreen_ASM(srcptr,destptr,width,height,srcrowbytes,destrowbytes);
#else
	if (srcrowbytes == destrowbytes)
		M_Memcpy(destptr, srcptr, srcrowbytes * height);
	else
	{
		while (height--)
		{
			M_Memcpy(destptr, srcptr, width);

			destptr += destrowbytes;
			srcptr += srcrowbytes;
		}
	}
#endif
}

//
// V_DrawTranslucentMappedPatch: like V_DrawMappedPatch, but with translucency.
//
static void V_DrawTranslucentMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap)
{
	size_t count;
	INT32 col, w, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest;
	const UINT8 *source, *translevel, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawMappedPatch((GLPatch_t *)patch, x, y, scrn, colormap);
		return;
	}
#endif

	if (scrn & V_8020TRANS)
		translevel = ((tr_trans80)<<FF_TRANSSHIFT) - 0x10000 + transtables;
	else
		translevel = ((tr_trans50)<<FF_TRANSSHIFT) - 0x10000 + transtables;

	if (scrn & V_NOSCALEPATCH)
		dupx = dupy = 1;
	else
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (scrn & V_NOSCALESTART)
	{
		desttop = screens[scrn&0xffff] + (y*vid.width) + x;
		deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;
	}
	else
	{
		desttop = screens[scrn&0xffff] + (y*vid.dupy*vid.width) + (x*vid.dupx);
		deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;

		// Center it if necessary
		if (!(scrn & V_NOSCALEPATCH))
		{
			if (vid.fdupx != dupx)
			{
				// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
				// so center this imaginary screen
				if (scrn & V_SNAPTORIGHT)
					desttop += (vid.width - (BASEVIDWIDTH * dupx));
				else if (!(scrn & V_SNAPTOLEFT))
					desttop += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
			}
			if (vid.fdupy != dupy)
			{
				// same thing here
				if (scrn & V_SNAPTOBOTTOM)
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width;
				else if (!(scrn & V_SNAPTOTOP))
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
			}
			// if it's meant to cover the whole screen, black out the rest
			if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT)
				V_DrawFill(0, 0, vid.width, vid.height, 31);
		}
	}
	scrn &= 0xffff;

	col = 0;
	colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	w = SHORT(patch->width)<<FRACBITS;

	for (; col < w; col += colfrac, desttop++)
	{
		column = (const column_t *)((const UINT8 *)patch + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;

			ofs = 0;
			while (count--)
			{
				if (dest < deststop)
					*dest = *(translevel + (((*(colormap + source[ofs>>FRACBITS]))<<8)&0xff00) + (*dest&0xff));
				else
					count = 0;
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

//
// V_DrawMappedPatch: like V_DrawScaledPatch, but with a colormap.
//
void V_DrawMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap)
{
	size_t count;
	INT32 col, w, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest;
	const UINT8 *source, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawMappedPatch((GLPatch_t *)patch, x, y, scrn, colormap);
		return;
	}
#endif

	if (scrn & V_NOSCALEPATCH)
		dupx = dupy = 1;
	else
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (scrn & V_NOSCALESTART)
	{
		desttop = screens[scrn&0xffff] + (y*vid.width) + x;
		deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;
	}
	else
	{
		desttop = screens[scrn&0xffff] + (y*vid.dupy*vid.width) + (x*vid.dupx);
		deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;

		// Center it if necessary
		if (!(scrn & V_NOSCALEPATCH))
		{
			if (vid.fdupx != dupx)
			{
				// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
				// so center this imaginary screen
				if (scrn & V_SNAPTORIGHT)
					desttop += (vid.width - (BASEVIDWIDTH * dupx));
				else if (!(scrn & V_SNAPTOLEFT))
					desttop += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
			}
			if (vid.fdupy != dupy)
			{
				// same thing here
				if (scrn & V_SNAPTOBOTTOM)
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width;
				else if (!(scrn & V_SNAPTOTOP))
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
			}
			// if it's meant to cover the whole screen, black out the rest
			if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT)
				V_DrawFill(0, 0, vid.width, vid.height, 31);
		}
	}
	scrn &= 0xffff;

	col = 0;
	colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	w = SHORT(patch->width)<<FRACBITS;

	for (; col < w; col += colfrac, desttop++)
	{
		column = (const column_t *)((const UINT8 *)patch + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;

			ofs = 0;
			while (count--)
			{
				if (dest < deststop)
					*dest = *(colormap + source[ofs>>FRACBITS]);
				else
					count = 0;
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

//
// V_DrawScaledPatch
//
// Like V_DrawPatch, but scaled 2, 3, 4 times the original size and position.
// This is used for menu and title screens, with high resolutions.
//
void V_DrawScaledPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawPatch((GLPatch_t *)patch, x, y, scrn);
		return;
	}
#endif

	if ((scrn & V_NOSCALEPATCH))
		dupx = dupy = 1;
	else
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	desttop = screens[scrn&0xFF];
	deststop = screens[scrn&0xFF] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (scrn & V_NOSCALESTART)
		desttop += (y*vid.width) + x;
	else
	{
		desttop += (y*dupy*vid.width) + (x*dupx);

		// Center it if necessary
		if (!(scrn & V_NOSCALEPATCH))
		{
			if (vid.fdupx != dupx)
			{
				// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
				// so center this imaginary screen
				if (scrn & V_SNAPTORIGHT)
					desttop += (vid.width - (BASEVIDWIDTH * dupx));
				else if (!(scrn & V_SNAPTOLEFT))
					desttop += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
			}
			if (vid.fdupy != dupy)
			{
				// same thing here
				if (scrn & V_SNAPTOBOTTOM)
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width;
				else if (!(scrn & V_SNAPTOTOP))
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
			}
			// if it's meant to cover the whole screen, black out the rest
			if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT)
				V_DrawFill(0, 0, vid.width, vid.height, 31);
		}
	}
	destend = desttop + SHORT(patch->width) * dupx;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)(patch) + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)(column) + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;
					dest += vid.width;
					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto donedrawing;
				} while (count--);
			}
			else
			{
				while (count--)
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;
					dest += vid.width;
					ofs += rowfrac;
				}
			}
donedrawing:
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

/** Draws a patch to the screen, being careful not to go off the right
  * side or bottom of the screen. This is slower than a normal draw, so
  * it gets a separate function.
  *
  * With hardware rendering, the patch is clipped anyway, so this is
  * just the same as V_DrawScaledPatch().
  *
  * \param x     X coordinate for left side, based on 320x200 screen.
  * \param y     Y coordinate for top, based on 320x200 screen.
  * \param scrn  Any of several flags to change the drawing behavior.
  * \param patch Patch to draw.
  * \sa V_DrawScaledPatch
  * \author Graue <graue@oceanbase.org>
  */
static void V_DrawClippedScaledPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		// V_NOSCALESTART might be impled for software, but not for hardware!
		HWR_DrawClippedPatch((GLPatch_t *)patch, x, y, V_NOSCALESTART);
		return;
	}
#endif

	if ((scrn & V_NOSCALEPATCH))
		dupx = dupy = 1;
	else
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (x < 0 || y < 0 || x >= vid.width || y >= vid.height)
		return;

	colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	if (!screens[scrn&0xff])
		return;

	desttop = screens[scrn&0xff] + (y*vid.width) + x;
	deststop = screens[scrn&0xff] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	// make sure it doesn't go off the right
	if (x + SHORT(patch->width)*dupx <= vid.width)
		destend = desttop + SHORT(patch->width) * dupx;
	else
		destend = desttop + vid.width - x;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)patch + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;
			if ((dest-screens[scrn&0xff])/vid.width + count > (unsigned)vid.height - 1)
				count = vid.height - 1 - (dest-screens[scrn&0xff])/vid.width;
			if (count <= 0)
				break;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				// length is not a power of two
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;
					dest += vid.width;
					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto doneclipping;
				} while (count--);
			}
			else
			{
				// length is a power of two
				while (count--)
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;
					dest += vid.width;
					ofs += rowfrac;
				}
			}
doneclipping:
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

// Draws a patch 2x as small.
void V_DrawSmallScaledPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;
	boolean skippixels = false;
	INT32 skiprowcnt;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		if (!(scrn & V_NOSCALESTART)) // Graue 07-08-2004: I have no idea why this works
		{
			x = (INT32)(vid.fdupx*x);
			y = (INT32)(vid.fdupy*y);
			scrn |= V_NOSCALESTART;
		}
		HWR_DrawSmallPatch((GLPatch_t *)patch, x, y, scrn, colormaps);
		return;
	}
#endif

	if (vid.dupx > 1 && vid.dupy > 1)
	{
		dupx = vid.dupx / 2;
		dupy = vid.dupy / 2;
	}
	else
	{
		dupx = dupy = 1;
		skippixels = true;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (skippixels)
		colfrac = FixedDiv(FRACUNIT, (dupx)<<(FRACBITS-1));
	else
		colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);

	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	if (scrn & V_NOSCALESTART)
		desttop = screens[scrn&0xFF] + (y * vid.width) + x;
	else
		desttop = screens[scrn&0xFF] + (y * vid.dupy * vid.width) + (x * vid.dupx);

	deststop = screens[scrn&0xFF] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (!(scrn & V_NOSCALESTART))
	{
		/// \bug yeah... the Y still seems to be off a few lines...
		/// see rankings in 640x480 or 800x600
		if (vid.fdupx != vid.dupx)
		{
			// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
			// so center this imaginary screen
			if (scrn & V_SNAPTORIGHT)
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx));
			else if (!(scrn & V_SNAPTOLEFT))
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx)) / 2;
		}
		if (vid.fdupy != dupy)
		{
			// same thing here
			if (scrn & V_SNAPTOBOTTOM)
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width;
			else if (!(scrn & V_SNAPTOTOP))
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width / 2;
		}

		// if it's meant to cover the whole screen, black out the rest
		if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH*2 && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT*2)
			V_DrawFill(0, 0, vid.width, vid.height, 31);
	}

	if (skippixels)
		destend = desttop + SHORT(patch->width)/2 * dupx;
	else
		destend = desttop + SHORT(patch->width) * dupx;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)(patch) + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)(column) + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;
			skiprowcnt = 0;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto donesmalling;

					skiprowcnt++;
				} while (count--);
			}
			else
			{
				while (count--)
				{
					if (dest < deststop)
						*dest = source[ofs>>FRACBITS];
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					skiprowcnt++;
				}
			}
donesmalling:
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

// Draws a patch 2x as small, translucent, and colormapped.
void V_DrawSmallTranslucentMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;
	boolean skippixels = false;
	INT32 skiprowcnt;
	UINT8 *translevel;

	if (scrn & V_8020TRANS)
		translevel = ((tr_trans80)<<FF_TRANSSHIFT) - 0x10000 + transtables;
	else
		translevel = ((tr_trans50)<<FF_TRANSSHIFT) - 0x10000 + transtables;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		if (!(scrn & V_NOSCALESTART)) // Graue 07-08-2004: I have no idea why this works
		{
			x = (INT32)(vid.fdupx*x);
			y = (INT32)(vid.fdupy*y);
			scrn |= V_NOSCALESTART;
		}
		HWR_DrawSmallPatch((GLPatch_t *)patch, x, y, scrn, colormap);
		return;
	}
#endif

	if (vid.dupx > 1 && vid.dupy > 1)
	{
		dupx = vid.dupx / 2;
		dupy = vid.dupy / 2;
	}
	else
	{
		dupx = dupy = 1;
		skippixels = true;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (skippixels)
		colfrac = FixedDiv(FRACUNIT, (dupx)<<(FRACBITS-1));
	else
		colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);

	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	if (scrn & V_NOSCALESTART)
		desttop = screens[scrn&0xFF] + (y * vid.width) + x;
	else
		desttop = screens[scrn&0xFF] + (y * vid.dupy * vid.width) + (x * vid.dupx);

	deststop = screens[scrn&0xFF] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (!(scrn & V_NOSCALESTART))
	{
		/// \bug yeah... the Y still seems to be off a few lines...
		/// see rankings in 640x480 or 800x600
		if (vid.fdupx != vid.dupx)
		{
			// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
			// so center this imaginary screen
			if (scrn & V_SNAPTORIGHT)
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx));
			else if (!(scrn & V_SNAPTOLEFT))
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx)) / 2;
		}
		if (vid.fdupy != dupy)
		{
			// same thing here
			if (scrn & V_SNAPTOBOTTOM)
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width;
			else if (!(scrn & V_SNAPTOTOP))
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width / 2;
		}

		// if it's meant to cover the whole screen, black out the rest
		if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH*2 && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT*2)
			V_DrawFill(0, 0, vid.width, vid.height, 31);
	}

	if (skippixels)
		destend = desttop + SHORT(patch->width)/2 * dupx;
	else
		destend = desttop + SHORT(patch->width) * dupx;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)(patch) + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)(column) + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;
			skiprowcnt = 0;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = *(translevel + (colormap[source[ofs>>FRACBITS]]<<8) + (*dest));
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto donesmallmapping;

					skiprowcnt++;
				} while (count--);
			}
			else
			{
				while (count--)
				{
					if (dest < deststop)
						*dest = *(translevel + (colormap[source[ofs>>FRACBITS]]<<8) + (*dest));
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					skiprowcnt++;
				}
			}
donesmallmapping:
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

// Draws a patch 2x as small, and translucent.
void V_DrawSmallTranslucentPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;
	UINT8 *translevel;
	boolean skippixels = false;
	INT32 skiprowcnt;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		if (!(scrn & V_NOSCALESTART)) // Graue 07-08-2004: I have no idea why this works
		{
			x = (INT32)(vid.fdupx*x);
			y = (INT32)(vid.fdupy*y);
			scrn |= V_NOSCALESTART;
		}
		HWR_DrawSmallPatch((GLPatch_t *)patch, x, y, scrn, colormaps);
		return;
	}
#endif

	if (scrn & V_8020TRANS)
		translevel = ((tr_trans80)<<FF_TRANSSHIFT) - 0x10000 + transtables;
	else
		translevel = ((tr_trans50)<<FF_TRANSSHIFT) - 0x10000 + transtables;

	if (vid.dupx > 1 && vid.dupy > 1)
	{
		dupx = vid.dupx / 2;
		dupy = vid.dupy / 2;
	}
	else
	{
		dupx = dupy = 1;
		skippixels = true;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (skippixels)
		colfrac = FixedDiv(FRACUNIT, (dupx)<<(FRACBITS-1));
	else
		colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);

	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	if (scrn & V_NOSCALESTART)
		desttop = screens[scrn&0xFF] + (y * vid.width) + x;
	else
		desttop = screens[scrn&0xFF] + (y * vid.dupy * vid.width) + (x * vid.dupx);

	deststop = screens[scrn&0xFF] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (!(scrn & V_NOSCALESTART))
	{
		/// \bug yeah... the Y still seems to be off a few lines...
		/// see rankings in 640x480 or 800x600
		if (vid.fdupx != vid.dupx)
		{
			// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
			// so center this imaginary screen
			if (scrn & V_SNAPTORIGHT)
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx));
			else if (!(scrn & V_SNAPTOLEFT))
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx)) / 2;
		}
		if (vid.fdupy != dupy)
		{
			// same thing here
			if (scrn & V_SNAPTOBOTTOM)
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width;
			else if (!(scrn & V_SNAPTOTOP))
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width / 2;
		}

		// if it's meant to cover the whole screen, black out the rest
		if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH*2 && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT*2)
			V_DrawFill(0, 0, vid.width, vid.height, 31);
	}

	if (skippixels)
		destend = desttop + SHORT(patch->width)/2 * dupx;
	else
		destend = desttop + SHORT(patch->width) * dupx;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)(patch) + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)(column) + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;
			skiprowcnt = 0;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = *(translevel + ((source[ofs>>FRACBITS]<<8)&0xff00) + (*dest&0xff));
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto donesmallmapping;

					skiprowcnt++;
				} while (count--);
			}
			else
			{
				while (count--)
				{
					if (dest < deststop)
						*dest = *(translevel + ((source[ofs>>FRACBITS]<<8)&0xff00) + (*dest&0xff));
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					skiprowcnt++;
				}
			}
donesmallmapping:
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

// Draws a patch 2x as small, and colormapped.
void V_DrawSmallMappedPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch, const UINT8 *colormap)
{
	size_t count;
	INT32 col, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest, *destend;
	const UINT8 *source, *deststop;
	boolean skippixels = false;
	INT32 skiprowcnt;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		if (!(scrn & V_NOSCALESTART)) // Graue 07-08-2004: I have no idea why this works
		{
			x = (INT32)(vid.fdupx*x);
			y = (INT32)(vid.fdupy*y);
			scrn |= V_NOSCALESTART;
		}
		HWR_DrawSmallPatch((GLPatch_t *)patch, x, y, scrn, colormap);
		return;
	}
#endif

	if (vid.dupx > 1 && vid.dupy > 1)
	{
		dupx = vid.dupx / 2;
		dupy = vid.dupy / 2;
	}
	else
	{
		dupx = dupy = 1;
		skippixels = true;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (skippixels)
		colfrac = FixedDiv(FRACUNIT, (dupx)<<(FRACBITS-1));
	else
		colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);

	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	if (scrn & V_NOSCALESTART)
		desttop = screens[scrn&0xFF] + (y * vid.width) + x;
	else
		desttop = screens[scrn&0xFF] + (y * vid.dupy * vid.width) + (x * vid.dupx);

	deststop = screens[scrn&0xFF] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (!(scrn & V_NOSCALESTART))
	{
		/// \bug yeah... the Y still seems to be off a few lines...
		/// see rankings in 640x480 or 800x600
		if (vid.fdupx != vid.dupx)
		{
			// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
			// so center this imaginary screen
			if (scrn & V_SNAPTORIGHT)
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx));
			else if (!(scrn & V_SNAPTOLEFT))
				desttop += (vid.width - (BASEVIDWIDTH * vid.dupx)) / 2;
		}
		if (vid.fdupy != dupy)
		{
			// same thing here
			if (scrn & V_SNAPTOBOTTOM)
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width;
			else if (!(scrn & V_SNAPTOTOP))
				desttop += (vid.height - (BASEVIDHEIGHT * vid.dupy)) * vid.width / 2;
		}

		// if it's meant to cover the whole screen, black out the rest
		if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH*2 && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT*2)
			V_DrawFill(0, 0, vid.width, vid.height, 31);
	}

	if (skippixels)
		destend = desttop + SHORT(patch->width)/2 * dupx;
	else
		destend = desttop + SHORT(patch->width) * dupx;

	for (col = 0; desttop < destend; col += colfrac, desttop++)
	{
		register INT32 heightmask;

		column = (const column_t *)((const UINT8 *)(patch) + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)(column) + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;
			skiprowcnt = 0;

			ofs = 0;

			heightmask = column->length - 1;

			if (column->length & heightmask)
			{
				heightmask++;
				heightmask <<= FRACBITS;

				if (rowfrac < 0)
					while ((rowfrac += heightmask) < 0)
						;
				else
					while (rowfrac >= heightmask)
						rowfrac -= heightmask;

				do
				{
					if (dest < deststop)
						*dest = *(colormap + source[ofs>>FRACBITS]);
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					if ((ofs + rowfrac) > heightmask)
						goto donesmallmapping;

					skiprowcnt++;
				} while (count--);
			}
			else
			{
				while (count--)
				{
					if (dest < deststop)
						*dest = *(colormap + source[ofs>>FRACBITS]);
					else
						count = 0;

					if (!(skippixels && (skiprowcnt & 1)))
						dest += vid.width;

					ofs += rowfrac;
					skiprowcnt++;
				}
			}
donesmallmapping:

			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

// This draws a patch over a background with translucency...SCALED.
// SCALE THE STARTING COORDS!
// Used for crosshair.
//
void V_DrawTranslucentPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, w, dupx, dupy, ofs, colfrac, rowfrac;
	const column_t *column;
	UINT8 *desttop, *dest;
	const UINT8 *source, *translevel, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawTranslucentPatch((GLPatch_t *)patch, x, y, scrn);
		return;
	}
#endif

	if (scrn & V_8020TRANS)
		translevel = ((tr_trans80)<<FF_TRANSSHIFT) - 0x10000 + transtables;
	else
		translevel = ((tr_trans50)<<FF_TRANSSHIFT) - 0x10000 + transtables;

	if ((scrn & V_NOSCALEPATCH))
		dupx = dupy = 1;
	else
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
	}

	if (scrn & V_TOPLEFT)
	{
		y -= SHORT(patch->topoffset);
		x -= SHORT(patch->leftoffset);
	}
	else
	{
		y -= SHORT(patch->topoffset)*dupy;
		x -= SHORT(patch->leftoffset)*dupx;
	}

	colfrac = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	rowfrac = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	desttop = screens[scrn&0xffff];
	deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;

	if (!desttop)
		return;

	if (scrn & V_NOSCALESTART)
		desttop += (y*vid.width) + x;
	else
	{
		desttop += (y*dupy*vid.width) + (x*dupx);

		// Center it if necessary
		if (!(scrn & V_NOSCALEPATCH))
		{
			if (vid.fdupx != dupx)
			{
				// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
				// so center this imaginary screen
				if (scrn & V_SNAPTORIGHT)
					desttop += (vid.width - (BASEVIDWIDTH * dupx));
				else if (!(scrn & V_SNAPTOLEFT))
					desttop += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
			}
			if (vid.fdupy != dupy)
			{
				// same thing here
				if (scrn & V_SNAPTOBOTTOM)
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width;
				else if (!(scrn & V_SNAPTOTOP))
					desttop += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
			}
			// if it's meant to cover the whole screen, black out the rest
			if (x == 0 && SHORT(patch->width) == BASEVIDWIDTH && y == 0 && SHORT(patch->height) == BASEVIDHEIGHT)
				V_DrawFill(0, 0, vid.width, vid.height, 31);
		}
	}

	w = SHORT(patch->width)<<FRACBITS;

	for (col = 0; col < w; col += colfrac, desttop++)
	{
		column = (const column_t *)((const UINT8 *)patch + LONG(patch->columnofs[col>>FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			dest = desttop + column->topdelta*dupy*vid.width;
			count = column->length*dupy;

			ofs = 0;
			while (count--)
			{
				if (dest < deststop)
					*dest = *(translevel + ((source[ofs>>FRACBITS]<<8)&0xff00) + (*dest&0xff));
				else
					count = 0;
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. NO SCALING!
//
void V_DrawPatch(INT32 x, INT32 y, INT32 scrn, patch_t *patch)
{
	size_t count;
	INT32 col, w;
	const column_t *column;
	UINT8 *desttop, *dest;
	const UINT8 *source, *deststop;

#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawPatch((GLPatch_t *)patch, x, y, V_NOSCALESTART|V_NOSCALEPATCH);
		return;
	}
#endif

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
#ifdef RANGECHECK
	if (x < 0 || x + SHORT(patch->width) > vid.width || y < 0
		|| y + SHORT(patch->height) > vid.height || (unsigned)scrn > 4)
	{
		fprintf(stderr, "Patch at %d, %d exceeds LFB\n", x, y);
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(stderr, "V_DrawPatch: bad patch (ignored)\n");
		return;
	}
#endif

	desttop = screens[scrn] + y*vid.width + x;
	deststop = screens[scrn&0xffff] + vid.width * vid.height * vid.bpp;
	w = SHORT(patch->width);

	for (col = 0; col < w; x++, col++, desttop++)
	{
		column = (const column_t *)((const UINT8 *)patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			dest = desttop + column->topdelta*vid.width;
			count = column->length;

			while (count--)
			{
				if (dest < deststop)
					*dest = *source++;
				else
					count = 0;
				dest += vid.width;
			}
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock(INT32 x, INT32 y, INT32 scrn, INT32 width, INT32 height, const UINT8 *src)
{
	UINT8 *dest;
	const UINT8 *deststop;

#ifdef RANGECHECK
	if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned)scrn > 4)
		I_Error("Bad V_DrawBlock");
#endif

	dest = screens[scrn] + y*vid.width + x;
	deststop = screens[scrn] + vid.width * vid.height * vid.bpp;

	while (height--)
	{
		M_Memcpy(dest, src, width);

		src += width;
		dest += vid.width;
		if (dest > deststop)
			return;
	}
}

static void V_BlitScaledPic(INT32 px1, INT32 py1, INT32 scrn, pic_t *pic);
//  Draw a linear pic, scaled, TOTALLY CRAP CODE!!! OPTIMISE AND ASM!!
//
void V_DrawScaledPic(INT32 rx1, INT32 ry1, INT32 scrn, INT32 lumpnum)
{
#ifdef HWRENDER
	if (rendermode != render_soft)
	{
		HWR_DrawPic(rx1, ry1, lumpnum);
		return;
	}
#endif

	V_BlitScaledPic(rx1, ry1, scrn, W_CacheLumpNum(lumpnum, PU_CACHE));
}

static void V_BlitScaledPic(INT32 rx1, INT32 ry1, INT32 scrn, pic_t * pic)
{
	INT32 dupx, dupy;
	INT32 x, y;
	UINT8 *src, *dest;
	INT32 width, height;

	width = SHORT(pic->width);
	height = SHORT(pic->height);
	scrn &= 0xffff;

	if (pic->mode != 0)
	{
		CONS_Printf("pic mode %d not supported in Software\n", pic->mode);
		return;
	}

	dest = screens[scrn] + max(0, ry1 * vid.width) + max(0, rx1);
	// y cliping to the screen
	if (ry1 + height * vid.dupy >= vid.width)
		height = (vid.width - ry1) / vid.dupy - 1;
	// WARNING no x clipping (not needed for the moment)

	for (y = max(0, -ry1 / vid.dupy); y < height; y++)
	{
		for (dupy = vid.dupy; dupy; dupy--)
		{
			src = pic->data + y * width;
			for (x = 0; x < width; x++)
			{
				for (dupx = vid.dupx; dupx; dupx--)
					*dest++ = *src;
				src++;
			}
			dest += vid.width - vid.dupx * width;
		}
	}
}

//
// Fills a box of pixels with a single color, NOTE: scaled to screen size
//
void V_DrawFill(INT32 x, INT32 y, INT32 w, INT32 h, INT32 c)
{
	UINT8 *dest;
	const UINT8 *deststop;
	INT32 u, v, dupx, dupy;

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawFill(x, y, w, h, c);
		return;
	}
#endif

	dupx = vid.dupx;
	dupy = vid.dupy;

	if (!screens[0])
		return;

	dest = screens[0] + y*dupy*vid.width + x*dupx;
	deststop = screens[0] + vid.width * vid.height * vid.bpp;

	w *= dupx;
	h *= dupy;

	if (x && y && x + w < vid.width && y + h < vid.height)
	{
		// Center it if necessary
		if (vid.fdupx != dupx)
		{
			// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
			// so center this imaginary screen
			if (c & V_SNAPTORIGHT)
				dest += (vid.width - (BASEVIDWIDTH * dupx));
			else if (!(c & V_SNAPTOLEFT))
				dest += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
		}
		if (vid.fdupy != dupy)
		{
			// same thing here
			if (c & V_SNAPTOBOTTOM)
				dest += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width;
			else if (!(c & V_SNAPTOTOP))
				dest += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
		}
	}

	c &= 255;

	for (v = 0; v < h; v++, dest += vid.width)
		for (u = 0; u < w; u++)
		{
			if (dest > deststop)
				return;
			dest[u] = (UINT8)c;
		}
}

//
// Fills a box of pixels using a flat texture as a pattern, scaled to screen size.
//
void V_DrawFlatFill(INT32 x, INT32 y, INT32 w, INT32 h, lumpnum_t flatnum)
{
	INT32 u, v, dupx, dupy;
	fixed_t dx, dy, xfrac, yfrac;
	const UINT8 *src, *deststop;
	UINT8 *flat, *dest;
	size_t size, lflatsize, flatshift;

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_DrawFlatFill(x, y, w, h, flatnum);
		return;
	}
#endif

	size = W_LumpLength(flatnum);

	switch (size)
	{
		case 4194304: // 2048x2048 lump
			lflatsize = 2048;
			flatshift = 10;
			break;
		case 1048576: // 1024x1024 lump
			lflatsize = 1024;
			flatshift = 9;
			break;
		case 262144:// 512x512 lump
			lflatsize = 512;
			flatshift = 8;
			break;
		case 65536: // 256x256 lump
			lflatsize = 256;
			flatshift = 7;
			break;
		case 16384: // 128x128 lump
			lflatsize = 128;
			flatshift = 7;
			break;
		case 1024: // 32x32 lump
			lflatsize = 32;
			flatshift = 5;
			break;
		default: // 64x64 lump
			lflatsize = 64;
			flatshift = 6;
			break;
	}

	flat = W_CacheLumpNum(flatnum, PU_CACHE);

	dupx = vid.dupx;
	dupy = vid.dupy;

	dest = screens[0] + y*dupy*vid.width + x*dupx;
	deststop = screens[0] + vid.width * vid.height * vid.bpp;

	// from V_DrawScaledPatch
	if (vid.fdupx != vid.dupx)
	{
		// dupx adjustments pretend that screen width is BASEVIDWIDTH * dupx,
		// so center this imaginary screen
		dest += (vid.width - (BASEVIDWIDTH * dupx)) / 2;
	}
	if (vid.fdupy != vid.dupy)
	{
		// same thing here
		dest += (vid.height - (BASEVIDHEIGHT * dupy)) * vid.width / 2;
	}

	w *= dupx;
	h *= dupy;

	dx = FixedDiv(FRACUNIT, dupx<<FRACBITS);
	dy = FixedDiv(FRACUNIT, dupy<<FRACBITS);

	yfrac = 0;
	for (v = 0; v < h; v++, dest += vid.width)
	{
		xfrac = 0;
		src = flat + (((yfrac >> (FRACBITS - 1)) & (lflatsize - 1)) << flatshift);
		for (u = 0; u < w; u++)
		{
			if (&dest[u] > deststop)
				return;
			dest[u] = src[(xfrac>>FRACBITS)&(lflatsize-1)];
			xfrac += dx;
		}
		yfrac += dy;
	}
}

//
// V_DrawPatchFill
//
void V_DrawPatchFill(patch_t *pat)
{
	INT32 x, y, pw = SHORT(pat->width) * vid.dupx, ph = SHORT(pat->height) * vid.dupy;

	for (x = 0; x < vid.width; x += pw)
	{
		for (y = 0; y < vid.height; y += ph)
		{
			if (x + pw >= vid.width || y + ph >= vid.height)
				V_DrawClippedScaledPatch(x, y, 0, pat); // V_NOSCALESTART is implied
			else
				V_DrawScaledPatch(x, y, V_NOSCALESTART, pat);
		}
	}
}

//
// Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
void V_DrawFadeScreen(void)
{
	INT32 x, y, w;
	INT32 *buf;
	UINT32 quad;
	UINT8 p1, p2, p3, p4;
	const UINT8 *fadetable = (UINT8 *)colormaps + 16*256, *deststop = screens[0] + vid.width * vid.height * vid.bpp;

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
	{
		HWR_FadeScreenMenuBack(0x01010160, 0); // hack, 0 means full height
		return;
	}
#endif

	w = vid.width>>2;
	for (y = 0; y < vid.height; y++)
	{
		buf = (INT32 *)(void *)(screens[0] + vid.width*y);
		for (x = 0; x < w; x++)
		{
			if (buf+ x > (const INT32 *)(const void *)deststop)
				return;
			M_Memcpy(&quad,buf+x,sizeof (quad)); //quad = buf[x];
			p1 = fadetable[quad&255];
			p2 = fadetable[(quad>>8)&255];
			p3 = fadetable[(quad>>16)&255];
			p4 = fadetable[quad>>24];
			quad = (p4<<24) | (p3<<16) | (p2<<8) | p1;//buf[x] = (p4<<24) | (p3<<16) | (p2<<8) | p1;
			M_Memcpy(buf+x,&quad,sizeof (quad));
		}
	}
}

// Simple translucency with one color. Coords are resolution dependent.
//
void V_DrawFadeConsBack(INT32 px1, INT32 py1, INT32 px2, INT32 py2, INT32 color)
{
	INT32 x, y, w;
	INT32 *buf;
	UINT32 quad;
	UINT8 p1, p2, p3, p4;
	INT16 *wput;
	const UINT8 *deststop = screens[0] + vid.width * vid.height * vid.bpp;
	UINT8 *colormap;

#ifdef HWRENDER // not win32 only 19990829 by Kin
	if (rendermode != render_soft && rendermode != render_none)
	{
		UINT32 hwcolor;

		switch (color)
		{
			case 0: // white
				hwcolor = 0xffffff00;
				break;
			case 1: // orange
				hwcolor = 0xff800000;
				break;
			case 2: // blue
				hwcolor = 0x0000ff00;
				break;
			case 3: // green
				hwcolor = 0x00800000;
				break;
			case 4: // gray
				hwcolor = 0x80808000;
				break;
			case 5: // red
				hwcolor = 0xff000000;
				break;
			default:
				hwcolor = 0x00800000;
				break;
		}

		HWR_DrawConsoleBack(hwcolor, py2);
		return;
	}
#endif

	switch (color)
	{
		case 0:
			colormap = cwhitemap;
			break;
		case 1:
			colormap = corangemap;
			break;
		case 2:
			colormap = cbluemap;
			break;
		case 3:
			colormap = cgreenmap;
			break;
		case 4:
			colormap = cgraymap;
			break;
		case 5:
			colormap = credmap;
			break;
		default:
			colormap = cgreenmap;
			break;
	}

	if (scr_bpp == 1)
	{
		px1 >>=2;
		px2 >>=2;
		for (y = py1; y < py2; y++)
		{
			buf = (INT32 *)(void *)(screens[0] + vid.width*y);
			for (x = px1; x < px2; x++)
			{
				if (&buf[x] > (const INT32 *)(const void *)deststop)
					return;
				M_Memcpy(&quad,buf+x,sizeof (quad)); //quad = buf[x];
				p1 = colormap[quad&255];
				p2 = colormap[(quad>>8)&255];
				p3 = colormap[(quad>>16)&255];
				p4 = colormap[quad>>24];
				quad = (p4<<24) | (p3<<16) | (p2<<8) | p1;//buf[x] = (p4<<24) | (p3<<16) | (p2<<8) | p1;
				M_Memcpy(buf+x, &quad, sizeof (quad));
			}
		}
	}
	else
	{
		w = px2 - px1;
		for (y = py1; y < py2; y++)
		{
			wput = (INT16 *)(void *)(screens[0] + vid.width*y) + px1;
			for (x = 0; x < w; x++)
			{
				if (wput > (const INT16 *)(const void *)deststop)
					return;
				*wput = (INT16)(((*wput&0x7bde) + (15<<5)) >>1);
				wput++;
			}
		}
	}
}

// Writes a single character (draw WHITE if bit 7 set)
//
void V_DrawCharacter(INT32 x, INT32 y, INT32 c, boolean lowercaseallowed)
{
	INT32 w, flags;
	const UINT8 *colormap = NULL;

	switch (c & 0xff00)
	{
	case 0x100: // 0x81, purple
		colormap = purplemap;
		break;
	case 0x200: // 0x82, yellow
		colormap = yellowmap;
		break;
	case 0x300: // 0x83, lgreen
		colormap = lgreenmap;
		break;
	case 0x400: // 0x84, blue
		colormap = bluemap;
		break;
	case 0x500: // 0x85, red
		colormap = redmap;
		break;
	case 0x600: // 0x86, gray
		colormap = graymap;
		break;
	case 0x700: // 0x87, orange
		colormap = orangemap;
		break;
	}

	flags = c & 0xffff0000;
	c &= 0x7f;
	if (lowercaseallowed)
		c -= HU_FONTSTART;
	else
		c = toupper(c) - HU_FONTSTART;
	if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART))
		return;

	w = SHORT(hu_font[c]->width);
	if (x + w > vid.width)
		return;

	if (colormap != NULL)
		V_DrawMappedPatch(x, y, flags, hu_font[c], colormap);
	else
		V_DrawScaledPatch(x, y, flags, hu_font[c]);
}

//
// Write a string using the hu_font
// NOTE: the text is centered for screens larger than the base width
//
static void V_DrawWordWrapString(INT32 x, INT32 y, INT32 option, const char *string)
{
	char newstring[1024];
	int c;
	size_t nx = x, i, lastusablespace = 0;

	strncpy(newstring, string, 1024);

	for (i = 0; i < strlen(newstring); i++)
	{
		c = newstring[i];
		if ((UINT8)c >= 0x80 && (UINT8)c <= 0x89) //color parsing! -Inuyasha 2.16.09
			continue;

		c = toupper(c) - HU_FONTSTART;
		if (c == '\n')
		{
			nx = x;
			lastusablespace = 0;
			continue;
		}
		else if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART)
			|| hu_font[c] == NULL)
		{
			nx += 4;
			lastusablespace = i;
		}
		else
			nx += 8;

		if (lastusablespace != 0 && nx > BASEVIDWIDTH-8)
		{
			newstring[lastusablespace] = '\n';
			nx = x + ((i-lastusablespace)*8);
			lastusablespace = 0;
		}
	}

	//Oh the hilarity.
	//Just call V_DrawString with the new string...
	//Clear word wrap flag, first, though.  Obvious reasons.
	option &= ~V_WORDWRAP;
	V_DrawString(x, y, option, newstring);
}

//
// Write a string using the hu_font
// NOTE: the text is centered for screens larger than the base width
//
void V_DrawString(INT32 x, INT32 y, INT32 option, const char *string)
{
	INT32 w, c, cx = x, cy = y, dupx, dupy, scrwidth = BASEVIDWIDTH;
	const char *ch = string;
	UINT8 lastcolorchar = 0x80; //for dynamic colors
	const UINT8 *colormap = NULL;

	if (option & V_WORDWRAP)
	{
		V_DrawWordWrapString(x, y, option, string);
		return;
	}

	if (option & V_NOSCALESTART)
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
		scrwidth = vid.width;
	}
	else
		dupx = dupy = 1;

	for (;;)
	{
		c = *ch++;
		if (!c)
			break;
		if ((UINT8)c >= 0x80 && (UINT8)c <= 0x89) //color parsing -x 2.16.09
		{
			lastcolorchar = (UINT8)c;
			continue;
		}
		if (c == '\n')
		{
			cx = x;

			if (option & V_RETURN8)
				cy += 8*dupy;
			else
				cy += 12*dupy;

			continue;
		}

		if (option & V_ALLOWLOWERCASE)
			c -= HU_FONTSTART;
		else
			c = toupper(c) - HU_FONTSTART;
		if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART))
		{
			cx += 4*dupx;
			continue;
		}

		w = SHORT(hu_font[c]->width) * dupx;
		if (cx + w > scrwidth)
			break;
		if (cx < 0) //left boundary check
		{
			cx += w;
			continue;
		}

		if ((option & V_YELLOWMAP) || lastcolorchar == 0x82)
			colormap = yellowmap;
		else if ((option & V_GREENMAP) || lastcolorchar == 0x83)
			colormap = lgreenmap;
		else if (lastcolorchar == 0x81)
			colormap = purplemap;
		else if (lastcolorchar == 0x84)
			colormap = bluemap;
		else if (lastcolorchar == 0x85)
			colormap = redmap;
		else if (lastcolorchar == 0x86)
			colormap = graymap;
		else if (lastcolorchar == 0x87)
			colormap = orangemap;
		else if (lastcolorchar == 0x80)
			colormap = NULL;

		if (colormap != NULL && ((option & V_TRANSLUCENT) || (option & V_8020TRANS)))
			V_DrawTranslucentMappedPatch(cx, cy, option, hu_font[c], colormap);
		else if (colormap != NULL)
			V_DrawMappedPatch(cx, cy, option, hu_font[c], colormap);
		else if ((option & V_TRANSLUCENT) || (option & V_8020TRANS))
			V_DrawTranslucentPatch(cx, cy, option & ~V_TRANSLUCENT, hu_font[c]);
		else
			V_DrawScaledPatch(cx, cy, option, hu_font[c]);

		cx += w;
	}
}

void V_DrawCenteredString(INT32 x, INT32 y, INT32 option, const char *string)
{
	x -= V_StringWidth(string)/2;
	V_DrawString(x, y, option, string);
}

void V_DrawRightAlignedString(INT32 x, INT32 y, INT32 option, const char *string)
{
	x -= V_StringWidth(string);
	V_DrawString(x, y, option, string);
}

// Draw a tiny number, yay.
//
void V_DrawTinyNum(INT32 x, INT32 y, INT32 c, INT32 num)
{
	INT32 w = SHORT(tinynum[0]->width);
	INT32 tempnum = num;

	// special case for 0
	if (!num)
	{
		if ((c & V_YELLOWMAP) && ((c & V_TRANSLUCENT) || (c & V_8020TRANS)))
			V_DrawTranslucentMappedPatch(x, y, c, tinynum[0], yellowmap);
		else if ((c & V_GREENMAP) && ((c & V_TRANSLUCENT) || (c & V_8020TRANS)))
			V_DrawTranslucentMappedPatch(x, y, c, tinynum[0], lgreenmap);
		else if (c & V_YELLOWMAP)
			V_DrawMappedPatch(x, y, c, tinynum[0], yellowmap);
		else if (c & V_GREENMAP)
			V_DrawMappedPatch(x, y, c, tinynum[0], lgreenmap);
		else if ((c & V_TRANSLUCENT) || (c & V_8020TRANS))
			V_DrawTranslucentPatch(x, y, c & ~V_TRANSLUCENT, tinynum[0]);
		else
			V_DrawScaledPatch(x, y, c, tinynum[0]);
		return;
	}

	I_Assert(num >= 0); // this function does not draw negative numbers

	// Position the string correctly.
	while (tempnum)
	{
		x += w;
		tempnum /= 10;
	}

	// draw the number
	while (num)
	{
		x -= w;

		if ((c & V_YELLOWMAP) && ((c & V_TRANSLUCENT) || (c & V_8020TRANS)))
			V_DrawTranslucentMappedPatch(x, y, c, tinynum[num % 10], yellowmap);
		else if ((c & V_GREENMAP) && ((c & V_TRANSLUCENT) || (c & V_8020TRANS)))
			V_DrawTranslucentMappedPatch(x, y, c, tinynum[num % 10], lgreenmap);
		else if (c & V_YELLOWMAP)
			V_DrawMappedPatch(x, y, c, tinynum[num % 10], yellowmap);
		else if (c & V_GREENMAP)
			V_DrawMappedPatch(x, y, c, tinynum[num % 10], lgreenmap);
		else if ((c & V_TRANSLUCENT) || (c & V_8020TRANS))
			V_DrawTranslucentPatch(x, y, c & ~V_TRANSLUCENT, tinynum[num % 10]);
		else
			V_DrawScaledPatch(x, y, c, tinynum[num % 10]);

		num /= 10;
	}
}

// Write a string using the credit font
// NOTE: the text is centered for screens larger than the base width
//
void V_DrawCreditString(INT32 x, INT32 y, INT32 option, const char *string)
{
	INT32 w, c, cx = x, cy = y, dupx, dupy, scrwidth = BASEVIDWIDTH;
	const char *ch = string;

	if (option & V_NOSCALESTART)
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
		scrwidth = vid.width;
	}
	else
		dupx = dupy = 1;

	for (;;)
	{
		c = *ch++;
		if (!c)
			break;
		if (c == '\n')
		{
			cx = x;
			cy += 12*dupy;
			continue;
		}

		c = toupper(c) - CRED_FONTSTART;
		if (c < 0 || c >= CRED_FONTSIZE)
		{
			cx += 16*dupx;
			continue;
		}

		w = SHORT(cred_font[c]->width) * dupx;
		if (cx + w > scrwidth)
			break;

		V_DrawScaledPatch(cx, cy, option, cred_font[c]);
		cx += w;
	}
}

// Find string width from cred_font chars
//
INT32 V_CreditStringWidth(const char *string)
{
	INT32 c, w = 0;
	size_t i;

	for (i = 0; i < strlen(string); i++)
	{
		c = toupper(string[i]) - CRED_FONTSTART;
		if (c < 0 || c >= CRED_FONTSIZE)
			w += 8;
		else
			w += SHORT(cred_font[c]->width);
	}

	return w;
}

// Write a string using the level title font
// NOTE: the text is centered for screens larger than the base width
//
void V_DrawLevelTitle(INT32 x, INT32 y, INT32 option, const char *string)
{
	INT32 w, c, cx = x, cy = y, dupx, dupy, scrwidth = BASEVIDWIDTH;
	const char *ch = string;

	if (option & V_NOSCALESTART)
	{
		dupx = vid.dupx;
		dupy = vid.dupy;
		scrwidth = vid.width;
	}
	else
		dupx = dupy = 1;

	for (;;)
	{
		c = *ch++;
		if (!c)
			break;
		if (c == '\n')
		{
			cx = x;
			cy += 12*dupy;
			continue;
		}

		c = toupper(c);
		if ((c != LT_FONTSTART && (c < '0' || c > '9')) && (c < LT_REALFONTSTART || c > LT_FONTEND))
		{ /// \note font start hack
			cx += 16*dupx;
			continue;
		}

		c -= LT_FONTSTART;

		w = SHORT(lt_font[c]->width) * dupx;
		if (cx + w > scrwidth)
			break;

		V_DrawScaledPatch(cx, cy, option, lt_font[c]);
		cx += w;
	}
}

// Find string width from lt_font chars
//
INT32 V_LevelNameWidth(const char *string)
{
	INT32 c, w = 0;
	size_t i;

	for (i = 0; i < strlen(string); i++)
	{
		c = toupper(string[i]) - LT_FONTSTART;
		if (c < 0 || (c > 0 && c < LT_REALFONTSTART - LT_FONTSTART) || c >= LT_FONTSIZE)
			w += 16;
		else
			w += SHORT(lt_font[c]->width);
	}

	return w;
}

// Find max height of the string
//
INT32 V_LevelNameHeight(const char *string)
{
	INT32 c, w = 0;
	size_t i;

	for (i = 0; i < strlen(string); i++)
	{
		c = toupper(string[i]) - LT_FONTSTART;
		if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART)
		    || hu_font[c] == NULL)
			continue;

		if (SHORT(lt_font[c]->height) > w)
			w = SHORT(lt_font[c]->height);
	}

	return w;
}

//
// Find string width from hu_font chars
//
INT32 V_StringWidth(const char *string)
{
	INT32 c, w = 0;
	size_t i;

	for (i = 0; i < strlen(string); i++)
	{
		c = string[i];
		if ((UINT8)c >= 0x80 && (UINT8)c <= 0x89) //color parsing! -Inuyasha 2.16.09
			continue;

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART)
			|| hu_font[c] == NULL)
		{
			w += 4;
		}
		else
			w += SHORT(hu_font[c]->width);
	}

	return w;
}

boolean *heatshifter = NULL;
INT32 lastheight = 0;
INT32 heatindex = 0;

//
// V_DoPostProcessor
//
// Perform a particular image postprocessing function.
//
#include "p_local.h"
void V_DoPostProcessor(postimg_t type)
{
#ifdef HWRENDER
	// draw a hardware converted patch
	if (rendermode != render_soft && rendermode != render_none)
		return;
#endif

#if NUMSCREENS < 4
	return; // do not enable image post processing for ARM, SH and MIPS CPUs
#endif

	if (splitscreen) // Not supported in splitscreen - someone want to add support?
		return;

	if (type == postimg_water)
	{
			UINT8 *tmpscr = screens[4];
			UINT8 *srcscr = screens[0];
			INT32 y;
			static angle_t disStart = 0; // in 0 to FINEANGLE
			INT32 newpix;
			INT32 sine;
			INT32 westart = disStart;
			//UINT8 *transme = ((tr_trans50)<<FF_TRANSSHIFT) + transtables;

			for (y = 0; y < vid.height; y++)
			{
				sine = (FINESINE(disStart)*5)>>FRACBITS;
				newpix = abs(sine);

				if (sine < 0)
				{
					M_Memcpy(&tmpscr[y*vid.width+newpix], &srcscr[y*vid.width], vid.width-newpix);

					// Cleanup edge
					while (newpix)
					{
						tmpscr[y*vid.width+newpix] = srcscr[y*vid.width];
						newpix--;
					}
				}
				else
				{
					M_Memcpy(&tmpscr[y*vid.width+0], &srcscr[y*vid.width+sine], vid.width-newpix);

					// Cleanup edge
					while (newpix)
					{
						tmpscr[y*vid.width+vid.width-newpix] = srcscr[y*vid.width+(vid.width-1)];
						newpix--;
					}
				}

/*
Unoptimized version
				for (x = 0; x < vid.width*vid.bpp; x++)
				{
					newpix = (x + sine);

					if (newpix < 0)
						newpix = 0;
					else if (newpix >= vid.width)
						newpix = vid.width-1;

					tmpscr[y*vid.width + x] = srcscr[y*vid.width+newpix]; // *(transme + (srcscr[y*vid.width+x]<<8) + srcscr[y*vid.width+newpix]);
				}*/
				disStart += 22;//the offset into the displacement map, increment each game loop
				disStart &= FINEMASK; //clip it to FINEMASK
			}

			disStart = westart + 128;
			disStart &= FINEMASK;

			VID_BlitLinearScreen(tmpscr, screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.width);
	}
	else if (type == postimg_motion) // Motion Blur!
	{
		UINT8 *tmpscr = screens[4];
		UINT8 *srcscr = screens[0];
		INT32 x, y;

		// TODO: Add a postimg_param so that we can pick the translucency level...
		UINT8 *transme = ((postimgparam)<<FF_TRANSSHIFT) - 0x10000 + transtables;

		for (y = 0; y < vid.height; y++)
		{
			for (x = 0; x < vid.width; x++)
			{
				tmpscr[y*vid.width + x]
					=     colormaps[*(transme     + (srcscr   [y*vid.width+x ] <<8) + (tmpscr[y*vid.width+x]))];
			}
		}
		VID_BlitLinearScreen(tmpscr, screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.width);
	}
	else if (type == postimg_flip) // Flip the screen upside-down
	{
		UINT8 *tmpscr = screens[4];
		UINT8 *srcscr = screens[0];
		INT32 y, y2;

		for (y = 0, y2 = vid.height - 1; y < vid.height; y++, y2--)
			M_Memcpy(&tmpscr[y2*vid.width], &srcscr[y*vid.width], vid.width);

		VID_BlitLinearScreen(tmpscr, screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.width);
	}
	else if (type == postimg_heat) // Heat wave
	{
		UINT8 *tmpscr = screens[4];
		UINT8 *srcscr = screens[0];
		INT32 y;

		// Make sure table is built
		if (heatshifter == NULL || lastheight != vid.height)
		{
			if (heatshifter)
				Z_Free(heatshifter);

			heatshifter = Z_Calloc(vid.height * sizeof(boolean), PU_STATIC, NULL);

			for (y = 0; y < vid.height; y++)
			{
				if (M_Random() < 32)
					heatshifter[y] = true;
			}

			heatindex = 0;
			lastheight = vid.height;
		}

		for (y = 0; y < vid.height; y++)
		{
			if (heatshifter[heatindex++])
			{
				// Shift this row of pixels to the right by 2
				tmpscr[y*vid.width] = srcscr[y*vid.width];
				M_Memcpy(&tmpscr[y*vid.width+vid.dupx], &srcscr[y*vid.width], vid.width-vid.dupx);
			}
			else
				M_Memcpy(&tmpscr[y*vid.width], &srcscr[y*vid.width], vid.width);

			heatindex %= vid.height;
		}

		heatindex++;
		heatindex %= vid.height;

		VID_BlitLinearScreen(tmpscr, screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.width);
	}
}

// V_Init
// old software stuff, buffers are allocated at video mode setup
// here we set the screens[x] pointers accordingly
// WARNING: called at runtime (don't init cvar here)
void V_Init(void)
{
	INT32 i;
	UINT8 *base = vid.buffer;
	const INT32 screensize = vid.width * vid.height * vid.bpp;

	LoadMapPalette();
	// hardware modes do not use screens[] pointers
	for (i = 0; i < NUMSCREENS; i++)
		screens[i] = NULL;
	if (rendermode != render_soft)
	{
		return; // be sure to cause a NULL read/write error so we detect it, in case of..
	}

	// start address of NUMSCREENS * width*height vidbuffers
	if (base)
	{
		for (i = 0; i < NUMSCREENS; i++)
			screens[i] = base + i*screensize;
	}

	if (vid.direct)
		screens[0] = vid.direct;

#ifdef DEBUG
	CONS_Printf("V_Init done:\n");
	for (i = 0; i < NUMSCREENS+1; i++)
		CONS_Printf(" screens[%d] = %x\n", i, screens[i]);
#endif
}
