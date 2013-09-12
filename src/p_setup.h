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
/// \brief Setup a game, startup stuff

#ifndef __P_SETUP__
#define __P_SETUP__

#include "doomdata.h"
#include "r_defs.h"

// map md5, sent to players via PT_SERVERINFO
extern unsigned char mapmd5[16];

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS 64
extern mapthing_t *deathmatchstarts[MAX_DM_STARTS];
extern INT32 numdmstarts, numcoopstarts, numredctfstarts, numbluectfstarts;

extern boolean levelloading;

extern lumpnum_t lastloadedmaplumpnum; // for comparative savegame
//
// MAP used flats lookup table
//
typedef struct
{
	char name[9]; // resource name from wad
	lumpnum_t lumpnum; // lump number of the flat

	// for flat animation
	lumpnum_t baselumpnum;
	INT32 animseq; // start pos. in the anim sequence
	INT32 numpics;
	INT32 speed;
} levelflat_t;

extern size_t numlevelflats;
extern levelflat_t *levelflats;
INT32 P_AddLevelFlat(const char *flatname, levelflat_t *levelflat);

extern size_t nummapthings;
extern mapthing_t *mapthings;

void P_SetupLevelSky(INT32 skynum);
void P_SpawnSecretItems(boolean loademblems);
void P_LoadThingsOnly(void);
void P_RehitStarposts(void);
boolean P_SetupLevel(INT32 map, boolean skipprecip);
boolean P_AddWadFile(const char *wadfilename, char **firstmapname);
boolean P_DelWadFile(void);
boolean P_RunSOC(const char *socfilename);
void P_WriteThings(lumpnum_t lump);
size_t P_PrecacheLevelFlats(void);
void P_InitMapHeaders(void);
void P_ClearMapHeaderInfo(void);

#endif
