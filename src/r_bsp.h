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
/// \brief Refresh module, BSP traversal and handling

#ifndef __R_BSP__
#define __R_BSP__

#ifdef __GNUG__
#pragma interface
#endif

extern seg_t *curline;
extern side_t *sidedef;
extern line_t *linedef;
extern sector_t *frontsector;
extern sector_t *backsector;

// drawsegs are allocated on the fly... see r_segs.c

extern INT32 checkcoord[12][4];

extern drawseg_t *drawsegs;
extern drawseg_t *ds_p;
extern drawseg_t *firstnewseg;
extern INT32 doorclosed;

typedef void (*drawfunc_t)(INT32 start, INT32 stop);

// BSP?
void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);
void R_RenderBSPNode(INT32 bspnum);

#ifdef POLYOBJECTS
void R_SortPolyObjects(subsector_t *sub);

extern size_t numpolys;        // number of polyobjects in current subsector
extern size_t num_po_ptrs;     // number of polyobject pointers allocated
extern polyobj_t **po_ptrs; // temp ptr array to sort polyobject pointers
#endif

sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec, INT32 *floorlightlevel,
	INT32 *ceilinglightlevel, boolean back);

INT32 R_GetPlaneLight(sector_t *sector, fixed_t planeheight, boolean underside);
void R_Prep3DFloors(sector_t *sector);
#endif
