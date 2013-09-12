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
/// \brief BSP traversal, handling of LineSegs for rendering

#include "doomdef.h"
#include "g_game.h"
#include "r_local.h"
#include "r_state.h"

#include "r_splats.h"
#include "p_local.h" // camera
#include "z_zone.h" // Check R_Prep3DFloors

seg_t *curline;
side_t *sidedef;
line_t *linedef;
sector_t *frontsector;
sector_t *backsector;

// very ugly realloc() of drawsegs at run-time, I upped it to 512
// instead of 256.. and someone managed to send me a level with
// 896 drawsegs! So too bad here's a limit removal a-la-Boom
drawseg_t *drawsegs = NULL;
drawseg_t *ds_p = NULL;
drawseg_t *firstnewseg = NULL;

// indicates doors closed wrt automap bugfix:
INT32 doorclosed;

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs(void)
{
	ds_p = drawsegs;
}

// Fix from boom.
#define MAXSEGS (MAXVIDWIDTH/2+1)

// newend is one past the last valid seg
static cliprange_t *newend;
static cliprange_t solidsegs[MAXSEGS];

//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
static void R_ClipSolidWallSegment(INT32 first, INT32 last)
{
	cliprange_t *next;
	cliprange_t *start;

	// Find the first range that touches the range (adjacent pixels are touching).
	start = solidsegs;
	while (start->last < first - 1)
		start++;

	if (first < start->first)
	{
		if (last < start->first - 1)
		{
			// Post is entirely visible (above start), so insert a new clippost.
			R_StoreWallRange(first, last);
			next = newend;
			newend++;
			// NO MORE CRASHING!
			if (newend - solidsegs > MAXSEGS)
				I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");

			while (next != start)
			{
				*next = *(next-1);
				next--;
			}
			next->first = first;
			next->last = last;
			return;
		}

		// There is a fragment above *start.
		R_StoreWallRange(first, start->first - 1);
		// Now adjust the clip size.
		start->first = first;
	}

	// Bottom contained in start?
	if (last <= start->last)
		return;

	next = start;
	while (last >= (next+1)->first - 1)
	{
		// There is a fragment between two posts.
		R_StoreWallRange(next->last + 1, (next+1)->first - 1);
		next++;

		if (last <= next->last)
		{
			// Bottom is contained in next.
			// Adjust the clip size.
			start->last = next->last;
			goto crunch;
		}
	}

	// There is a fragment after *next.
	R_StoreWallRange(next->last + 1, last);
	// Adjust the clip size.
	start->last = last;

	// Remove start+1 to next from the clip list, because start now covers their area.
crunch:
	if (next == start)
		return; // Post just extended past the bottom of one post.

	while (next++ != newend)
		*++start = *next; // Remove a post.

	newend = start + 1;

	// NO MORE CRASHING!
	if (newend - solidsegs > MAXSEGS)
		I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");
}

//
// R_ClipPassWallSegment
// Clips the given range of columns, but does not include it in the clip list.
// Does handle windows, e.g. LineDefs with upper and lower texture.
//
static inline void R_ClipPassWallSegment(INT32 first, INT32 last)
{
	cliprange_t *start;

	// Find the first range that touches the range
	//  (adjacent pixels are touching).
	start = solidsegs;
	while (start->last < first - 1)
		start++;

	if (first < start->first)
	{
		if (last < start->first - 1)
		{
			// Post is entirely visible (above start).
			R_StoreWallRange(first, last);
			return;
		}

		// There is a fragment above *start.
		R_StoreWallRange(first, start->first - 1);
	}

	// Bottom contained in start?
	if (last <= start->last)
		return;

	while (last >= (start+1)->first - 1)
	{
		// There is a fragment between two posts.
		R_StoreWallRange(start->last + 1, (start+1)->first - 1);
		start++;

		if (last <= start->last)
			return;
	}

	// There is a fragment after *next.
	R_StoreWallRange(start->last + 1, last);
}

//
// R_ClearClipSegs
//
void R_ClearClipSegs(void)
{
	solidsegs[0].first = -0x7fffffff;
	solidsegs[0].last = -1;
	solidsegs[1].first = viewwidth;
	solidsegs[1].last = 0x7fffffff;
	newend = solidsegs + 2;
}


// R_DoorClosed
//
// This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// It assumes that Doom has already ruled out a door being closed because
// of front-back closure (e.g. front floor is taller than back ceiling).
static inline INT32 R_DoorClosed(void)
{
	return

	// if door is closed because back is shut:
	backsector->ceilingheight <= backsector->floorheight

	// preserve a kind of transparent door/lift special effect:
	&& (backsector->ceilingheight >= frontsector->ceilingheight || curline->sidedef->toptexture)

	&& (backsector->floorheight <= frontsector->floorheight || curline->sidedef->bottomtexture)

	// properly render skies (consider door "open" if both ceilings are sky):
	&& (backsector->ceilingpic != skyflatnum || frontsector->ceilingpic != skyflatnum);
}

//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec, INT32 *floorlightlevel,
	INT32 *ceilinglightlevel, boolean back)
{
	INT32 mapnum = -1;
	mobj_t *viewmobj = viewplayer->mo;

	if (!viewmobj)
		return sec;

	if (floorlightlevel)
		*floorlightlevel = sec->floorlightsec == -1 ?
			sec->lightlevel : sectors[sec->floorlightsec].lightlevel;

	if (ceilinglightlevel)
		*ceilinglightlevel = sec->ceilinglightsec == -1 ?
			sec->lightlevel : sectors[sec->ceilinglightsec].lightlevel;

	// If the sector has a midmap, it's probably from 280 type
	if (sec->midmap != -1)
		mapnum = sec->midmap;
	else if (sec->heightsec != -1)
	{
		const sector_t *s = &sectors[sec->heightsec];
		INT32 heightsec = R_PointInSubsector(viewmobj->x, viewmobj->y)->sector->heightsec;
		INT32 underwater = heightsec != -1 && viewz <= sectors[heightsec].floorheight;

		if (splitscreen && viewplayer == &players[secondarydisplayplayer]
			&& camera2.chase)
		{
			heightsec = R_PointInSubsector(camera2.x, camera2.y)->sector->heightsec;
		}
		else if (camera.chase && viewplayer == &players[displayplayer])
			heightsec = R_PointInSubsector(camera.x, camera.y)->sector->heightsec;

		// Replace sector being drawn, with a copy to be hacked
		*tempsec = *sec;

		// Replace floor and ceiling height with other sector's heights.
		tempsec->floorheight = s->floorheight;
		tempsec->ceilingheight = s->ceilingheight;

		mapnum = s->midmap;

		if ((underwater && (tempsec->  floorheight = sec->floorheight,
			tempsec->ceilingheight = s->floorheight - 1, !back)) || viewz <= s->floorheight)
		{ // head-below-floor hack
			tempsec->floorpic = s->floorpic;
			tempsec->floor_xoffs = s->floor_xoffs;
			tempsec->floor_yoffs = s->floor_yoffs;
			tempsec->floorpic_angle = s->floorpic_angle;

			if (underwater)
			{
				if (s->ceilingpic == skyflatnum)
				{
					tempsec->floorheight = tempsec->ceilingheight+1;
					tempsec->ceilingpic = tempsec->floorpic;
					tempsec->ceiling_xoffs = tempsec->floor_xoffs;
					tempsec->ceiling_yoffs = tempsec->floor_yoffs;
					tempsec->ceilingpic_angle = tempsec->floorpic_angle;
				}
				else
				{
					tempsec->ceilingpic = s->ceilingpic;
					tempsec->ceiling_xoffs = s->ceiling_xoffs;
					tempsec->ceiling_yoffs = s->ceiling_yoffs;
					tempsec->ceilingpic_angle = s->ceilingpic_angle;
				}
				mapnum = s->bottommap;
			}

			tempsec->lightlevel = s->lightlevel;

			if (floorlightlevel)
				*floorlightlevel = s->floorlightsec == -1 ? s->lightlevel
					: sectors[s->floorlightsec].lightlevel;

			if (ceilinglightlevel)
				*ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel
					: sectors[s->ceilinglightsec].lightlevel;
		}
		else if (heightsec != -1 && viewz >= sectors[heightsec].ceilingheight
			&& sec->ceilingheight > s->ceilingheight)
		{ // Above-ceiling hack
			tempsec->ceilingheight = s->ceilingheight;
			tempsec->floorheight = s->ceilingheight + 1;

			tempsec->floorpic = tempsec->ceilingpic = s->ceilingpic;
			tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
			tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;
			tempsec->floorpic_angle = tempsec->ceilingpic_angle = s->ceilingpic_angle;

			mapnum = s->topmap;

			if (s->floorpic == skyflatnum) // SKYFIX?
			{
				tempsec->ceilingheight = tempsec->floorheight-1;
				tempsec->floorpic = tempsec->ceilingpic;
				tempsec->floor_xoffs = tempsec->ceiling_xoffs;
				tempsec->floor_yoffs = tempsec->ceiling_yoffs;
				tempsec->floorpic_angle = tempsec->ceilingpic_angle;
			}
			else
			{
				tempsec->ceilingheight = sec->ceilingheight;
				tempsec->floorpic = s->floorpic;
				tempsec->floor_xoffs = s->floor_xoffs;
				tempsec->floor_yoffs = s->floor_yoffs;
				tempsec->floorpic_angle = s->floorpic_angle;
			}

			tempsec->lightlevel = s->lightlevel;

			if (floorlightlevel)
				*floorlightlevel = s->floorlightsec == -1 ? s->lightlevel :
			sectors[s->floorlightsec].lightlevel;

			if (ceilinglightlevel)
				*ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel :
			sectors[s->ceilinglightsec].lightlevel;
		}
		sec = tempsec;
	}

	if (mapnum >= 0 && (size_t)mapnum < num_extra_colormaps)
		sec->extra_colormap = &extra_colormaps[mapnum];
	else
		sec->extra_colormap = NULL;

	return sec;
}

//
// R_AddLine
// Clips the given segment and adds any visible pieces to the line list.
//
static void R_AddLine(seg_t *line)
{
	INT32 x1, x2;
	angle_t angle1, angle2, span, tspan;
	static sector_t tempsec; // ceiling/water hack

	if (line->polyseg && !(line->polyseg->flags & POF_RENDERSIDES))
		return;

	curline = line;

	// OPTIMIZE: quickly reject orthogonal back sides.
	angle1 = R_PointToAngle(line->v1->x, line->v1->y);
	angle2 = R_PointToAngle(line->v2->x, line->v2->y);

	// Clip to view edges.
	span = angle1 - angle2;

	// Back side? i.e. backface culling?
	if (span >= ANGLE_180)
		return;

	// Global angle needed by segcalc.
	rw_angle1 = angle1;
	angle1 -= viewangle;
	angle2 -= viewangle;

	tspan = angle1 + clipangle;
	if (tspan > doubleclipangle)
	{
		tspan -= doubleclipangle;

		// Totally off the left edge?
		if (tspan >= span)
			return;

		angle1 = clipangle;
	}
	tspan = clipangle - angle2;
	if (tspan > doubleclipangle)
	{
		tspan -= doubleclipangle;

		// Totally off the left edge?
		if (tspan >= span)
			return;

		angle2 = -(signed)clipangle;
	}

	// The seg is in the view range, but not necessarily visible.
	angle1 = (angle1+ANGLE_90)>>ANGLETOFINESHIFT;
	angle2 = (angle2+ANGLE_90)>>ANGLETOFINESHIFT;
	x1 = viewangletox[angle1];
	x2 = viewangletox[angle2];

	// Does not cross a pixel?
	if (x1 >= x2)       // killough 1/31/98 -- change == to >= for robustness
		return;

	backsector = line->backsector;

	// Single sided line?
	if (!backsector)
		goto clipsolid;

	backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

	doorclosed = 0;

	// Closed door.
	if (backsector->ceilingheight <= frontsector->floorheight
		|| backsector->floorheight >= frontsector->ceilingheight)
	{
		goto clipsolid;
	}

	// Check for automap fix. Store in doorclosed for r_segs.c
	doorclosed = R_DoorClosed();
	if (doorclosed)
		goto clipsolid;

	// Window.
	if (backsector->ceilingheight != frontsector->ceilingheight
		|| backsector->floorheight != frontsector->floorheight)
	{
		goto clippass;
	}

	// Reject empty lines used for triggers and special events.
	// Identical floor and ceiling on both sides, identical light levels on both sides,
	// and no middle texture.
	if (line->linedef->flags & ML_EFFECT6) // Don't even draw these lines
		return;

	if (
#ifdef POLYOBJECTS
		!line->polyseg &&
#endif
		backsector->ceilingpic == frontsector->ceilingpic
		&& backsector->floorpic == frontsector->floorpic
		&& backsector->lightlevel == frontsector->lightlevel
		&& !curline->sidedef->midtexture
		// Check offsets too!
		&& backsector->floor_xoffs == frontsector->floor_xoffs
		&& backsector->floor_yoffs == frontsector->floor_yoffs
		&& backsector->floorpic_angle == frontsector->floorpic_angle
		&& backsector->ceiling_xoffs == frontsector->ceiling_xoffs
		&& backsector->ceiling_yoffs == frontsector->ceiling_yoffs
		&& backsector->ceilingpic_angle == frontsector->ceilingpic_angle
		// Consider altered lighting.
		&& backsector->floorlightsec == frontsector->floorlightsec
		&& backsector->ceilinglightsec == frontsector->ceilinglightsec
		// Consider colormaps
		&& backsector->extra_colormap == frontsector->extra_colormap
		&& ((!frontsector->ffloors && !backsector->ffloors)
		|| frontsector->tag == backsector->tag))
	{
		return;
	}


clippass:
	R_ClipPassWallSegment(x1, x2 - 1);
	return;

clipsolid:
	R_ClipSolidWallSegment(x1, x2 - 1);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true if some part of the bbox might be visible.
//
//   | 0 | 1 | 2
// --+---+---+---
// 0 | 0 | 1 | 2
// 1 | 4 | 5 | 6
// 2 | 8 | 9 | A
INT32 checkcoord[12][4] =
{
	{3, 0, 2, 1},
	{3, 0, 2, 0},
	{3, 1, 2, 0},
	{0}, // UNUSED
	{2, 0, 2, 1},
	{0}, // UNUSED
	{3, 1, 3, 0},
	{0}, // UNUSED
	{2, 0, 3, 1},
	{2, 1, 3, 1},
	{2, 1, 3, 0}
};

static boolean R_CheckBBox(fixed_t *bspcoord)
{
	INT32 boxpos, sx1, sx2;
	fixed_t px1, py1, px2, py2;
	angle_t angle1, angle2, span, tspan;
	cliprange_t *start;

	// Find the corners of the box that define the edges from current viewpoint.
	if (viewx <= bspcoord[BOXLEFT])
		boxpos = 0;
	else if (viewx < bspcoord[BOXRIGHT])
		boxpos = 1;
	else
		boxpos = 2;

	if (viewy >= bspcoord[BOXTOP])
		boxpos |= 0;
	else if (viewy > bspcoord[BOXBOTTOM])
		boxpos |= 1<<2;
	else
		boxpos |= 2<<2;

	if (boxpos == 5)
		return true;

	px1 = bspcoord[checkcoord[boxpos][0]];
	py1 = bspcoord[checkcoord[boxpos][1]];
	px2 = bspcoord[checkcoord[boxpos][2]];
	py2 = bspcoord[checkcoord[boxpos][3]];

	// check clip list for an open space
	angle1 = R_PointToAngle(px1, py1) - viewangle;
	angle2 = R_PointToAngle(px2, py2) - viewangle;

	span = angle1 - angle2;

	// Sitting on a line?
	if (span >= ANGLE_180)
		return true;

	tspan = angle1 + clipangle;

	if (tspan > doubleclipangle)
	{
		tspan -= doubleclipangle;

		// Totally off the left edge?
		if (tspan >= span)
			return false;

		angle1 = clipangle;
	}
	tspan = clipangle - angle2;
	if (tspan > doubleclipangle)
	{
		tspan -= doubleclipangle;

		// Totally off the left edge?
		if (tspan >= span)
			return false;

		angle2 = -(signed)clipangle;
	}

	// Find the first clippost that touches the source post (adjacent pixels are touching).
	angle1 = (angle1+ANGLE_90)>>ANGLETOFINESHIFT;
	angle2 = (angle2+ANGLE_90)>>ANGLETOFINESHIFT;
	sx1 = viewangletox[angle1];
	sx2 = viewangletox[angle2];

	// Does not cross a pixel.
	if (sx1 == sx2)
		return false;
	sx2--;

	start = solidsegs;
	while (start->last < sx2)
		start++;

	if (sx1 >= start->first && sx2 <= start->last)
		return false; // The clippost contains the new span.

	return true;
}

#ifdef POLYOBJECTS

size_t numpolys;        // number of polyobjects in current subsector
size_t num_po_ptrs;     // number of polyobject pointers allocated
polyobj_t **po_ptrs; // temp ptr array to sort polyobject pointers

//
// R_PolyobjCompare
//
// Callback for qsort that compares the z distance of two polyobjects.
// Returns the difference such that the closer polyobject will be
// sorted first.
//
static int R_PolyobjCompare(const void *p1, const void *p2)
{
	const polyobj_t *po1 = *(const polyobj_t * const *)p1;
	const polyobj_t *po2 = *(const polyobj_t * const *)p2;

	return po1->zdist - po2->zdist;
}

//
// R_SortPolyObjects
//
// haleyjd 03/03/06: Here's the REAL meat of Eternity's polyobject system.
// Hexen just figured this was impossible, but as mentioned in polyobj.c,
// it is perfectly doable within the confines of the BSP tree. Polyobjects
// must be sorted to draw in DOOM's front-to-back order within individual
// subsectors. This is a modified version of R_SortVisSprites.
//
void R_SortPolyObjects(subsector_t *sub)
{
	if (numpolys)
	{
		polyobj_t *po;
		INT32 i = 0;

		// allocate twice the number needed to minimize allocations
		if (num_po_ptrs < numpolys*2)
		{
			// use free instead realloc since faster (thanks Lee ^_^)
			free(po_ptrs);
			po_ptrs = malloc((num_po_ptrs = numpolys*2)
				* sizeof(*po_ptrs));
		}

		po = sub->polyList;

		while (po)
		{
			po->zdist = R_PointToDist2(viewx, viewy,
				po->centerPt.x, po->centerPt.y);
			po_ptrs[i++] = po;
			po = (polyobj_t *)(po->link.next);
		}

		// the polyobjects are NOT in any particular order, so use qsort
		// 03/10/06: only bother if there are actually polys to sort
		if (numpolys >= 2)
		{
			qsort(po_ptrs, numpolys, sizeof(polyobj_t *),
				R_PolyobjCompare);
		}
	}
}

//
// R_AddPolyObjects
//
// haleyjd 02/19/06
// Adds all segs in all polyobjects in the given subsector.
//
static void R_AddPolyObjects(subsector_t *sub)
{
	polyobj_t *po = sub->polyList;
	size_t i, j;

	numpolys = 0;

	// count polyobjects
	while (po)
	{
		++numpolys;
		po = (polyobj_t *)(po->link.next);
	}

	// sort polyobjects
	R_SortPolyObjects(sub);

	// render polyobjects
	for (i = 0; i < numpolys; ++i)
	{
		for (j = 0; j < po_ptrs[i]->segCount; ++j)
			R_AddLine(po_ptrs[i]->segs[j]);
	}
}
#endif

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//

drawseg_t *firstseg;

static void R_Subsector(size_t num)
{
	INT32 count, floorlightlevel, ceilinglightlevel, light;
	seg_t *line;
	subsector_t *sub;
	static sector_t tempsec; // Deep water hack
	extracolormap_t *floorcolormap;
	extracolormap_t *ceilingcolormap;

#ifdef RANGECHECK
	if (num >= numsubsectors)
		I_Error("R_Subsector: ss %"PRIdS" with numss = %"PRIdS"\n", num, numsubsectors);
#endif

	// subsectors added at run-time
	if (num >= numsubsectors)
		return;

	sub = &subsectors[num];
	frontsector = sub->sector;
	count = sub->numlines;
	line = &segs[sub->firstline];

	// Deep water/fake ceiling effect.
	frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);

	floorcolormap = ceilingcolormap = frontsector->extra_colormap;

	// Check and prep all 3D floors. Set the sector floor/ceiling light levels and colormaps.
	if (frontsector->ffloors)
	{
		if (frontsector->moved)
		{
			frontsector->numlights = sub->sector->numlights = 0;
			R_Prep3DFloors(frontsector);
			sub->sector->lightlist = frontsector->lightlist;
			sub->sector->numlights = frontsector->numlights;
			sub->sector->moved = frontsector->moved = false;
		}

		light = R_GetPlaneLight(frontsector, frontsector->floorheight, false);
		if (frontsector->floorlightsec == -1)
			floorlightlevel = *frontsector->lightlist[light].lightlevel;
		floorcolormap = frontsector->lightlist[light].extra_colormap;
		light = R_GetPlaneLight(frontsector, frontsector->ceilingheight, false);
		if (frontsector->ceilinglightsec == -1)
			ceilinglightlevel = *frontsector->lightlist[light].lightlevel;
		ceilingcolormap = frontsector->lightlist[light].extra_colormap;
	}

	sub->sector->extra_colormap = frontsector->extra_colormap;

	if ((frontsector->floorheight < viewz || (frontsector->heightsec != -1
		&& sectors[frontsector->heightsec].ceilingpic == skyflatnum)))
	{
		floorplane = R_FindPlane(frontsector->floorheight, frontsector->floorpic, floorlightlevel,
			frontsector->floor_xoffs, frontsector->floor_yoffs, frontsector->floorpic_angle, floorcolormap, NULL);
	}
	else
		floorplane = NULL;

	if ((frontsector->ceilingheight > viewz || frontsector->ceilingpic == skyflatnum
		|| (frontsector->heightsec != -1
		&& sectors[frontsector->heightsec].floorpic == skyflatnum)))
	{
		ceilingplane = R_FindPlane(frontsector->ceilingheight, frontsector->ceilingpic,
			ceilinglightlevel, frontsector->ceiling_xoffs, frontsector->ceiling_yoffs, frontsector->ceilingpic_angle,
			ceilingcolormap, NULL);
	}
	else
		ceilingplane = NULL;

	numffloors = 0;
	ffloor[numffloors].plane = NULL;
	if (frontsector->ffloors)
	{
		ffloor_t *rover;

		for (rover = frontsector->ffloors; rover && numffloors < MAXFFLOORS; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
				continue;

			if (frontsector->cullheight)
			{
				if (frontsector->cullheight->flags & ML_NOCLIMB) // Group culling
				{
					// Make sure this is part of the same group
					if (viewsector->cullheight && viewsector->cullheight->frontsector
						== frontsector->cullheight->frontsector)
					{
						// OK, we can cull
						if (viewz > frontsector->cullheight->frontsector->floorheight
							&& *rover->topheight < frontsector->cullheight->frontsector->floorheight) // Cull if below plane
						{
							rover->norender = leveltime;
							continue;
						}

						if (*rover->bottomheight > frontsector->cullheight->frontsector->floorheight
							&& viewz <= frontsector->cullheight->frontsector->floorheight) // Cull if above plane
						{
							rover->norender = leveltime;
							continue;
						}
					}
				}
				else // Quick culling
				{
					if (viewz > frontsector->cullheight->frontsector->floorheight
						&& *rover->topheight < frontsector->cullheight->frontsector->floorheight) // Cull if below plane
					{
						rover->norender = leveltime;
						continue;
					}

					if (*rover->bottomheight > frontsector->cullheight->frontsector->floorheight
						&& viewz <= frontsector->cullheight->frontsector->floorheight) // Cull if above plane
					{
						rover->norender = leveltime;
						continue;
					}
				}
			}

			ffloor[numffloors].plane = NULL;
			if (*rover->bottomheight <= frontsector->ceilingheight
				&& *rover->bottomheight >= frontsector->floorheight
				&& ((viewz < *rover->bottomheight && !(rover->flags & FF_INVERTPLANES))
				|| (viewz > *rover->bottomheight && (rover->flags & FF_BOTHPLANES))))
			{
				light = R_GetPlaneLight(frontsector, *rover->bottomheight,
					viewz < *rover->bottomheight);
				ffloor[numffloors].plane = R_FindPlane(*rover->bottomheight, *rover->bottompic,
					*frontsector->lightlist[light].lightlevel, *rover->bottomxoffs,
					*rover->bottomyoffs, *rover->bottomangle, frontsector->lightlist[light].extra_colormap, rover);

				ffloor[numffloors].height = *rover->bottomheight;
				ffloor[numffloors].ffloor = rover;
				numffloors++;
			}
			if (numffloors >= MAXFFLOORS)
				break;
			ffloor[numffloors].plane = NULL;
			if (*rover->topheight >= frontsector->floorheight
				&& *rover->topheight <= frontsector->ceilingheight
				&& ((viewz > *rover->topheight && !(rover->flags & FF_INVERTPLANES))
				|| (viewz < *rover->topheight && (rover->flags & FF_BOTHPLANES))))
			{
				light = R_GetPlaneLight(frontsector, *rover->topheight, viewz < *rover->topheight);
				ffloor[numffloors].plane = R_FindPlane(*rover->topheight, *rover->toppic,
					*frontsector->lightlist[light].lightlevel, *rover->topxoffs, *rover->topyoffs, *rover->topangle,
					frontsector->lightlist[light].extra_colormap, rover);
				ffloor[numffloors].height = *rover->topheight;
				ffloor[numffloors].ffloor = rover;
				numffloors++;
			}
		}
	}

#ifdef POLYOBJECTS_PLANES
	// Polyobjects have planes, too!
	if (sub->polyList)
	{
		polyobj_t *po = sub->polyList;
		sector_t *polysec;

		while (po)
		{
			if (numffloors >= MAXFFLOORS)
				break;

			if (!(po->flags & POF_RENDERPLANES)) // Don't draw planes
			{
				po = (polyobj_t *)(po->link.next);
				continue;
			}

			polysec = po->lines[0]->backsector;
			ffloor[numffloors].plane = NULL;

			if (polysec->floorheight <= frontsector->ceilingheight
				&& polysec->floorheight >= frontsector->floorheight
				&& (viewz < polysec->floorheight))
			{
				light = R_GetPlaneLight(frontsector, polysec->floorheight, viewz < polysec->floorheight);
				light = 0;
				ffloor[numffloors].plane = R_FindPlane(polysec->floorheight, polysec->floorpic,
						polysec->lightlevel, polysec->floor_xoffs,
						polysec->floor_yoffs,
						polysec->floorpic_angle,
						NULL,
						NULL);
				ffloor[numffloors].plane->polyobj = true;

				ffloor[numffloors].height = polysec->floorheight;
				ffloor[numffloors].polyobj = po;
//				ffloor[numffloors].ffloor = rover;
				numffloors++;
			}

			if (numffloors >= MAXFFLOORS)
				break;

			ffloor[numffloors].plane = NULL;

			if (polysec->ceilingheight >= frontsector->floorheight
				&& polysec->ceilingheight <= frontsector->ceilingheight
				&& (viewz > polysec->ceilingheight))
			{
				light = R_GetPlaneLight(frontsector, polysec->ceilingheight, viewz < polysec->ceilingheight);
				light = 0;
				ffloor[numffloors].plane = R_FindPlane(polysec->ceilingheight, polysec->ceilingpic,
					polysec->lightlevel, polysec->ceiling_xoffs, polysec->ceiling_yoffs, polysec->ceilingpic_angle,
					NULL, NULL);
				ffloor[numffloors].plane->polyobj = true;
				ffloor[numffloors].polyobj = po;
				ffloor[numffloors].height = polysec->ceilingheight;
//				ffloor[numffloors].ffloor = rover;
				numffloors++;
			}

			po = (polyobj_t *)(po->link.next);
		}
	}
#endif

#ifdef FLOORSPLATS
	if (sub->splats)
		R_AddVisibleFloorSplats(sub);
#endif

   // killough 9/18/98: Fix underwater slowdown, by passing real sector
   // instead of fake one. Improve sprite lighting by basing sprite
   // lightlevels on floor & ceiling lightlevels in the surrounding area.
   //
   // 10/98 killough:
   //
   // NOTE: TeamTNT fixed this bug incorrectly, messing up sprite lighting!!!
   // That is part of the 242 effect!!!  If you simply pass sub->sector to
   // the old code you will not get correct lighting for underwater sprites!!!
   // Either you must pass the fake sector and handle validcount here, on the
   // real sector, or you must account for the lighting in some other way,
   // like passing it as an argument.
	R_AddSprites(sub->sector, (floorlightlevel+ceilinglightlevel)/2);

	firstseg = NULL;

#ifdef POLYOBJECTS
	// haleyjd 02/19/06: draw polyobjects before static lines
	if (sub->polyList)
		R_AddPolyObjects(sub);
#endif

	while (count--)
	{
//		CONS_Printf("Adding normal line %d...(%d)\n", line->linedef-lines, leveltime);
		R_AddLine(line);
		line++;
		curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so stuff doesn't try using it for other things */
	}
}

//
// R_Prep3DFloors
//
// This function creates the lightlists that the given sector uses to light
// floors/ceilings/walls according to the 3D floors.
void R_Prep3DFloors(sector_t *sector)
{
	ffloor_t *rover;
	ffloor_t *best;
	fixed_t bestheight, maxheight;
	INT32 count, i, mapnum;
	sector_t *sec;

	count = 1;
	for (rover = sector->ffloors; rover; rover = rover->next)
	{
		if ((rover->flags & FF_EXISTS) && (!(rover->flags & FF_NOSHADE)
			|| (rover->flags & FF_CUTLEVEL) || (rover->flags & FF_CUTSPRITES)))
		{
			count++;
			if (rover->flags & FF_DOUBLESHADOW)
				count++;
		}
	}

	if (count != sector->numlights)
	{
		Z_Free(sector->lightlist);
		sector->lightlist = Z_Calloc(sizeof (*sector->lightlist) * count, PU_LEVEL, NULL);
		sector->numlights = count;
	}
	else
		memset(sector->lightlist, 0, sizeof (lightlist_t) * count);

	sector->lightlist[0].height = sector->ceilingheight + 1;
	sector->lightlist[0].lightlevel = &sector->lightlevel;
	sector->lightlist[0].caster = NULL;
	sector->lightlist[0].extra_colormap = sector->extra_colormap;
	sector->lightlist[0].flags = 0;

	maxheight = INT32_MAX;
	for (i = 1; i < count; i++)
	{
		bestheight = INT32_MAX * -1;
		best = NULL;
		for (rover = sector->ffloors; rover; rover = rover->next)
		{
			rover->lastlight = 0;
			if (!(rover->flags & FF_EXISTS) || (rover->flags & FF_NOSHADE
				&& !(rover->flags & FF_CUTLEVEL) && !(rover->flags & FF_CUTSPRITES)))
			continue;

			if (*rover->topheight > bestheight && *rover->topheight < maxheight)
			{
				best = rover;
				bestheight = *rover->topheight;
				continue;
			}
			if (rover->flags & FF_DOUBLESHADOW && *rover->bottomheight > bestheight
				&& *rover->bottomheight < maxheight)
			{
				best = rover;
				bestheight = *rover->bottomheight;
				continue;
			}
		}
		if (!best)
		{
			sector->numlights = i;
			return;
		}

		sector->lightlist[i].height = maxheight = bestheight;
		sector->lightlist[i].caster = best;
		sector->lightlist[i].flags = best->flags;
		sec = &sectors[best->secnum];
		mapnum = sec->midmap;
		if (mapnum >= 0 && (size_t)mapnum < num_extra_colormaps)
			sec->extra_colormap = &extra_colormaps[mapnum];
		else
			sec->extra_colormap = NULL;

		if (best->flags & FF_NOSHADE)
		{
			sector->lightlist[i].lightlevel = sector->lightlist[i-1].lightlevel;
			sector->lightlist[i].extra_colormap = sector->lightlist[i-1].extra_colormap;
		}
		else if (best->flags & FF_COLORMAPONLY)
		{
			sector->lightlist[i].lightlevel = sector->lightlist[i-1].lightlevel;
			sector->lightlist[i].extra_colormap = sec->extra_colormap;
		}
		else
		{
			sector->lightlist[i].lightlevel = best->toplightlevel;
			sector->lightlist[i].extra_colormap = sec->extra_colormap;
		}

		if (best->flags & FF_DOUBLESHADOW)
		{
			if (bestheight == *best->bottomheight)
			{
				sector->lightlist[i].lightlevel = sector->lightlist[best->lastlight].lightlevel;
				sector->lightlist[i].extra_colormap =
					sector->lightlist[best->lastlight].extra_colormap;
			}
			else
				best->lastlight = i - 1;
		}
	}
}

INT32 R_GetPlaneLight(sector_t *sector, fixed_t planeheight, boolean underside)
{
	INT32 i;

	if (!underside)
	{
		for (i = 1; i < sector->numlights; i++)
			if (sector->lightlist[i].height <= planeheight)
				return i - 1;

		return sector->numlights - 1;
	}

	for (i = 1; i < sector->numlights; i++)
		if (sector->lightlist[i].height < planeheight)
			return i - 1;

	return sector->numlights - 1;
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
//
// killough 5/2/98: reformatted, removed tail recursion

void R_RenderBSPNode(INT32 bspnum)
{
	node_t *bsp;
	INT32 side;
	while (!(bspnum & NF_SUBSECTOR))  // Found a subsector?
	{
		bsp = &nodes[bspnum];

		// Decide which side the view point is on.
		side = R_PointOnSide(viewx, viewy, bsp);
		// Recursively divide front space.
		R_RenderBSPNode(bsp->children[side]);

		// Possibly divide back space.

		if (!R_CheckBBox(bsp->bbox[side^1]))
			return;

		bspnum = bsp->children[side^1];
	}
	R_Subsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}
