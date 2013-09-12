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
/// \brief Refresh/render internal state variables (global)

#ifndef __R_STATE__
#define __R_STATE__

// Need data structure definitions.
#include "d_player.h"
#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// Refresh internal data structures, for rendering.
//

// needed for texture pegging
extern fixed_t *textureheight;

// needed for pre rendering (fracs)
typedef struct
{
	fixed_t width;
	fixed_t offset;
	fixed_t topoffset;
	fixed_t height;
} sprcache_t;

extern sprcache_t *spritecachedinfo;

extern lighttable_t *colormaps;

// Boom colormaps.
// Had to put a limit on colormaps :(
#define MAXCOLORMAPS 60

extern size_t num_extra_colormaps;
extern extracolormap_t extra_colormaps[MAXCOLORMAPS];

// for global animation
extern INT32 *texturetranslation;

// Sprites
extern size_t numspritelumps;

//
// Lookup tables for map data.
//
extern size_t numsprites;
extern spritedef_t *sprites;

extern size_t numvertexes;
extern vertex_t *vertexes;

extern size_t numsegs;
extern seg_t *segs;

extern size_t numsectors;
extern sector_t *sectors;

extern size_t numsubsectors;
extern subsector_t *subsectors;

extern size_t numnodes;
extern node_t *nodes;

extern size_t numlines;
extern line_t *lines;

extern size_t numsides;
extern side_t *sides;

//
// POV data.
//
extern fixed_t viewx, viewy, viewz;
extern sector_t *viewsector;

extern angle_t viewangle, aimingangle;
extern player_t *viewplayer;

extern consvar_t cv_allowmlook;

extern angle_t clipangle;
extern angle_t doubleclipangle;

extern INT32 viewangletox[FINEANGLES/2];
extern angle_t xtoviewangle[MAXVIDWIDTH+1];

extern fixed_t rw_distance;
extern angle_t rw_normalangle;

// angle to line origin
extern angle_t rw_angle1;

// Segs count?
extern size_t sscount;

#endif
