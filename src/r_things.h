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
/// \brief Rendering of moving objects, sprites

#ifndef __R_THINGS__
#define __R_THINGS__

#include "sounds.h"
#include "r_plane.h"

// number of sprite lumps for spritewidth,offset,topoffset lookup tables
// Fab: this is a hack : should allocate the lookup tables per sprite
#define	MAXSPRITELUMPS 8192 // Increase maxspritelumps Graue 11-06-2003

#define MAXVISSPRITES 2048 // added 2-2-98 was 128

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern INT16 negonearray[MAXVIDWIDTH];
extern INT16 screenheightarray[MAXVIDWIDTH];

// vars for R_DrawMaskedColumn
extern INT16 *mfloorclip;
extern INT16 *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t sprbotscreen;
extern fixed_t windowtop;
extern fixed_t windowbottom;

void R_DrawMaskedColumn(column_t *column);
void R_SortVisSprites(void);

//faB: find sprites in wadfile, replace existing, add new ones
//     (only sprites from namelist are added or replaced)
void R_AddSpriteDefs(UINT16 wadnum);
void R_DelSpriteDefs(UINT16 wadnum);
//SoM: 6/5/2000: Light sprites correctly!
void R_AddSprites(sector_t *sec, INT32 lightlevel);
void R_InitSprites(void);
void R_ClearSprites(void);
void R_DrawMasked(void);

// -----------
// SKINS STUFF
// -----------
#define SKINNAMESIZE 16
#define DEFAULTSKIN "sonic\0\0\0\0\0\0\0\0\0\0"

typedef struct
{
	char name[SKINNAMESIZE+1]; // INT16 descriptive name of the skin
	spritedef_t spritedef;
	UINT16 wadnum;
	char sprite[9];
	char faceprefix[9]; // 8 chars+'\0', default is "SBOSLIFE"
	char superprefix[9]; // 8 chars+'\0', default is "SUPERICO"
	char nameprefix[9]; // 8 chars+'\0', default is "STSONIC"
	char ability[2]; // ability definition
	char ability2[2]; // secondary ability definition
	char thokitem[8];
	char ghostthokitem[2];
	char spinitem[8];
	char ghostspinitem[2];
	char actionspd[4];
	char mindash[3];
	char maxdash[3];

	// Lots of super definitions...
	char super[2];
	char superanims[2];
	char superspin[2];

	char normalspeed[3]; // Normal ground

	char runspeed[3]; // Speed that you break into your run animation

	char accelstart[4]; // Acceleration if speed = 0
	char acceleration[3]; // Acceleration
	char thrustfactor[2]; // Thrust = thrustfactor * acceleration

	char jumpfactor[4]; // % of standard jump height

	// Definable color translation table
	char starttranscolor[4];

	char prefcolor[3];

	// Draw the sprite 2x as small?
	char highres[2];

	// specific sounds per skin
	sfxenum_t soundsid[NUMSKINSOUNDS]; // sound # in S_sfx table
} skin_t;

// -----------
// NOT SKINS STUFF !
// -----------
typedef enum
{
	SC_NONE = 0,
	SC_TOP = 1,
	SC_BOTTOM = 2
} spritecut_e;

// A vissprite_t is a thing that will be drawn during a refresh,
// i.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
	// Doubly linked list.
	struct vissprite_s *prev;
	struct vissprite_s *next;

	mobj_t *mobj; // for easy access

	INT32 x1, x2;

	fixed_t gx, gy; // for line side calculation
	fixed_t gz, gzt; // global bottom/top for silhouette clipping
	fixed_t pz, pzt; // physical bottom/top for sorting with 3D floors

	fixed_t startfrac; // horizontal position of x1
	fixed_t scale;
	fixed_t xiscale; // negative if flipped

	fixed_t texturemid;
	lumpnum_t patch;

	lighttable_t *colormap; // for color translation and shadow draw
	                        // maxbright frames as well

	UINT8 *transmap; // for MF2_SHADOW sprites, which translucency table to use

	INT32 mobjflags;

	INT32 heightsec; // height sector for underwater/fake ceiling support

	extracolormap_t *extra_colormap; // global colormaps

	fixed_t xscale;

	// Precalculated top and bottom screen coords for the sprite.
	fixed_t thingheight; // The actual height of the thing (for 3D floors)
	sector_t *sector; // The sector containing the thing.
	INT16 sz, szt;

	spritecut_e cut;

	boolean precip;
	boolean vflip; // Flip vertically
} vissprite_t;

// A drawnode is something that points to a 3D floor, 3D side, or masked
// middle texture. This is used for sorting with sprites.
typedef struct drawnode_s
{
	visplane_t *plane;
	drawseg_t *seg;
	drawseg_t *thickseg;
	ffloor_t *ffloor;
	vissprite_t *sprite;

	struct drawnode_s *next;
	struct drawnode_s *prev;
} drawnode_t;

extern INT32 numskins;
extern skin_t skins[MAXSKINS + 1];

void SetPlayerSkin(INT32 playernum,const char *skinname);
void SetPlayerSkinByNum(INT32 playernum,INT32 skinnum); // Tails 03-16-2002
INT32 R_SkinAvailable(const char *name);
void R_AddSkins(UINT16 wadnum);
void R_DelSkins(UINT16 wadnum);
void R_InitDrawNodes(void);
void SetSavedSkin(INT32 playernum, INT32 skinnum, INT32 skincolor);

char *GetPlayerFacePic(INT32 skinnum);

#endif //__R_THINGS__
