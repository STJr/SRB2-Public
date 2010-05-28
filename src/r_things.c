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
/// \brief Refresh of things, i.e. objects represented by sprites

#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "r_local.h"
#include "sounds.h" // skin sounds
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_misc.h"
#include "i_video.h" // rendermode
#include "i_system.h" // I_OutputMsg
#include "r_things.h"
#include "r_plane.h"
#include "p_tick.h"
#include "p_local.h"

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

static void R_InitSkins(void);

#define MINZ (FRACUNIT*4)
#define BASEYCENTER (BASEVIDHEIGHT/2)

typedef struct
{
	INT32 x1, x2;
	INT32 column;
	INT32 topclip, bottomclip;
} maskdraw_t;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
static lighttable_t **spritelights;

// constant arrays used for psprite clipping and initializing clipping
INT16 negonearray[MAXVIDWIDTH];
INT16 screenheightarray[MAXVIDWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches
spritedef_t *sprites;
size_t numsprites;

static spriteframe_t sprtemp[64];
static size_t maxframe;
static const char *spritename;

// ==========================================================================
//
// Sprite loading routines: support sprites in pwad, dehacked sprite renaming,
// replacing not all frames of an existing sprite, add sprites at run-time,
// add wads at run-time.
//
// ==========================================================================

//
//
//
static void R_InstallSpriteLump(UINT16 wad,            // graphics patch
                                UINT16 lump,
                                size_t lumpid,      // identifier
                                UINT8 frame,
                                UINT8 rotation,
                                UINT8 flipped)
{
	INT32 r;
	lumpnum_t lumppat = wad;
	lumppat <<= 16;
	lumppat += lump;

	if (frame >= 64 || rotation > 8)
		I_Error("R_InstallSpriteLump: Bad frame characters in lump %s", W_CheckNameForNum(lumppat));

	if (maxframe ==(size_t)-1 || frame > maxframe)
		maxframe = frame;

	if (rotation == 0)
	{
		// the lump should be used for all rotations
		if (sprtemp[frame].rotate == 0 && devparm)
			I_OutputMsg("R_InitSprites: Sprite %s frame %c has multiple rot = 0 lump\n",
				spritename, 'A'+frame);

		if (sprtemp[frame].rotate == 1 && devparm)
			I_OutputMsg("R_InitSprites: Sprite %s frame %c has rotations and a rot = 0 lump\n",
				spritename, 'A'+frame);

		sprtemp[frame].rotate = 0;
		for (r = 0; r < 8; r++)
		{
			sprtemp[frame].lumppat[r] = lumppat;
			sprtemp[frame].lumpid[r] = lumpid;
			sprtemp[frame].flip[r] = flipped;
		}
		return;
	}

	// the lump is only used for one rotation
	if (sprtemp[frame].rotate == 0 && devparm)
		I_OutputMsg("R_InitSprites: Sprite %s frame %c has rotations and a rot = 0 lump\n",
			spritename, 'A'+frame);

	sprtemp[frame].rotate = 1;

	// make 0 based
	rotation--;

	if (sprtemp[frame].lumppat[rotation] != LUMPERROR && devparm)
		I_OutputMsg("R_InitSprites: Sprite %s: %c:%c has two lumps mapped to it\n",
			spritename, 'A'+frame, '1'+rotation);

	// lumppat & lumpid are the same for original Doom, but different
	// when using sprites in pwad : the lumppat points the new graphics
	sprtemp[frame].lumppat[rotation] = lumppat;
	sprtemp[frame].lumpid[rotation] = lumpid;
	sprtemp[frame].flip[rotation] = flipped;
}

static void R_RemoveSpriteLump(UINT16 wad,            // graphics patch
                               UINT16 lump,
                               size_t lumpid,      // identifier
                               UINT8 frame,
                               UINT8 rotation,
                               UINT8 flipped)
{
	(void)wad; /// \todo: how do I remove sprites?
	(void)lump;
	(void)lumpid;
	(void)frame;
	(void)rotation;
	(void)flipped;
}

// Install a single sprite, given its identifying name (4 chars)
//
// (originally part of R_AddSpriteDefs)
//
// Pass: name of sprite : 4 chars
//       spritedef_t
//       wadnum         : wad number, indexes wadfiles[], where patches
//                        for frames are found
//       startlump      : first lump to search for sprite frames
//       endlump        : AFTER the last lump to search
//
// Returns true if the sprite was succesfully added
//
static boolean R_AddSingleSpriteDef(const char *sprname, spritedef_t *spritedef, UINT16 wadnum, UINT16 startlump, UINT16 endlump)
{
	UINT16 l;
	UINT8 frame;
	UINT8 rotation;
	lumpinfo_t *lumpinfo;
	patch_t patch;

	memset(sprtemp,0xFF, sizeof (sprtemp));
	maxframe = (size_t)-1;

	// are we 'patching' a sprite already loaded ?
	// if so, it might patch only certain frames, not all
	if (spritedef->numframes) // (then spriteframes is not null)
	{
		// copy the already defined sprite frames
		M_Memcpy(sprtemp, spritedef->spriteframes,
		 spritedef->numframes * sizeof (spriteframe_t));
		maxframe = spritedef->numframes - 1;
	}

	// scan the lumps,
	//  filling in the frames for whatever is found
	lumpinfo = wadfiles[wadnum]->lumpinfo;
	if (endlump > wadfiles[wadnum]->numlumps)
		endlump = wadfiles[wadnum]->numlumps;

	for (l = startlump; l < endlump; l++)
	{
		if (memcmp(lumpinfo[l].name,sprname,4)==0)
		{
			frame = (UINT8)(lumpinfo[l].name[4] - 'A');
			rotation = (UINT8)(lumpinfo[l].name[5] - '0');

			if (frame >= 64 || rotation > 8) // Give an actual NAME error -_-...
			{
				CONS_Printf("WARNING! Bad sprite name: %s", W_CheckNameForNumPwad(wadnum,l));
				continue;
			}

			// skip NULL sprites from very old dmadds pwads
			if (W_LumpLengthPwad(wadnum,l)<=8)
				continue;

			// store sprite info in lookup tables
			//FIXME : numspritelumps do not duplicate sprite replacements
			W_ReadLumpHeaderPwad(wadnum, l, &patch, sizeof (patch_t), 0);
			spritecachedinfo[numspritelumps].width = SHORT(patch.width)<<FRACBITS;
			spritecachedinfo[numspritelumps].offset = SHORT(patch.leftoffset)<<FRACBITS;
			spritecachedinfo[numspritelumps].topoffset = SHORT(patch.topoffset)<<FRACBITS;
			spritecachedinfo[numspritelumps].height = SHORT(patch.height)<<FRACBITS;

#ifdef HWRENDER
			//BP: we cannot use special tric in hardware mode because feet in ground caused by z-buffer
			if (rendermode != render_soft && rendermode != render_none // not for psprite
			 && SHORT(patch.topoffset)>0 && SHORT(patch.topoffset)<SHORT(patch.height))
				// perfect is patch.height but sometime it is too high
				spritecachedinfo[numspritelumps].topoffset = min(SHORT(patch.topoffset)+4,SHORT(patch.height))<<FRACBITS;
#endif

			//----------------------------------------------------

			R_InstallSpriteLump(wadnum, l, numspritelumps, frame, rotation, 0);

			if (lumpinfo[l].name[6])
			{
				frame = (UINT8)(lumpinfo[l].name[6] - 'A');
				rotation = (UINT8)(lumpinfo[l].name[7] - '0');
				R_InstallSpriteLump(wadnum, l, numspritelumps, frame, rotation, 1);
			}

			if (++numspritelumps >= MAXSPRITELUMPS)
				I_Error("R_AddSingleSpriteDef: too much sprite replacements (numspritelumps)\n");
		}
	}

	//
	// if no frames found for this sprite
	//
	if (maxframe == (size_t)-1)
	{
		// the first time (which is for the original wad),
		// all sprites should have their initial frames
		// and then, patch wads can replace it
		// we will skip non-replaced sprite frames, only if
		// they have already have been initially defined (original wad)

		//check only after all initial pwads added
		//if (spritedef->numframes == 0)
		//    I_Error("R_AddSpriteDefs: no initial frames found for sprite %s\n",
		//             namelist[i]);

		// sprite already has frames, and is not replaced by this wad
		return false;
	}

	maxframe++;

	//
	//  some checks to help development
	//
	for (frame = 0; frame < maxframe; frame++)
	{
		switch (sprtemp[frame].rotate)
		{
			case 0xff:
			// no rotations were found for that frame at all
			I_Error("R_AddSingleSpriteDef: No patches found "
			        "for %s frame %c", sprname, frame+'A');
			break;

			case 0:
			// only the first rotation is needed
			break;

			case 1:
			// must have all 8 frames
			for (rotation = 0; rotation < 8; rotation++)
				// we test the patch lump, or the id lump whatever
				// if it was not loaded the two are LUMPERROR
				if (sprtemp[frame].lumppat[rotation] == LUMPERROR)
					I_Error("R_AddSingleSpriteDef: Sprite %s frame %c "
					        "is missing rotations",
					        sprname, frame+'A');
			break;
		}
	}

	// allocate space for the frames present and copy sprtemp to it
	if (spritedef->numframes &&             // has been allocated
		spritedef->numframes < maxframe)   // more frames are defined ?
	{
		Z_Free(spritedef->spriteframes);
		spritedef->spriteframes = NULL;
	}

	// allocate this sprite's frames
	if (!spritedef->spriteframes)
		spritedef->spriteframes =
		 Z_Malloc(maxframe * sizeof (*spritedef->spriteframes), PU_STATIC, NULL);

	spritedef->numframes = maxframe;
	M_Memcpy(spritedef->spriteframes, sprtemp, maxframe*sizeof (spriteframe_t));

	return true;
}

static boolean R_DelSingleSpriteDef(const char *sprname, spritedef_t *spritedef, UINT16 wadnum, UINT16 startlump, UINT16 endlump)
{
	UINT16 l;
	UINT8 frame;
	UINT8 rotation;
	lumpinfo_t *lumpinfo;

	maxframe = (size_t)-1;

	// scan the lumps,
	//  filling in the frames for whatever is found
	lumpinfo = wadfiles[wadnum]->lumpinfo;
	if (endlump > wadfiles[wadnum]->numlumps)
		endlump = wadfiles[wadnum]->numlumps;

	for (l = startlump; l < endlump; l++)
	{
		if (memcmp(lumpinfo[l].name,sprname,4)==0)
		{
			frame = (UINT8)(lumpinfo[l].name[4] - 'A');
			rotation = (UINT8)(lumpinfo[l].name[5] - '0');

			// skip NULL sprites from very old dmadds pwads
			if (W_LumpLengthPwad(wadnum,l)<=8)
				continue;

			//----------------------------------------------------

			R_RemoveSpriteLump(wadnum, l, numspritelumps, frame, rotation, 0);

			if (lumpinfo[l].name[6])
			{
				frame = (UINT8)(lumpinfo[l].name[6] - 'A');
				rotation = (UINT8)(lumpinfo[l].name[7] - '0');
				R_RemoveSpriteLump(wadnum, l, numspritelumps, frame, rotation, 1);
			}
		}
	}

	if (maxframe == (size_t)-1)
		return false;

	spritedef->numframes = 0;
	Z_Free(spritedef->spriteframes);
	spritedef->spriteframes = NULL;
	return true;
}

//
// Search for sprites replacements in a wad whose names are in namelist
//
void R_AddSpriteDefs(UINT16 wadnum)
{
	size_t i, addsprites = 0;
	UINT16 start, end;

	// find the sprites section in this pwad
	// we need at least the S_END
	// (not really, but for speedup)

	start = W_CheckNumForNamePwad("S_START", wadnum, 0);
	if (start == INT16_MAX)
		start = W_CheckNumForNamePwad("SS_START", wadnum, 0); //deutex compatib.
	if (start == INT16_MAX)
		start = 0; //let say S_START is lump 0
	else
		start++;   // just after S_START

	end = W_CheckNumForNamePwad("S_END",wadnum,start);
	if (end == INT16_MAX)
		end = W_CheckNumForNamePwad("SS_END",wadnum,start);     //deutex compatib.
	if (end == INT16_MAX)
	{
		if (devparm)
			CONS_Printf("no sprites in pwad %d\n", wadnum);
		return;
		//I_Error("R_AddSpriteDefs: S_END, or SS_END missing for sprites "
		//         "in pwad %d\n",wadnum);
	}

	//
	// scan through lumps, for each sprite, find all the sprite frames
	//
	for (i = 0; i < numsprites; i++)
	{
		spritename = sprnames[i];

		if (R_AddSingleSpriteDef(spritename, &sprites[i], wadnum, start, end))
		{
			// if a new sprite was added (not just replaced)
			addsprites++;
			if (devparm)
				I_OutputMsg("sprite %s set in pwad %d\n", spritename, wadnum);//Fab
		}
	}

	CONS_Printf("%"PRIdS" sprites added from file %s\n", addsprites, wadfiles[wadnum]->filename);
}

void R_DelSpriteDefs(UINT16 wadnum)
{
	size_t i, delsprites = 0;
	UINT16 start, end;

	// find the sprites section in this pwad
	// we need at least the S_END
	// (not really, but for speedup)

	start = W_CheckNumForNamePwad("S_START", wadnum, 0);
	if (start == INT16_MAX)
		start = W_CheckNumForNamePwad("SS_START", wadnum, 0); //deutex compatib.
	if (start == INT16_MAX)
		start = 0; //let say S_START is lump 0
	else
		start++;   // just after S_START

	end = W_CheckNumForNamePwad("S_END",wadnum,start);
	if (end == INT16_MAX)
		end = W_CheckNumForNamePwad("SS_END",wadnum,start);     //deutex compatib.
	if (end == INT16_MAX)
	{
		if (devparm)
			CONS_Printf("no sprites in pwad %d\n", wadnum);
		return;
		//I_Error("R_DelSpriteDefs: S_END, or SS_END missing for sprites "
		//         "in pwad %d\n",wadnum);
	}

	//
	// scan through lumps, for each sprite, find all the sprite frames
	//
	for (i = 0; i < numsprites; i++)
	{
		spritename = sprnames[i];

		if (R_DelSingleSpriteDef(spritename, &sprites[i], wadnum, start, end))
		{
			// if a new sprite was removed (not just replaced)
			delsprites++;
			if (devparm)
				I_OutputMsg("sprite %s set in pwad %d\n", spritename, wadnum);//Fab
		}
	}

	CONS_Printf("%"PRIdS" sprites removed from file %s\n", delsprites, wadfiles[wadnum]->filename);
}

//
// GAME FUNCTIONS
//
static vissprite_t vissprites[MAXVISSPRITES];
static vissprite_t *vissprite_p;

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(void)
{
	size_t i;

	for (i = 0; i < MAXVIDWIDTH; i++)
	{
		negonearray[i] = -1;
	}

	//
	// count the number of sprite names, and allocate sprites table
	//
	numsprites = 0;
	for (i = 0; i < NUMSPRITES + 1; i++)
		if (sprnames[i][0] != '\0') numsprites++;

	if (!numsprites)
		I_Error("R_AddSpriteDefs: no sprites in namelist\n");

	sprites = Z_Calloc(numsprites * sizeof (*sprites), PU_STATIC, NULL);

	// find sprites in each -file added pwad
	for (i = 0; i < numwadfiles; i++)
		R_AddSpriteDefs((UINT16)i);

	//
	// now check for skins
	//

	// it can be is do before loading config for skin cvar possible value
	R_InitSkins();
	for (i = 0; i < numwadfiles; i++)
		R_AddSkins((UINT16)i);

	//
	// check if all sprites have frames
	//
	/*
	for (i = 0; i < numsprites; i++)
		if (sprites[i].numframes < 1)
			CONS_Printf("R_InitSprites: sprite %s has no frames at all\n", sprnames[i]);
	*/
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
	vissprite_p = vissprites;
}

//
// R_NewVisSprite
//
static vissprite_t overflowsprite;

static vissprite_t *R_NewVisSprite(void)
{
	if (vissprite_p == &vissprites[MAXVISSPRITES])
		return &overflowsprite;

	vissprite_p++;
	return vissprite_p-1;
}

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
INT16 *mfloorclip;
INT16 *mceilingclip;

fixed_t spryscale = 0, sprtopscreen = 0, sprbotscreen = 0;
fixed_t windowtop = 0, windowbottom = 0;

void R_DrawMaskedColumn(column_t *column)
{
	INT32 topscreen;
	INT32 bottomscreen;
	fixed_t basetexturemid;

	basetexturemid = dc_texturemid;

	for (; column->topdelta != 0xff ;)
	{
		// calculate unclipped screen coordinates
		// for post
		topscreen = sprtopscreen + spryscale*column->topdelta;
		bottomscreen = sprbotscreen == INT32_MAX ? topscreen + spryscale*column->length
		                                      : sprbotscreen + spryscale*column->length;

		dc_yl = (topscreen+FRACUNIT-1)>>FRACBITS;
		dc_yh = (bottomscreen-1)>>FRACBITS;

		if (windowtop != INT32_MAX && windowbottom != INT32_MAX)
		{
			if (windowtop > topscreen)
				dc_yl = (windowtop + FRACUNIT - 1)>>FRACBITS;
			if (windowbottom < bottomscreen)
				dc_yh = (windowbottom - 1)>>FRACBITS;
		}

		if (dc_yh >= mfloorclip[dc_x])
			dc_yh = mfloorclip[dc_x]-1;
		if (dc_yl <= mceilingclip[dc_x])
			dc_yl = mceilingclip[dc_x]+1;
		if (dc_yl < 0)
			dc_yl = 0;
		if (dc_yh >= vid.height)
			dc_yh = vid.height - 1;

		if (dc_yl <= dc_yh && dc_yl < vid.height && dc_yh > 0)
		{
			dc_source = (UINT8 *)column + 3;
			dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);

			// Drawn by R_DrawColumn.
			// This stuff is a likely cause of the splitscreen water crash bug.
			// FIXTHIS: Figure out what "something more proper" is and do it.
			// quick fix... something more proper should be done!!!
			if (ylookup[dc_yl])
				colfunc();
			else if (colfunc == R_DrawColumn_8
#ifdef USEASM
			|| colfunc == R_DrawColumn_8_ASM || colfunc == R_DrawColumn_8_Pentium
			|| colfunc == R_DrawColumn_8_NOMMX || colfunc == R_DrawColumn_8_K6_MMX
#endif
			)
			{
				static INT32 first = 1;
				if (first)
				{
					CONS_Printf("WARNING: avoiding a crash in %s %d\n", __FILE__, __LINE__);
					first = 0;
				}
			}
		}
		column = (column_t *)((UINT8 *)column + column->length + 4);
	}

	dc_texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
static void R_DrawVisSprite(vissprite_t *vis)
{
	column_t *column;
#ifdef RANGECHECK
	INT32 texturecolumn;
#endif
	fixed_t frac;
	patch_t *patch;

	// flip it in memory here
	if (vis->vflip)
	{
		INT32 x, count;
		UINT8 *source, *dest;
		column_t *destcol;
		patch_t *oldpatch;

		oldpatch = W_CacheLumpNum(vis->patch, PU_STATIC);
		patch = W_CacheLumpNumForce(vis->patch, PU_STATIC);

		if (!(oldpatch && patch))
			return;

		for (x = 0; x < SHORT(oldpatch->width); x++)
		{
			column = (column_t *)((UINT8 *)oldpatch + LONG(oldpatch->columnofs[x]));
			destcol = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[x]));

			while (column->topdelta != 0xff)
			{
				destcol->topdelta = (UINT8)(SHORT(oldpatch->height)-column->length-column->topdelta); //flippy

				source = (UINT8 *)column + 3;
				dest = (UINT8 *)destcol + 3;
				count = column->length;

				while (count--)
					source++;

				for (; count < column->length; count++)
				{
					source--;
					*dest++ = *source;
				}

				column = (column_t *)((UINT8 *)column + column->length + 4);
				destcol = (column_t *)((UINT8 *)destcol + destcol->length + 4);
			}
		}

		// Only free oldpatch if it wasn't cached before!
		// Otherwise we may later attempt to draw patches that don't exist!
		if (!(W_IsLumpCached(vis->patch, oldpatch)))
			Z_Free(oldpatch);
	}
	else
	{
		patch = W_CacheLumpNum(vis->patch, PU_CACHE);

		if (!patch)
			return;
	}

	colfunc = basecolfunc; // hack: this isn't resetting properly somewhere.
	dc_colormap = vis->colormap;
	if ((vis->mobjflags & MF_TRANSLATION) && vis->transmap) // Color mapping
	{
		colfunc = transtransfunc;
		dc_transmap = vis->transmap;
		dc_translation = defaulttranslationtables - 256 + ((INT32)vis->mobj->color<<8);
	}
	else if (vis->transmap)
	{
		colfunc = fuzzcolfunc;
		dc_transmap = vis->transmap;    //Fab : 29-04-98: translucency table
	}
	else if (vis->mobjflags & MF_TRANSLATION)
	{
		// translate green skin to another color
		colfunc = transcolfunc;

		// New colormap stuff for skins Tails 06-07-2002
#ifdef TRANSFIX
		if (vis->mobj->skin) // This thing is a player!
			dc_translation = translationtables[(skin_t*)vis->mobj->skin-skins] - 256 +
				((INT32)vis->mobj->color<<8);
#else
		if (vis->mobj->player) // This thing is a player!
		{
			if (vis->mobj->player->skincolor)
				dc_translation = translationtables[vis->mobj->player->skin] - 256 + ((INT32)vis->mobj->color<<8);
			else
			{
				static INT32 firsttime = 1;
				colfunc = basecolfunc; // Graue 04-08-2004
				if (firsttime)
				{
					CONS_Printf("Abandoning!\n");
					firsttime = 0;
				}
			}
		}
#endif
		else if ((vis->mobj->flags & MF_BOSS) && (vis->mobj->flags2 & MF2_FRET) && (leveltime & 1)) // Bosses "flash"
		{
			dc_translation = bosstranslationtables;
		}
		else // Use the defaults
			dc_translation = defaulttranslationtables - 256 + ((INT32)vis->mobj->color<<8);
	}

	if (vis->extra_colormap)
	{
		if (!dc_colormap)
			dc_colormap = vis->extra_colormap->colormap;
		else
			dc_colormap = &vis->extra_colormap->colormap[dc_colormap - colormaps];
	}
	if (!dc_colormap)
		dc_colormap = colormaps;

	dc_iscale = FixedDiv(FRACUNIT, vis->scale);
	dc_texturemid = vis->texturemid;
	dc_texheight = 0;

	frac = vis->startfrac;
	spryscale = vis->scale;
	sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
	windowtop = windowbottom = sprbotscreen = INT32_MAX;
	if (vis->mobjflags & MF_HIRES)
	{
		spryscale >>= 1;
		vis->scale >>= 1;
		dc_iscale <<= 1;
		vis->xiscale <<= 1;
		dc_hires = 1;
	}
	else if (vis->mobj->scale >= 400)
	{ // Scale > 400%? Software can't handle that! Render it as 400% instead.
		spryscale *= 4;
		vis->scale *= 4;
		dc_iscale /= 4;
		vis->xiscale /= 4;

		//Oh lordy.  (Fixing scaled sprites messing up if only partway on screen)
		if (vis->xiscale > 0)
			frac /= 4;
		else if (vis->x1 <= 0)
			frac = (vis->x1 - vis->x2) * vis->xiscale;

		dc_hires = 1;
	}
	else if (vis->mobj->scale != 100)
	{
		spryscale = spryscale*vis->mobj->scale/100;
		vis->scale = vis->scale*vis->mobj->scale/100;
		dc_iscale = dc_iscale*100/vis->mobj->scale;
		vis->xiscale = vis->xiscale*100/vis->mobj->scale;

		//Oh lordy, again.  See above.
		if (vis->xiscale > 0)
			frac = frac*100/vis->mobj->scale;
		else if (vis->x1 <= 0)
			frac = (vis->x1 - vis->x2) * vis->xiscale;

		dc_hires = 1;
	}

	if (vis->x1 < 0)
		vis->x1 = 0;

	if (vis->x2 >= vid.width)
		vis->x2 = vid.width-1;

	for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
	{
#ifdef RANGECHECK
		texturecolumn = frac>>FRACBITS;

		if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
			I_Error("R_DrawSpriteRange: bad texturecolumn");
		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[texturecolumn]));
#else
		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[frac>>FRACBITS]));
#endif
		R_DrawMaskedColumn(column);
	}

	colfunc = basecolfunc;
	if (vis->mobjflags & MF_HIRES)
	{
		spryscale <<= 1;
		vis->scale <<= 1;
		dc_iscale >>= 1;
		vis->xiscale >>= 1;
	}
	dc_hires = 0;

	if (vis->vflip)
		Z_Free(patch);
}

// Special precipitation drawer Tails 08-18-2002
static void R_DrawPrecipitationVisSprite(vissprite_t *vis)
{
	column_t *column;
#ifdef RANGECHECK
	INT32 texturecolumn;
#endif
	fixed_t frac;
	patch_t *patch;

	//Fab : R_InitSprites now sets a wad lump number
	patch = W_CacheLumpNum(vis->patch, PU_CACHE);
	if (!patch)
		return;

	if (vis->transmap)
	{
		colfunc = fuzzcolfunc;
		dc_transmap = vis->transmap;    //Fab : 29-04-98: translucency table
	}

	dc_colormap = colormaps;

	dc_iscale = FixedDiv(FRACUNIT, vis->scale);
	dc_texturemid = vis->texturemid;
	dc_texheight = 0;

	frac = vis->startfrac;
	spryscale = vis->scale;
	sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);
	windowtop = windowbottom = sprbotscreen = INT32_MAX;

	if (vis->x1 < 0)
		vis->x1 = 0;

	if (vis->x2 >= vid.width)
		vis->x2 = vid.width-1;

	for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
	{
#ifdef RANGECHECK
		texturecolumn = frac>>FRACBITS;

		if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
			I_Error("R_DrawPrecipitationSpriteRange: bad texturecolumn");

		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[texturecolumn]));
#else
		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[frac>>FRACBITS]));
#endif
		R_DrawMaskedColumn(column);
	}

	colfunc = basecolfunc;
}

//
// R_SplitSprite
// runs through a sector's lightlist and
static void R_SplitSprite(vissprite_t *sprite, mobj_t *thing)
{
	INT32 i, lightnum, lindex;
	INT16 cutfrac;
	sector_t *sector;
	vissprite_t *newsprite;

	sector = sprite->sector;

	for (i = 1; i < sector->numlights; i++)
	{
		if (sector->lightlist[i].height >= sprite->gzt || !(sector->lightlist[i].caster->flags & FF_CUTSPRITES))
			continue;
		if (sector->lightlist[i].height <= sprite->gz)
			return;

		cutfrac = (INT16)((centeryfrac - FixedMul(sector->lightlist[i].height - viewz, sprite->scale))>>FRACBITS);
		if (cutfrac < 0)
			continue;
		if (cutfrac > vid.height)
			return;

		// Found a split! Make a new sprite, copy the old sprite to it, and
		// adjust the heights.
		newsprite = M_Memcpy(R_NewVisSprite(), sprite, sizeof (vissprite_t));

		sprite->cut |= SC_BOTTOM;
		sprite->gz = sector->lightlist[i].height;

		newsprite->gzt = sprite->gz;

		sprite->sz = cutfrac;
		newsprite->szt = (INT16)(sprite->sz - 1);

		if (sector->lightlist[i].height < sprite->pzt && sector->lightlist[i].height > sprite->pz)
			sprite->pz = newsprite->pzt = sector->lightlist[i].height;
		else
		{
			newsprite->pz = newsprite->gz;
			newsprite->pzt = newsprite->gzt;
		}

		newsprite->szt -= 8;

		newsprite->cut |= SC_TOP;
		if (!(sector->lightlist[i].caster->flags & FF_NOSHADE))
		{
			lightnum = (*sector->lightlist[i].lightlevel >> LIGHTSEGSHIFT);

			if (lightnum < 0)
				spritelights = scalelight[0];
			else if (lightnum >= LIGHTLEVELS)
				spritelights = scalelight[LIGHTLEVELS-1];
			else
				spritelights = scalelight[lightnum];

			newsprite->extra_colormap = sector->lightlist[i].extra_colormap;

/*
			if (thing->frame & FF_TRANSMASK)
				;
			else if (thing->flags2 & MF2_SHADOW)
				;
			else
*/
			if (!((thing->frame & (FF_FULLBRIGHT|FF_TRANSMASK) || thing->flags2 & MF2_SHADOW)
				&& (!newsprite->extra_colormap || !newsprite->extra_colormap->fog)))
			{
				lindex = sprite->xscale>>(LIGHTSCALESHIFT);

				if (lindex >= MAXLIGHTSCALE)
					lindex = MAXLIGHTSCALE-1;
				newsprite->colormap = spritelights[lindex];
			}
		}
		sprite = newsprite;
	}
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
// if it might be visible.
//
static void R_ProjectSprite(mobj_t *thing)
{
	fixed_t tr_x, tr_y;
	fixed_t gxt, gyt;
	fixed_t tx, tz;
	fixed_t xscale, yscale; //added : 02-02-98 : aaargll..if I were a math-guy!!!

	INT32 x1, x2;

	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	size_t lump;

	size_t rot;
	UINT8 flip;

	INT32 lindex;

	vissprite_t *vis;

	angle_t ang;
	fixed_t iscale;

	//SoM: 3/17/2000
	fixed_t gz, gzt;
	INT32 heightsec;
	INT32 light = 0;

	// transform the origin point
	tr_x = thing->x - viewx;
	tr_y = thing->y - viewy;

	gxt = FixedMul(tr_x, viewcos);
	gyt = -FixedMul(tr_y, viewsin);

	tz = gxt-gyt;

	// thing is behind view plane?
	if (tz < MINZ)
		return;

	gxt = -FixedMul(tr_x, viewsin);
	gyt = FixedMul(tr_y, viewcos);
	tx = -(gyt + gxt);

	// too far off the side?
	if (abs(tx) > tz<<2)
		return;

	// aspect ratio stuff
	xscale = FixedDiv(projection, tz);
	yscale = FixedDiv(projectiony, tz);

	// decide which patch to use for sprite relative to player
	if ((size_t)(thing->sprite) >= numsprites)
#ifdef RANGECHECK
		I_Error("R_ProjectSprite: invalid sprite number %d ", thing->sprite);
#else
	{
		CONS_Printf("Warning: Mobj of type %d with invalid sprite data (%d) detected and removed.\n", thing->type, thing->sprite);
		if (thing->player)
		{
			P_SetPlayerMobjState(thing, S_PLAY_STND);
		}
		else
		{
			P_SetMobjState(thing, S_DISS);
		}
		return;
	}
#endif

	rot = thing->frame&FF_FRAMEMASK;

	//Fab : 02-08-98: 'skin' override spritedef currently used for skin
	if (thing->skin)
		sprdef = &((skin_t *)thing->skin)->spritedef;
	else
		sprdef = &sprites[thing->sprite];

	if (rot >= sprdef->numframes)
#ifdef RANGECHECK
		I_Error("R_ProjectSprite: invalid sprite frame %u : %"PRIdS"/%"PRIdS" for %s",
		 thing->sprite, rot, sprdef->numframes, sprnames[thing->sprite]);
#else
	{
		CONS_Printf("Warning: Mobj of type %d with invalid sprite frame (%"PRIdS"/%"PRIdS") of %s detected and removed.\n", thing->type, rot, sprdef->numframes, sprnames[thing->sprite]);
		if (thing->player)
		{
			P_SetPlayerMobjState(thing, S_PLAY_STND);
		}
		else
		{
			P_SetMobjState(thing, S_DISS);
		}
		return;
	}
#endif

	sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

	if (!sprframe)
#ifdef PARANOIA //heretic hack
		I_Error("sprframes NULL for sprite %d\n", thing->sprite);
#else
		return;
#endif

	if (sprframe->rotate)
	{
		// choose a different rotation based on player view
		ang = R_PointToAngle (thing->x, thing->y);
		rot = (ang-thing->angle+ANGLE_202h)>>29;
		//Fab: lumpid is the index for spritewidth,spriteoffset... tables
		lump = sprframe->lumpid[rot];
		flip = sprframe->flip[rot];
	}
	else
	{
		// use single rotation for all views
		rot = 0;                        //Fab: for vis->patch below
		lump = sprframe->lumpid[0];     //Fab: see note above
		flip = sprframe->flip[0];
	}

	I_Assert(lump < MAXSPRITELUMPS);

	// calculate edges of the shape
	if (thing->scale > 400)
		tx -= FIXEDSCALE(spritecachedinfo[lump].offset,400);
	else if (thing->scale != 100)
		tx -= FIXEDSCALE(spritecachedinfo[lump].offset,thing->scale);
	else if (thing->flags & MF_HIRES)
		tx -= spritecachedinfo[lump].offset/2;
	else
		tx -= spritecachedinfo[lump].offset;
	x1 = (centerxfrac + FixedMul (tx,xscale)) >>FRACBITS;

	// off the right side?
	if (x1 > viewwidth)
		return;

	if (thing->scale > 400)
		tx += FIXEDSCALE(spritecachedinfo[lump].width,400);
	else if (thing->scale != 100)
		tx += FIXEDSCALE(spritecachedinfo[lump].width,thing->scale);
	else if (thing->flags & MF_HIRES)
		tx += spritecachedinfo[lump].width/2;
	else
		tx += spritecachedinfo[lump].width;
	x2 = ((centerxfrac + FixedMul (tx,xscale)) >>FRACBITS) - 1;

	// off the left side
	if (x2 < 0)
		return;

	//SoM: 3/17/2000: Disregard sprites that are out of view..
	if (thing->eflags & MFE_VERTICALFLIP)
	{
		if (thing->scale > 400)
		{
			gzt = thing->z + thing->height + FIXEDSCALE(spritecachedinfo[lump].height - spritecachedinfo[lump].topoffset, 400);
			gz = gzt - FIXEDSCALE(spritecachedinfo[lump].height, 400);
		}
		else if (thing->scale != 100)
		{
			gzt = thing->z + thing->height + FIXEDSCALE(spritecachedinfo[lump].height - spritecachedinfo[lump].topoffset, thing->scale);
			gz = gzt - FIXEDSCALE(spritecachedinfo[lump].height,thing->scale);
		}
		else if (thing->flags & MF_HIRES)
		{
			gzt = thing->z + thing->height + (spritecachedinfo[lump].height - spritecachedinfo[lump].topoffset)/2;
			gz = gzt - spritecachedinfo[lump].height/2;
		}
		else
		{
			// When vertical flipped, draw sprites from the top down, at least as far as offsets are concerned.
			// Visual errors occur from "thing->height" being inexact otherwise, so you have to use it. Duh.
			// sprite height - sprite topoffset is the proper inverse of the vertical offset, of course.
			gzt = thing->z + thing->height + spritecachedinfo[lump].height - spritecachedinfo[lump].topoffset;
			gz = gzt - spritecachedinfo[lump].height;
		}
	}
	else
	{
		if (thing->scale > 400)
		{
			gzt = thing->z + FIXEDSCALE(spritecachedinfo[lump].topoffset,400);
			gz = gzt - FIXEDSCALE(spritecachedinfo[lump].height,400);
		}
		else if (thing->scale != 100)
		{
			gzt = thing->z + FIXEDSCALE(spritecachedinfo[lump].topoffset,thing->scale);
			gz = gzt - FIXEDSCALE(spritecachedinfo[lump].height,thing->scale);
		}
		else if (thing->flags & MF_HIRES)
		{
			gzt = thing->z + spritecachedinfo[lump].topoffset/2;
			gz = gzt - spritecachedinfo[lump].height/2;
		}
		else
		{
			gzt = thing->z + spritecachedinfo[lump].topoffset;
			gz = gzt - spritecachedinfo[lump].height;
		}
	}

	if (thing->subsector->sector->cullheight)
	{
		if (thing->subsector->sector->cullheight->flags & ML_NOCLIMB) // Group culling
		{
			// Make sure this is part of the same group
			if (viewsector->cullheight && viewsector->cullheight->frontsector
				== thing->subsector->sector->cullheight->frontsector)
			{
				// OK, we can cull
				if (viewz > thing->subsector->sector->cullheight->frontsector->floorheight
					&& gzt < thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if below plane
					return;

				if (gz > thing->subsector->sector->cullheight->frontsector->floorheight
					&& viewz <= thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if above plane
					return;
			}
		}
		else // Quick culling
		{
			if (viewz > thing->subsector->sector->cullheight->frontsector->floorheight
				&& gzt < thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if below plane
				return;

			if (gz > thing->subsector->sector->cullheight->frontsector->floorheight
				&& viewz <= thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if above plane
				return;
		}
	}

	if (thing->subsector->sector->numlights)
	{
		INT32 lightnum;
		light = R_GetPlaneLight(thing->subsector->sector, gzt, false);
		lightnum = (*thing->subsector->sector->lightlist[light].lightlevel >> LIGHTSEGSHIFT);

		if (lightnum < 0)
			spritelights = scalelight[0];
		else if (lightnum >= LIGHTLEVELS)
			spritelights = scalelight[LIGHTLEVELS-1];
		else
			spritelights = scalelight[lightnum];
	}

	heightsec = thing->subsector->sector->heightsec;

	if (heightsec != -1)   // only clip things which are in special sectors
	{
		INT32 phs = viewplayer->mo->subsector->sector->heightsec;
		if (phs != -1 && viewz < sectors[phs].floorheight ?
		    thing->z >= sectors[heightsec].floorheight :
		    gzt < sectors[heightsec].floorheight)
			return;
		if (phs != -1 && viewz > sectors[phs].ceilingheight ?
		     gzt < sectors[heightsec].ceilingheight &&
		     viewz >= sectors[heightsec].ceilingheight :
		     thing->z >= sectors[heightsec].ceilingheight)
			return;
	}

	// store information in a vissprite
	vis = R_NewVisSprite();
	vis->heightsec = heightsec; //SoM: 3/17/2000
	vis->mobjflags = thing->flags;
	vis->scale = yscale + thing->info->dispoffset;           //<<detailshift;
	vis->gx = thing->x;
	vis->gy = thing->y;
	vis->gz = gz;
	vis->gzt = gzt;
	vis->thingheight = thing->height;
	vis->pz = thing->z;
	vis->pzt = vis->pz + vis->thingheight;
	vis->texturemid = vis->gzt - viewz;

	vis->mobj = thing; // Easy access! Tails 06-07-2002

	vis->x1 = x1 < 0 ? 0 : x1;
	vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	vis->xscale = xscale; //SoM: 4/17/2000
	vis->sector = thing->subsector->sector;
	vis->szt = (INT16)((centeryfrac - FixedMul(vis->gzt - viewz, yscale))>>FRACBITS);
	vis->sz = (INT16)((centeryfrac - FixedMul(vis->gz - viewz, yscale))>>FRACBITS);
	vis->cut = false;
	if (thing->subsector->sector->numlights)
		vis->extra_colormap = thing->subsector->sector->lightlist[light].extra_colormap;
	else
		vis->extra_colormap = thing->subsector->sector->extra_colormap;

	iscale = FixedDiv(FRACUNIT, xscale);

	if (flip)
	{
		vis->startfrac = spritecachedinfo[lump].width-1;
		vis->xiscale = -iscale;
	}
	else
	{
		vis->startfrac = 0;
		vis->xiscale = iscale;
	}

	if (vis->x1 > x1)
		vis->startfrac += vis->xiscale*(vis->x1-x1);

	//Fab: lumppat is the lump number of the patch to use, this is different
	//     than lumpid for sprites-in-pwad : the graphics are patched
	vis->patch = sprframe->lumppat[rot];

//
// determine the colormap (lightlevel & special effects)
//
	vis->transmap = NULL;

	// specific translucency
	if (thing->flags2 & MF2_SHADOW) // actually only the player should use this (temporary invisibility)
		vis->transmap = ((tr_trans80-1)<<FF_TRANSSHIFT) + transtables; // because now the translucency is set through FF_TRANSMASK
	else if (thing->frame & FF_TRANSMASK)
		vis->transmap = (thing->frame & FF_TRANSMASK) - 0x10000 + transtables;

	if (((thing->frame & (FF_FULLBRIGHT|FF_TRANSMASK)) || (thing->flags2 & MF2_SHADOW))
		&& (!vis->extra_colormap || !vis->extra_colormap->fog))
	{
		// full bright: goggles
		vis->colormap = colormaps;
	}
	else
	{
		// diminished light
		lindex = xscale>>(LIGHTSCALESHIFT);

		if (lindex >= MAXLIGHTSCALE)
			lindex = MAXLIGHTSCALE-1;

		vis->colormap = spritelights[lindex];
	}

	vis->precip = false;

	if (thing->eflags & MFE_VERTICALFLIP)
		vis->vflip = true;
	else
		vis->vflip = false;

	if (thing->subsector->sector->numlights)
		R_SplitSprite(vis, thing);
}

static void R_ProjectPrecipitationSprite(precipmobj_t *thing)
{
	fixed_t tr_x, tr_y;
	fixed_t gxt, gyt;
	fixed_t tx, tz;
	fixed_t xscale, yscale; //added : 02-02-98 : aaargll..if I were a math-guy!!!

	INT32 x1, x2;

	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	size_t lump;

	vissprite_t *vis;

	fixed_t iscale;

	//SoM: 3/17/2000
	fixed_t gzt;

	// transform the origin point
	tr_x = thing->x - viewx;
	tr_y = thing->y - viewy;

	gxt = FixedMul(tr_x, viewcos);
	gyt = -FixedMul(tr_y, viewsin);

	tz = gxt - gyt;

	// thing is behind view plane?
	if (tz < MINZ)
		return;

	gxt = -FixedMul(tr_x, viewsin);
	gyt = FixedMul(tr_y, viewcos);
	tx = -(gyt + gxt);

	// too far off the side?
	if (abs(tx) > tz<<2)
		return;

	// aspect ratio stuff :
	xscale = FixedDiv(projection, tz);
	yscale = FixedDiv(projectiony, tz);

	// decide which patch to use for sprite relative to player
	if ((unsigned)thing->sprite >= numsprites)
#ifdef RANGECHECK
		I_Error("R_ProjectSprite: invalid sprite number %d ",
			thing->sprite);
#else
		return;
#endif

	sprdef = &sprites[thing->sprite];

	if ((UINT8)(thing->frame&FF_FRAMEMASK) >= sprdef->numframes)
#ifdef RANGECHECK
		I_Error("R_ProjectSprite: invalid sprite frame %d : %d for %s",
			thing->sprite, thing->frame, sprnames[thing->sprite]);
#else
		return;
#endif

	sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

	if (!sprframe)
#ifdef PARANOIA //heretic hack
		I_Error("sprframes NULL for sprite %d\n", thing->sprite);
#else
		return;
#endif

	// use single rotation for all views
	lump = sprframe->lumpid[0];     //Fab: see note above

	// calculate edges of the shape
	tx -= spritecachedinfo[lump].offset;
	x1 = (centerxfrac + FixedMul (tx,xscale)) >>FRACBITS;

	// off the right side?
	if (x1 > viewwidth)
		return;

	tx += spritecachedinfo[lump].width;
	x2 = ((centerxfrac + FixedMul (tx,xscale)) >>FRACBITS) - 1;

	// off the left side
	if (x2 < 0)
		return;

	//SoM: 3/17/2000: Disregard sprites that are out of view..
	gzt = thing->z + spritecachedinfo[lump].topoffset;

	if (thing->subsector->sector->cullheight)
	{
		if (thing->subsector->sector->cullheight->flags & ML_NOCLIMB) // Group culling
		{
			// Make sure this is part of the same group
			if (viewsector->cullheight && viewsector->cullheight->frontsector
				== thing->subsector->sector->cullheight->frontsector)
			{
				// OK, we can cull
				if (viewz > thing->subsector->sector->cullheight->frontsector->floorheight
					&& gzt < thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if below plane
					return;
				else if (gzt - spritecachedinfo[lump].height > thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if above plane
					return;
			}
		}
		else // Quick culling
		{
			if (viewz > thing->subsector->sector->cullheight->frontsector->floorheight
				&& gzt < thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if below plane
				return;
			else if (gzt - spritecachedinfo[lump].height > thing->subsector->sector->cullheight->frontsector->floorheight) // Cull if above plane
				return;
		}
	}

	// store information in a vissprite
	vis = R_NewVisSprite();
	vis->scale = yscale; //<<detailshift;
	vis->gx = thing->x;
	vis->gy = thing->y;
	vis->gz = gzt - spritecachedinfo[lump].height;
	vis->gzt = gzt;
	vis->thingheight = 4*FRACUNIT;
	vis->pz = thing->z;
	vis->pzt = vis->pz + vis->thingheight;
	vis->texturemid = vis->gzt - viewz;

	vis->x1 = x1 < 0 ? 0 : x1;
	vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	vis->xscale = xscale; //SoM: 4/17/2000
	vis->sector = thing->subsector->sector;
	vis->szt = (INT16)((centeryfrac - FixedMul(vis->gzt - viewz, yscale))>>FRACBITS);
	vis->sz = (INT16)((centeryfrac - FixedMul(vis->gz - viewz, yscale))>>FRACBITS);

	iscale = FixedDiv(FRACUNIT, xscale);

	vis->startfrac = 0;
	vis->xiscale = iscale;

	if (vis->x1 > x1)
		vis->startfrac += vis->xiscale*(vis->x1-x1);

	//Fab: lumppat is the lump number of the patch to use, this is different
	//     than lumpid for sprites-in-pwad : the graphics are patched
	vis->patch = sprframe->lumppat[0];

	// specific translucency
	if (thing->frame & FF_TRANSMASK)
		vis->transmap = (thing->frame & FF_TRANSMASK) - 0x10000 + transtables;
	else
		vis->transmap = NULL;

	vis->mobjflags = 0;
	vis->cut = false;
	vis->extra_colormap = thing->subsector->sector->extra_colormap;
	vis->heightsec = thing->subsector->sector->heightsec;

	// Fullbright
	vis->colormap = colormaps;
	vis->precip = true;
	vis->vflip = false;
}

// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites(sector_t *sec, INT32 lightlevel)
{
	mobj_t *thing;
	precipmobj_t *precipthing; // Tails 08-25-2002
	INT32 lightnum;
	fixed_t adx, ady, approx_dist;

	if (rendermode != render_soft)
		return;

	// BSP is traversed by subsector.
	// A sector might have been split into several
	//  subsectors during BSP building.
	// Thus we check whether its already added.
	if (sec->validcount == validcount)
		return;

	// Well, now it will be done.
	sec->validcount = validcount;

	if (!sec->numlights)
	{
		if (sec->heightsec == -1) lightlevel = sec->lightlevel;

		lightnum = (lightlevel >> LIGHTSEGSHIFT);

		if (lightnum < 0)
			spritelights = scalelight[0];
		else if (lightnum >= LIGHTLEVELS)
			spritelights = scalelight[LIGHTLEVELS-1];
		else
			spritelights = scalelight[lightnum];
	}

	// Handle all things in sector.

	// NiGHTS stages have a draw distance limit because of the
	// HUGE number of SPRiTES!
	if (maptol & TOL_NIGHTS && players[displayplayer].mo)
	{
		for (thing = sec->thinglist; thing; thing = thing->snext)
		{
			if (!thing)
				continue;

			if ((thing->flags2 & MF2_DONTDRAW)==0)
			{
				adx = abs(players[displayplayer].mo->x - thing->x);
				ady = abs(players[displayplayer].mo->y - thing->y);

				// From _GG1_ p.428. Approx. eucledian distance fast.
				approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

				if (approx_dist < LIMIT_DRAW_DIST)
					R_ProjectSprite(thing);
				else if (splitscreen && players[secondarydisplayplayer].mo)
				{
					adx = abs(players[secondarydisplayplayer].mo->x - thing->x);
					ady = abs(players[secondarydisplayplayer].mo->y - thing->y);

					// From _GG1_ p.428. Approx. eucledian distance fast.
					approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

					if (approx_dist < LIMIT_DRAW_DIST)
						R_ProjectSprite (thing);
				}
			}
		}
	}
	else
	{
		for (thing = sec->thinglist; thing; thing = thing->snext)
		{
			if (!thing)
				continue;

			if ((thing->flags2 & MF2_DONTDRAW)==0)
				R_ProjectSprite(thing);

			if (cv_objectplace.value
			&& !(thing->flags2 & MF2_DONTDRAW))
				objectsdrawn++;

			if (!thing->snext)
				break;
		}
	}

	// Special function for precipitation Tails 08-18-2002
	if (playeringame[displayplayer] && players[displayplayer].mo)
	{
		for (precipthing = sec->preciplist; precipthing; precipthing = precipthing->snext)
		{
			if (!precipthing)
				continue;

			if (precipthing->invisible)
				continue;

			adx = abs(players[displayplayer].mo->x - precipthing->x);
			ady = abs(players[displayplayer].mo->y - precipthing->y);

			// From _GG1_ p.428. Approx. eucledian distance fast.
			approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

			// Only draw the precipitation oh-so-far from the player.
			if (approx_dist < (cv_precipdist.value << FRACBITS))
				R_ProjectPrecipitationSprite(precipthing);
			else if (splitscreen && players[secondarydisplayplayer].mo)
			{
				adx = abs(players[secondarydisplayplayer].mo->x - precipthing->x);
				ady = abs(players[secondarydisplayplayer].mo->y - precipthing->y);

				// From _GG1_ p.428. Approx. eucledian distance fast.
				approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

				if (approx_dist < (cv_precipdist.value << FRACBITS))
					R_ProjectPrecipitationSprite (precipthing);
			}
		}
	}
}

//
// R_SortVisSprites
//
static vissprite_t vsprsortedhead;

void R_SortVisSprites(void)
{
	size_t       i, count;
	vissprite_t *ds;
	vissprite_t *best = NULL;
	vissprite_t  unsorted;
	fixed_t      bestscale;

	count = vissprite_p - vissprites;

	unsorted.next = unsorted.prev = &unsorted;

	if (!count)
		return;

	for (ds = vissprites; ds < vissprite_p; ds++)
	{
		ds->next = ds+1;
		ds->prev = ds-1;
	}

	vissprites[0].prev = &unsorted;
	unsorted.next = &vissprites[0];
	(vissprite_p-1)->next = &unsorted;
	unsorted.prev = vissprite_p-1;

	// pull the vissprites out by scale
	vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
	for (i = 0; i < count; i++)
	{
		bestscale = INT32_MAX;
		for (ds = unsorted.next; ds != &unsorted; ds = ds->next)
		{
			if (ds->scale < bestscale)
			{
				bestscale = ds->scale;
				best = ds;
			}
		}
		best->next->prev = best->prev;
		best->prev->next = best->next;
		best->next = &vsprsortedhead;
		best->prev = vsprsortedhead.prev;
		vsprsortedhead.prev->next = best;
		vsprsortedhead.prev = best;
	}
}

//
// R_CreateDrawNodes
// Creates and sorts a list of drawnodes for the scene being rendered.
static drawnode_t *R_CreateDrawNode(drawnode_t *link);

static drawnode_t nodebankhead;
static drawnode_t nodehead;

static void R_CreateDrawNodes(void)
{
	drawnode_t *entry;
	drawseg_t *ds;
	INT32 i, p, best, x1, x2;
	fixed_t bestdelta, delta;
	vissprite_t *rover;
	drawnode_t *r2;
	visplane_t *plane;
	INT32 sintersect;
	fixed_t gzm;
	fixed_t scale = 0;

	// Add the 3D floors, thicksides, and masked textures...
	for (ds = ds_p; ds-- > drawsegs ;)
	{
		if (ds->numthicksides)
		{
			for (i = 0; i < ds->numthicksides; i++)
			{
				entry = R_CreateDrawNode(&nodehead);
				entry->thickseg = ds;
				entry->ffloor = ds->thicksides[i];
			}
		}
		if (ds->maskedtexturecol)
		{
			entry = R_CreateDrawNode(&nodehead);
			entry->seg = ds;
		}
		if (ds->numffloorplanes)
		{
			for (i = 0; i < ds->numffloorplanes; i++)
			{
				best = -1;
				bestdelta = 0;
				for (p = 0; p < ds->numffloorplanes; p++)
				{
					if (!ds->ffloorplanes[p])
						continue;
					plane = ds->ffloorplanes[p];
					R_PlaneBounds(plane);

					if (plane->low < con_clipviewtop || plane->high > vid.height || plane->high > plane->low)
					{
						ds->ffloorplanes[p] = NULL;
						continue;
					}

					delta = abs(plane->height - viewz);
					if (delta > bestdelta)
					{
						best = p;
						bestdelta = delta;
					}
				}
				if (best != -1)
				{
					entry = R_CreateDrawNode(&nodehead);
					entry->plane = ds->ffloorplanes[best];
					entry->seg = ds;
					ds->ffloorplanes[best] = NULL;
				}
				else
					break;
			}
		}
	}

	if (vissprite_p == vissprites)
		return;

	R_SortVisSprites();
	for (rover = vsprsortedhead.prev; rover != &vsprsortedhead; rover = rover->prev)
	{
		if (rover->szt > vid.height || rover->sz < 0)
			continue;

		sintersect = (rover->x1 + rover->x2) / 2;
		gzm = (rover->gz + rover->gzt) / 2;

		for (r2 = nodehead.next; r2 != &nodehead; r2 = r2->next)
		{
			if (r2->plane)
			{
				if (r2->plane->minx > rover->x2 || r2->plane->maxx < rover->x1)
					continue;
				if (rover->szt > r2->plane->low || rover->sz < r2->plane->high)
					continue;

				if (rover->mobjflags & MF_NOCLIPHEIGHT)
				{
					//Objects with NOCLIPHEIGHT can appear halfway in.
					if (r2->plane->height < viewz && rover->pz+(rover->thingheight/2) >= r2->plane->height)
						continue;
					if (r2->plane->height > viewz && rover->pzt-(rover->thingheight/2) <= r2->plane->height)
						continue;
				}
				else
				{
					if (r2->plane->height < viewz && rover->pz >= r2->plane->height)
						continue;
					if (r2->plane->height > viewz && rover->pzt <= r2->plane->height)
						continue;
				}

				// SoM: NOTE: Because a visplane's shape and scale is not directly
				// bound to any single linedef, a simple poll of it's frontscale is
				// not adequate. We must check the entire frontscale array for any
				// part that is in front of the sprite.

				x1 = rover->x1;
				x2 = rover->x2;
				if (x1 < r2->plane->minx) x1 = r2->plane->minx;
				if (x2 > r2->plane->maxx) x2 = r2->plane->maxx;

				for (i = x1; i <= x2; i++)
				{
					if (r2->seg->frontscale[i] > rover->scale)
						break;
				}
				if (i > x2)
					continue;

				entry = R_CreateDrawNode(NULL);
				(entry->prev = r2->prev)->next = entry;
				(entry->next = r2)->prev = entry;
				entry->sprite = rover;
				break;
			}
			else if (r2->thickseg)
			{
				if (rover->x1 > r2->thickseg->x2 || rover->x2 < r2->thickseg->x1)
					continue;

				scale = r2->thickseg->scale1 > r2->thickseg->scale2 ? r2->thickseg->scale1 : r2->thickseg->scale2;
				if (scale <= rover->scale)
					continue;
				scale = r2->thickseg->scale1 + (r2->thickseg->scalestep * (sintersect - r2->thickseg->x1));
				if (scale <= rover->scale)
					continue;

				if ((*r2->ffloor->topheight > viewz && *r2->ffloor->bottomheight < viewz) ||
				    (*r2->ffloor->topheight < viewz && rover->gzt < *r2->ffloor->topheight) ||
				    (*r2->ffloor->bottomheight > viewz && rover->gz > *r2->ffloor->bottomheight))
				{
					entry = R_CreateDrawNode(NULL);
					(entry->prev = r2->prev)->next = entry;
					(entry->next = r2)->prev = entry;
					entry->sprite = rover;
					break;
				}
			}
			else if (r2->seg)
			{
				if (rover->x1 > r2->seg->x2 || rover->x2 < r2->seg->x1)
					continue;

				scale = r2->seg->scale1 > r2->seg->scale2 ? r2->seg->scale1 : r2->seg->scale2;
				if (scale <= rover->scale)
					continue;
				scale = r2->seg->scale1 + (r2->seg->scalestep * (sintersect - r2->seg->x1));

				if (rover->scale < scale)
				{
					entry = R_CreateDrawNode(NULL);
					(entry->prev = r2->prev)->next = entry;
					(entry->next = r2)->prev = entry;
					entry->sprite = rover;
					break;
				}
			}
			else if (r2->sprite)
			{
				if (r2->sprite->x1 > rover->x2 || r2->sprite->x2 < rover->x1)
					continue;
				if (r2->sprite->szt > rover->sz || r2->sprite->sz < rover->szt)
					continue;

				if (r2->sprite->scale > rover->scale)
				{
					entry = R_CreateDrawNode(NULL);
					(entry->prev = r2->prev)->next = entry;
					(entry->next = r2)->prev = entry;
					entry->sprite = rover;
					break;
				}
			}
		}
		if (r2 == &nodehead)
		{
			entry = R_CreateDrawNode(&nodehead);
			entry->sprite = rover;
		}
	}
}

static drawnode_t *R_CreateDrawNode(drawnode_t *link)
{
	drawnode_t *node = nodebankhead.next;

	if (node == &nodebankhead)
	{
		node = malloc(sizeof (*node));
		if (!node)
			I_Error("No more free memory to CreateDrawNode");
	}
	else
		(nodebankhead.next = node->next)->prev = &nodebankhead;

	if (link)
	{
		node->next = link;
		node->prev = link->prev;
		link->prev->next = node;
		link->prev = node;
	}

	node->plane = NULL;
	node->seg = NULL;
	node->thickseg = NULL;
	node->ffloor = NULL;
	node->sprite = NULL;
	return node;
}

static void R_DoneWithNode(drawnode_t *node)
{
	(node->next->prev = node->prev)->next = node->next;
	(node->next = nodebankhead.next)->prev = node;
	(node->prev = &nodebankhead)->next = node;
}

static void R_ClearDrawNodes(void)
{
	drawnode_t *rover;
	drawnode_t *next;

	for (rover = nodehead.next; rover != &nodehead ;)
	{
		next = rover->next;
		R_DoneWithNode(rover);
		rover = next;
	}

	nodehead.next = nodehead.prev = &nodehead;
}

void R_InitDrawNodes(void)
{
	nodebankhead.next = nodebankhead.prev = &nodebankhead;
	nodehead.next = nodehead.prev = &nodehead;
}

//
// R_DrawSprite
//
//Fab : 26-04-98:
// NOTE : uses con_clipviewtop, so that when console is on,
//        don't draw the part of sprites hidden under the console
static void R_DrawSprite(vissprite_t *spr)
{
	drawseg_t *ds;
	INT16      clipbot[MAXVIDWIDTH];
	INT16      cliptop[MAXVIDWIDTH];
	INT32        x;
	INT32        r1;
	INT32        r2;
	fixed_t    scale;
	fixed_t    lowscale;
	INT32        silhouette;

	memset(clipbot,0x00,sizeof (clipbot));
	memset(cliptop,0x00,sizeof (cliptop));
	for (x = spr->x1; x <= spr->x2; x++)
		clipbot[x] = cliptop[x] = -2;

	// Scan drawsegs from end to start for obscuring segs.
	// The first drawseg that has a greater scale
	//  is the clip seg.
	//SoM: 4/8/2000:
	// Pointer check was originally nonportable
	// and buggy, by going past LEFT end of array:

	//    for (ds = ds_p-1; ds >= drawsegs; ds--)    old buggy code
	for (ds = ds_p; ds-- > drawsegs ;)
	{
		// determine if the drawseg obscures the sprite
		if (ds->x1 > spr->x2 ||
		    ds->x2 < spr->x1 ||
		    (!ds->silhouette
		     && !ds->maskedtexturecol))
		{
			// does not cover sprite
			continue;
		}

		r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
		r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

		if (ds->scale1 > ds->scale2)
		{
			lowscale = ds->scale2;
			scale = ds->scale1;
		}
		else
		{
			lowscale = ds->scale1;
			scale = ds->scale2;
		}

		if (scale < spr->scale ||
		    (lowscale < spr->scale &&
		     !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
		{
			// masked mid texture?
			/*if (ds->maskedtexturecol)
				R_RenderMaskedSegRange (ds, r1, r2);*/
			// seg is behind sprite
			continue;
		}

		// clip this piece of the sprite
		silhouette = ds->silhouette;

		if (spr->gz >= ds->bsilheight)
			silhouette &= ~SIL_BOTTOM;

		if (spr->gzt <= ds->tsilheight)
			silhouette &= ~SIL_TOP;

		if (silhouette == 1)
		{
			// bottom sil
			for (x = r1; x <= r2; x++)
				if (clipbot[x] == -2)
					clipbot[x] = ds->sprbottomclip[x];
		}
		else if (silhouette == 2)
		{
			// top sil
			for (x = r1; x <= r2; x++)
				if (cliptop[x] == -2)
					cliptop[x] = ds->sprtopclip[x];
		}
		else if (silhouette == 3)
		{
			// both
			for (x = r1; x <= r2; x++)
			{
				if (clipbot[x] == -2)
					clipbot[x] = ds->sprbottomclip[x];
				if (cliptop[x] == -2)
					cliptop[x] = ds->sprtopclip[x];
			}
		}
	}
	//SoM: 3/17/2000: Clip sprites in water.
	if (spr->heightsec != -1)  // only things in specially marked sectors
	{
		fixed_t mh, h;
		INT32 phs = viewplayer->mo->subsector->sector->heightsec;
		if ((mh = sectors[spr->heightsec].floorheight) > spr->gz &&
		    (h = centeryfrac - FixedMul(mh -= viewz, spr->scale)) >= 0 &&
		    (h >>= FRACBITS) < viewheight)
		{
			if (mh <= 0 || (phs != -1 && viewz > sectors[phs].floorheight))
			{                          // clip bottom
				for (x = spr->x1; x <= spr->x2; x++)
					if (clipbot[x] == -2 || h < clipbot[x])
						clipbot[x] = (INT16)h;
			}
			else                        // clip top
			{
				for (x = spr->x1; x <= spr->x2; x++)
					if (cliptop[x] == -2 || h > cliptop[x])
						cliptop[x] = (INT16)h;
			}
		}

		if ((mh = sectors[spr->heightsec].ceilingheight) < spr->gzt &&
		    (h = centeryfrac - FixedMul(mh-viewz, spr->scale)) >= 0 &&
		    (h >>= FRACBITS) < viewheight)
		{
			if (phs != -1 && viewz >= sectors[phs].ceilingheight)
			{                         // clip bottom
				for (x = spr->x1; x <= spr->x2; x++)
					if (clipbot[x] == -2 || h < clipbot[x])
						clipbot[x] = (INT16)h;
			}
			else                       // clip top
			{
				for (x = spr->x1; x <= spr->x2; x++)
					if (cliptop[x] == -2 || h > cliptop[x])
						cliptop[x] = (INT16)h;
			}
		}
	}
	if (spr->cut & SC_TOP && spr->cut & SC_BOTTOM)
	{
		for (x = spr->x1; x <= spr->x2; x++)
		{
			if (cliptop[x] == -2 || spr->szt > cliptop[x])
				cliptop[x] = spr->szt;

			if (clipbot[x] == -2 || spr->sz < clipbot[x])
				clipbot[x] = spr->sz;
		}
	}
	else if (spr->cut & SC_TOP)
	{
		for (x = spr->x1; x <= spr->x2; x++)
		{
			if (cliptop[x] == -2 || spr->szt > cliptop[x])
				cliptop[x] = spr->szt;
		}
	}
	else if (spr->cut & SC_BOTTOM)
	{
		for (x = spr->x1; x <= spr->x2; x++)
		{
			if (clipbot[x] == -2 || spr->sz < clipbot[x])
				clipbot[x] = spr->sz;
		}
	}

	// all clipping has been performed, so draw the sprite

	// check for unclipped columns
	for (x = spr->x1; x <= spr->x2; x++)
	{
		if (clipbot[x] == -2)
			clipbot[x] = (INT16)viewheight;

		if (cliptop[x] == -2)
			//Fab : 26-04-98: was -1, now clips against console bottom
		cliptop[x] = (INT16)con_clipviewtop;
	}

	mfloorclip = clipbot;
	mceilingclip = cliptop;
	R_DrawVisSprite(spr);
}

// Special drawer for precipitation sprites Tails 08-18-2002
static void R_DrawPrecipitationSprite(vissprite_t *spr)
{
	drawseg_t *ds;
	INT16      clipbot[MAXVIDWIDTH];
	INT16      cliptop[MAXVIDWIDTH];
	INT32        x;
	INT32        r1;
	INT32        r2;
	fixed_t    scale;
	fixed_t    lowscale;
	INT32        silhouette;

	memset(clipbot,0x00,sizeof (clipbot));
	memset(cliptop,0x00,sizeof (cliptop));
	for (x = spr->x1; x <= spr->x2; x++)
		clipbot[x] = cliptop[x] = -2;

	// Scan drawsegs from end to start for obscuring segs.
	// The first drawseg that has a greater scale
	//  is the clip seg.
	//SoM: 4/8/2000:
	// Pointer check was originally nonportable
	// and buggy, by going past LEFT end of array:

	//    for (ds = ds_p-1; ds >= drawsegs; ds--)    old buggy code
	for (ds = ds_p; ds-- > drawsegs ;)
	{
		// determine if the drawseg obscures the sprite
		if (ds->x1 > spr->x2 ||
		    ds->x2 < spr->x1 ||
		    (!ds->silhouette &&
		     !ds->maskedtexturecol))
		{
			// does not cover sprite
			continue;
		}

		r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
		r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

		if (ds->scale1 > ds->scale2)
		{
			lowscale = ds->scale2;
			scale = ds->scale1;
		}
		else
		{
			lowscale = ds->scale1;
			scale = ds->scale2;
		}

		if (scale < spr->scale ||
		    (lowscale < spr->scale &&
		     !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
		{
			// masked mid texture?
			/*if (ds->maskedtexturecol)
				R_RenderMaskedSegRange(ds, r1, r2);*/
			// seg is behind sprite
			continue;
		}

		// clip this piece of the sprite
		silhouette = ds->silhouette;

		if (silhouette == 1)
		{
			// bottom sil
			for (x = r1; x <= r2; x++)
				if (clipbot[x] == -2)
					clipbot[x] = ds->sprbottomclip[x];
		}
		else if (silhouette == 2)
		{
			// top sil
			for (x = r1; x <= r2; x++)
				if (cliptop[x] == -2)
					cliptop[x] = ds->sprtopclip[x];
		}
		else if (silhouette == 3)
		{
			// both
			for (x = r1; x <= r2; x++)
			{
				if (clipbot[x] == -2)
					clipbot[x] = ds->sprbottomclip[x];
				if (cliptop[x] == -2)
					cliptop[x] = ds->sprtopclip[x];
			}
		}
	}

	// all clipping has been performed, so draw the sprite

	// check for unclipped columns
	for (x = spr->x1; x <= spr->x2; x++)
	{
		if (clipbot[x] == -2)
			clipbot[x] = (INT16)viewheight;

		if (cliptop[x] == -2)
			//Fab : 26-04-98: was -1, now clips against console bottom
			cliptop[x] = (INT16)con_clipviewtop;
	}

	mfloorclip = clipbot;
	mceilingclip = cliptop;
	R_DrawPrecipitationVisSprite(spr);
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
	drawnode_t *r2;
	drawnode_t *next;

	R_CreateDrawNodes();

	for (r2 = nodehead.next; r2 != &nodehead; r2 = r2->next)
	{
		if (r2->plane)
		{
			next = r2->prev;
			R_DrawSinglePlane(r2->plane);
			R_DoneWithNode(r2);
			r2 = next;
		}
		else if (r2->seg && r2->seg->maskedtexturecol != NULL)
		{
			next = r2->prev;
			R_RenderMaskedSegRange(r2->seg, r2->seg->x1, r2->seg->x2);
			r2->seg->maskedtexturecol = NULL;
			R_DoneWithNode(r2);
			r2 = next;
		}
		else if (r2->thickseg)
		{
			next = r2->prev;
			R_RenderThickSideRange(r2->thickseg, r2->thickseg->x1, r2->thickseg->x2, r2->ffloor);
			R_DoneWithNode(r2);
			r2 = next;
		}
		else if (r2->sprite)
		{
			next = r2->prev;

			// Tails 08-18-2002
			if (r2->sprite->precip == true)
				R_DrawPrecipitationSprite(r2->sprite);
			else
				R_DrawSprite(r2->sprite);

			R_DoneWithNode(r2);
			r2 = next;
		}
	}
	R_ClearDrawNodes();
}

// ==========================================================================
//
//                              SKINS CODE
//
// ==========================================================================

INT32 numskins = 0;
skin_t skins[MAXSKINS+1];
// FIXTHIS: don't work because it must be inistilised before the config load
//#define SKINVALUES
#ifdef SKINVALUES
CV_PossibleValue_t skin_cons_t[MAXSKINS+1];
#endif

static void Sk_SetDefaultValue(skin_t *skin)
{
	INT32 i;
	//
	// setup Sonic as default skin
	//
	memset(skin, 0, sizeof (skin_t));
	strcpy(skin->name, DEFAULTSKIN);
	skin->wadnum = INT16_MAX;
	strcpy(skin->sprite, "");
	strcpy(skin->faceprefix, "SBOSLIFE");
	strcpy(skin->superprefix, "SUPERICO");
	strcpy(skin->nameprefix, "STSONIC");
	strcpy(skin->super, "1");
	strcpy(skin->superanims, "1");
	strcpy(skin->superspin, "1");
	strcpy(skin->starttranscolor, "160");
	strcpy(skin->prefcolor, "7");
	strcpy(skin->normalspeed, "36");
	strcpy(skin->runspeed, "28");
	strcpy(skin->thrustfactor, "5");
	sprintf(skin->accelstart, "%d", 96>>(16-FRACBITS));
	sprintf(skin->acceleration, "%d", 40>>(16-FRACBITS));
	strcpy(skin->jumpfactor, "100");
	strcpy(skin->ability, "0");
	strcpy(skin->ability2, "0");
	strcpy(skin->highres, "0");
	strcpy(skin->thokitem, "0");
	strcpy(skin->ghostthokitem, "1");
	strcpy(skin->spinitem, "0");
	strcpy(skin->ghostspinitem, "1");
	strcpy(skin->actionspd, "60");
	strcpy(skin->maxdash, "60");
	strcpy(skin->mindash, "15");

	for (i = 0; i < sfx_skinsoundslot0; i++)
		if (S_sfx[i].skinsound != -1)
			skin->soundsid[S_sfx[i].skinsound] = i;

	skins[0].spritedef.numframes = sprites[SPR_PLAY].numframes;
	skins[0].spritedef.spriteframes = sprites[SPR_PLAY].spriteframes;
}

//
// Initialize the basic skins
//
void R_InitSkins(void)
{
#ifdef SKINVALUES
	INT32 i;

	for (i = 0; i <= MAXSKINS; i++)
	{
		skin_cons_t[i].value = 0;
		skin_cons_t[i].strvalue = NULL;
	}
#endif

	// initialize free sfx slots for skin sounds
	S_InitRuntimeSounds();

	// skin[0] = Sonic skin
	Sk_SetDefaultValue(&skins[0]);
#ifdef SKINVALUES
	skin_cons_t[0].strvalue = skins[0].name;
#endif

	// make Sonic the default skin
	numskins = 1;

	// add face/facename graphics (special case: 1 to MAXSKINS-1 handled in R_AddSkins)
	ST_LoadFaceGraphics(skins[0].faceprefix, skins[0].superprefix, 0);
	ST_LoadFaceNameGraphics(skins[0].nameprefix, 0);
}

static void R_DoSkinTranslationInit(void)
{
	INT32 i;

	for (i = 0; i <= numskins && numskins < MAXSKINS; i++)
		R_InitSkinTranslationTables(atoi(skins[i].starttranscolor), i);
}

// returns true if the skin name is found (loaded from pwad)
// warning return -1 if not found
INT32 R_SkinAvailable(const char *name)
{
	INT32 i;

	for (i = 0; i < numskins; i++)
	{
		if (stricmp(skins[i].name,name)==0)
			return i;
	}
	return -1;
}

// network code calls this when a 'skin change' is received
void SetPlayerSkin(INT32 playernum, const char *skinname)
{
	INT32 i;
	player_t *player = &players[playernum];

	for (i = 0; i < numskins; i++)
	{
		// search in the skin list
		if (stricmp(skins[i].name, skinname) == 0)
		{
			SetPlayerSkinByNum(playernum, i);
			return;
		}
	}

	if (P_IsLocalPlayer(player))
		CONS_Printf("Skin %s not found\n", skinname);

	SetPlayerSkinByNum(playernum, 0);
}

// Same as SetPlayerSkin, but uses the skin #.
// network code calls this when a 'skin change' is received
void SetPlayerSkinByNum(INT32 playernum, INT32 skinnum)
{
	player_t *player;

	if (skinnum >= 0 && skinnum < numskins) // Make sure it exists!
	{
		players[playernum].skin = skinnum;
		if (players[playernum].mo)
		{
			players[playernum].mo->skin = &skins[skinnum];
			if (atoi(skins[skinnum].highres))
				players[playernum].mo->flags |= MF_HIRES;
			else
				players[playernum].mo->flags &= ~MF_HIRES;
		}

		players[playernum].charability = atoi(skins[skinnum].ability);
		players[playernum].charability2 = atoi(skins[skinnum].ability2);

		players[playernum].charflags = 0;

		if (atoi(skins[skinnum].superanims) == 1)
			players[playernum].charflags |= SF_SUPERANIMS;

		if (atoi(skins[skinnum].superspin) == 1)
			players[playernum].charflags |= SF_SUPERSPIN;

		if (atoi(skins[skinnum].ghostthokitem) == 1)
			players[playernum].charflags |= SF_GHOSTTHOKITEM;

		if (atoi(skins[skinnum].ghostspinitem) == 1)
			players[playernum].charflags |= SF_GHOSTSPINITEM;

		players[playernum].thokitem = atoi(skins[skinnum].thokitem);
		players[playernum].spinitem = atoi(skins[skinnum].spinitem);

		players[playernum].actionspd = atoi(skins[skinnum].actionspd);
		players[playernum].mindash = atoi(skins[skinnum].mindash);
		players[playernum].maxdash = atoi(skins[skinnum].maxdash);

		players[playernum].normalspeed = atoi(skins[skinnum].normalspeed);
		players[playernum].runspeed = atoi(skins[skinnum].runspeed);
		players[playernum].thrustfactor = atoi(skins[skinnum].thrustfactor);
		players[playernum].accelstart = atoi(skins[skinnum].accelstart);
		players[playernum].acceleration = atoi(skins[skinnum].acceleration);

		// Cheat checks!
		if (players[playernum].normalspeed > 36)
			players[playernum].normalspeed = 36;
		if (players[playernum].thrustfactor > 5)
			players[playernum].thrustfactor = 5;
		if (players[playernum].accelstart > 192)
			players[playernum].accelstart = 192;
		if (players[playernum].acceleration > 50)
			players[playernum].acceleration = 50;

		// Convert fractional values in case FRACBITS != 16.
		players[playernum].accelstart >>= (16-FRACBITS);
		players[playernum].acceleration >>= (16-FRACBITS);

		players[playernum].jumpfactor = atoi(skins[skinnum].jumpfactor);

		if (players[playernum].jumpfactor > 100)
			players[playernum].jumpfactor = 100;

		if (players[playernum].jumpfactor < 0)
			players[playernum].jumpfactor = 0;

#ifndef TRANSFIX
		// Set the proper translation tables
		players[playernum].starttranscolor = atoi(skins[skinnum].starttranscolor);
#endif

		players[playernum].prefcolor = atoi(skins[skinnum].prefcolor);

		if (players[playernum].mo)
			P_SetScale(players[playernum].mo, players[playernum].mo->scale);

		return;
	}

	player = &players[playernum];

	if (P_IsLocalPlayer(player))
		CONS_Printf("Skin %d not found\n", skinnum);

	players[playernum].skin = 0;  // not found put the sonic skin

	// a copy of the skin value
	// so that dead body detached from respawning player keeps the skin
	if (players[playernum].mo)
		players[playernum].mo->skin = &skins[0];
}

static void SetSkinValues(consvar_t *var, char *valstr, size_t valstrspace)
{
	if (var->PossibleValue)
	{
		INT32 v = atoi(valstr);

		if (!stricmp(var->PossibleValue[0].strvalue, "MIN"))
		{   // bounded cvar
			INT32 i;
			// search for maximum
			for (i = 1; var->PossibleValue[i].strvalue != NULL; i++)
				if (!stricmp(var->PossibleValue[i].strvalue, "MAX"))
					break;

			if (v < var->PossibleValue[0].value)
			{
				v = var->PossibleValue[0].value;
				snprintf(valstr, valstrspace, "%d", v);
			}
			if (v > var->PossibleValue[i].value)
			{
				v = var->PossibleValue[i].value;
				snprintf(valstr, valstrspace, "%d", v);
			}

			valstr[valstrspace - 1] = '\0';
		}
		else
		{
			// waw spaghetti programming ! :)
			INT32 i;

			// check first strings
			for (i = 0; var->PossibleValue[i].strvalue != NULL; i++)
				if (!stricmp(var->PossibleValue[i].strvalue, valstr))
					goto found;
			if (!v)
				if (strcmp(valstr, "0"))
					goto error;
			// check INT32 now
			for (i = 0; var->PossibleValue[i].strvalue != NULL; i++)
				if (v == var->PossibleValue[i].value)
					goto found;

error:
			// not found
			CONS_Printf("\"%s\" is not a possible value for \"%s\"\n", valstr, var->name);
			if (var->defaultvalue == valstr)
				I_Error("Variable %s default value \"%s\" is not a possible value\n",
					var->name, var->defaultvalue);
			return;
found:
			var->value = var->PossibleValue[i].value;
			var->string = var->PossibleValue[i].strvalue;
			goto finish;
		}
	}

	// free the old value string
	Z_Free(var->zstring);

	var->string = var->zstring = Z_StrDup(valstr);

	var->value = atoi(var->string);

finish:
	var->flags |= CV_MODIFIED;
}

// For loading from saved games
void SetSavedSkin(INT32 playernum, INT32 skinnum, INT32 skincolor)
{
	char val[32];

	players[playernum].skincolor = skincolor % MAXSKINCOLORS;
	snprintf(val, sizeof val, "%d", players[playernum].skincolor);
	val[sizeof val - 1] = '\0';

	SetSkinValues(&cv_skin, skins[skinnum].name,
		sizeof skins[skinnum].name);
	SetSkinValues(&cv_playercolor, val, sizeof val);

	if (players[playernum].mo)
	{
		players[playernum].mo->flags |= MF_TRANSLATION;
		players[playernum].mo->color = (UINT8)players[playernum].skincolor;
	}

	SetPlayerSkinByNum(playernum, skinnum);
}

//
// Add skins from a pwad, each skin preceded by 'S_SKIN' marker
//

// Does the same is in w_wad, but check only for
// the first 6 characters (this is so we can have S_SKIN1, S_SKIN2..
// for wad editors that don't like multiple resources of the same name)
//
static UINT16 W_CheckForSkinMarkerInPwad(UINT16 wadid, UINT16 startlump)
{
	UINT16 i;
	const char *S_SKIN = "S_SKIN";
	lumpinfo_t *lump_p;

	// scan forward, start at <startlump>
	if (startlump < wadfiles[wadid]->numlumps)
	{
		lump_p = wadfiles[wadid]->lumpinfo + startlump;
		for (i = startlump; i < wadfiles[wadid]->numlumps; i++, lump_p++)
			if (memcmp(lump_p->name,S_SKIN,6)==0)
				return i;
	}
	return INT16_MAX; // not found
}

//
// Find skin sprites, sounds & optional status bar face, & add them
//
void R_AddSkins(UINT16 wadnum)
{
	UINT16 lump, lastlump = 0;
	char *buf;
	char *buf2;
	char *stoken;
	char *value;
	size_t size;

	//
	// search for all skin markers in pwad
	//

	while ((lump = W_CheckForSkinMarkerInPwad(wadnum, lastlump)) != INT16_MAX)
	{
		if (numskins > MAXSKINS)
		{
			CONS_Printf("ignored skin (%d skins maximum)\n", MAXSKINS);
			lastlump++;
			continue; // so we know how many skins couldn't be added
		}
		buf = W_CacheLumpNumPwad(wadnum, lump, PU_CACHE);
		size = W_LumpLengthPwad(wadnum, lump);

		// for strtok
		buf2 = malloc(size+1);
		if (!buf2)
			I_Error("R_AddSkins: No more free memory\n");
		M_Memcpy(buf2,buf,size);
		buf2[size] = '\0';

		// set defaults
		Sk_SetDefaultValue(&skins[numskins]);
		skins[numskins].wadnum = wadnum;
		snprintf(skins[numskins].name,
			sizeof skins[numskins].name, "skin %d", numskins);
		skins[numskins].name[sizeof skins[numskins].name - 1] = '\0';
		// parse
		stoken = strtok (buf2, "\r\n= ");
		while (stoken)
		{
			if ((stoken[0] == '/' && stoken[1] == '/')
				|| (stoken[0] == '#'))// skip comments
			{
				stoken = strtok(NULL, "\r\n"); // skip end of line
				goto next_token;              // find the real next token
			}

			value = strtok(NULL, "\r\n= ");

			if (!value)
				I_Error("R_AddSkins: syntax error in S_SKIN lump# %d(%s) in WAD %s\n", lump, W_CheckNameForNumPwad(wadnum,lump), wadfiles[wadnum]->filename);

			if (!stricmp(stoken, "name"))
			{
				// the skin name must uniquely identify a single skin
				// I'm lazy so if name is already used I leave the 'skin x'
				// default skin name set above
				if (R_SkinAvailable(value) == -1)
				{
					STRBUFCPY(skins[numskins].name, value);
					strlwr(skins[numskins].name);
				}
				// I'm not lazy, so if the name is already used I make the name 'namex'
				// using the default skin name's number set above
				else
				{
					const size_t stringspace =
						strlen(value) + sizeof (numskins) + 1;
					char *value2 = Z_Malloc(stringspace, PU_STATIC, NULL);
					snprintf(value2, stringspace,
						"%s%d", value, numskins);
					value2[stringspace - 1] = '\0';
					if (R_SkinAvailable(value2) == -1)
					{
						STRBUFCPY(skins[numskins].name,
							value2);
						strlwr(skins[numskins].name);
					}
					Z_Free(value2);
				}
			}
// Macro to use for a lot of repeated code:
// str is the string used in the skin config file,
// field is the field in skin_t.
#define GETSKINATTRIB(str,field) \
	else if (!stricmp(stoken, str))\
	{\
		STRBUFCPY(skins[numskins].field, value);\
		strupr(skins[numskins].field);\
	}

// For those with the same name for the config file and internal field.
#define GETSKINATTRIB_(field) GETSKINATTRIB(#field, field)

			GETSKINATTRIB("face", faceprefix)
			GETSKINATTRIB("superface", superprefix)
			GETSKINATTRIB("facename", nameprefix) // Life icon name

			// character type identification
			GETSKINATTRIB_(ability)
			GETSKINATTRIB_(ability2)

			GETSKINATTRIB_(runspeed)
			GETSKINATTRIB_(normalspeed)
			GETSKINATTRIB_(thrustfactor)
			GETSKINATTRIB_(accelstart)
			GETSKINATTRIB_(acceleration)
			GETSKINATTRIB_(superanims)
			GETSKINATTRIB_(superspin)
			GETSKINATTRIB_(thokitem)
			GETSKINATTRIB_(ghostthokitem)
			GETSKINATTRIB_(spinitem)
			GETSKINATTRIB_(ghostspinitem)
			GETSKINATTRIB_(actionspd)
			GETSKINATTRIB_(mindash)
			GETSKINATTRIB_(maxdash)

			// custom translation table
			else if (!stricmp(stoken, "startcolor"))
			{
				UINT8 colorval;

				STRBUFCPY(skins[numskins].starttranscolor,
					value);
				strupr(skins[numskins].starttranscolor);

				colorval = (UINT8)atoi(skins[numskins].starttranscolor);
			}

			GETSKINATTRIB_(prefcolor)
			GETSKINATTRIB("jumpheight", jumpfactor)
			GETSKINATTRIB_(highres)
			GETSKINATTRIB_(sprite)
			else
			{
				INT32 found = false;
				sfxenum_t i;
				// copy name of sounds that are remapped
				// for this skin
				for (i = 0; i < sfx_skinsoundslot0; i++)
				{
					if (!S_sfx[i].name)
						continue;
					if (S_sfx[i].skinsound != -1
						&& !stricmp(S_sfx[i].name,
							stoken + 2))
					{
						skins[numskins].soundsid[S_sfx[i].skinsound] =
							S_AddSoundFx(value+2,S_sfx[i].singularity,S_sfx[i].pitch, true);
						found = true;
					}
				}
				if (!found)
					CONS_Printf("R_AddSkins: Unknown keyword '%s' in S_SKIN lump# %d (WAD %s)\n", stoken, lump, wadfiles[wadnum]->filename);
			}
next_token:
			stoken = strtok(NULL, "\r\n= ");
		}
		free(buf2);

		lump++; // if no sprite defined use spirte just after this one
		if (skins[numskins].sprite[0] == '\0')
		{
			const char *csprname = W_CheckNameForNumPwad(wadnum, lump);

			// skip to end of this skin's frames
			lastlump = lump;
			while (W_CheckNameForNumPwad(wadnum,lastlump) && memcmp(W_CheckNameForNumPwad(wadnum, lastlump),csprname,4)==0)
				lastlump++;
			// allocate (or replace) sprite frames, and set spritedef
			R_AddSingleSpriteDef(csprname, &skins[numskins].spritedef, wadnum, lump, lastlump);
		}
		else
		{
			// search in the normal sprite tables
			size_t name;
			boolean found = false;
			const char *sprname = skins[numskins].sprite;
			for (name = 0;sprnames[name][0] != '\0';name++)
				if (strcmp(sprnames[name], sprname) == 0)
				{
					found = true;
					skins[numskins].spritedef = sprites[name];
				}

			// not found so make a new one
			if (!found)
				R_AddSingleSpriteDef(sprname, &skins[numskins].spritedef, wadnum, 0, INT16_MAX);

			while (W_CheckNameForNumPwad(wadnum,lastlump) && memcmp(W_CheckNameForNumPwad(wadnum, lastlump),sprname,4)==0)
				lastlump++;
		}

		R_DoSkinTranslationInit();

		CONS_Printf("added skin '%s'\n", skins[numskins].name);
#ifdef SKINVALUES
		skin_cons_t[numskins].value = numskins;
		skin_cons_t[numskins].strvalue = skins[numskins].name;
#endif

		// add face/facename graphics
		if (!atoi(skins[numskins].super))
		{
			strncpy(skins[numskins].superprefix, skins[numskins].faceprefix, 8);
			skins[numskins].superprefix[8] = '\0';
		}

		ST_LoadFaceGraphics(skins[numskins].faceprefix, skins[numskins].superprefix, numskins);
		ST_LoadFaceNameGraphics(skins[numskins].nameprefix, numskins);

		numskins++;
	}
	return;
}

void R_DelSkins(UINT16 wadnum)
{
	UINT16 lump, lastlump = 0;
	while ((lump = W_CheckForSkinMarkerInPwad(wadnum, lastlump)) != INT16_MAX)
	{
		if (skins[numskins].wadnum != wadnum)
			break;
		numskins--;
		ST_UnLoadFaceNameGraphics(numskins);
		ST_UnLoadFaceGraphics(numskins);
		if (skins[numskins].sprite[0] != '\0')
		{
			const char *csprname = W_CheckNameForNumPwad(wadnum, lump);

			// skip to end of this skin's frames
			lastlump = lump;
			while (W_CheckNameForNumPwad(wadnum,lastlump) && memcmp(W_CheckNameForNumPwad(wadnum, lastlump),csprname,4)==0)
				lastlump++;
			// allocate (or replace) sprite frames, and set spritedef
			R_DelSingleSpriteDef(csprname, &skins[numskins].spritedef, wadnum, lump, lastlump);
		}
		else
		{
			// search in the normal sprite tables
			size_t name;
			boolean found = false;
			const char *sprname = skins[numskins].sprite;
			for (name = 0;sprnames[name][0] != '\0';name++)
				if (strcmp(sprnames[name], sprname) == 0)
				{
					found = true;
					skins[numskins].spritedef = sprites[name];
				}

			// not found so make a new one
			if (!found)
				R_DelSingleSpriteDef(sprname, &skins[numskins].spritedef, wadnum, 0, INT16_MAX);

			while (W_CheckNameForNumPwad(wadnum,lastlump) && memcmp(W_CheckNameForNumPwad(wadnum, lastlump),sprname,4)==0)
				lastlump++;
		}
		CONS_Printf("removed skin '%s'\n", skins[numskins].name);
	}
}
