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
/// \brief Movement/collision utility functions, as used by functions in p_map.c
///
///	Blockmap iterator functions, and some PIT_* functions to use for iteration

#include "p_local.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_polyobj.h"
#include "z_zone.h"

//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//
fixed_t P_AproxDistance(fixed_t dx, fixed_t dy)
{
	dx = abs(dx);
	dy = abs(dy);
	if (dx < dy)
		return dx + dy - (dx>>1);
	return dx + dy - (dy>>1);
}

//
// P_PartialDistance
// Useful only for iterations finding the 'closest point'
//
FUNCMATH static inline fixed_t P_PartialDistance(fixed_t dx, fixed_t dy)
{
	dx >>= FRACBITS;
	dy >>= FRACBITS;

	dx *= dx;
	dy *= dy;

	return dx + dy;
}

//
// P_ClosestPointOnLine
// Finds the closest point on a given line to the supplied point
//
void P_ClosestPointOnLine(fixed_t x, fixed_t y, line_t *line, vertex_t *result)
{
	fixed_t startx = line->v1->x;
	fixed_t starty = line->v1->y;
	fixed_t dx = line->dx;
	fixed_t dy = line->dy;

	// Determine t (the length of the vector from �Line[0]� to �p�)
	fixed_t cx, cy;
	fixed_t vx, vy;
	fixed_t magnitude;
	fixed_t t;

	//Sub (p, &Line[0], &c);
	cx = x - startx;
	cy = y - starty;

	//Sub (&Line[1], &Line[0], &V);
	vx = dx;
	vy = dy;

	//Normalize (&V, &V);
	magnitude = R_PointToDist2(line->v2->x, line->v2->y, startx, starty);
	vx = FixedDiv(vx, magnitude);
	vy = FixedDiv(vy, magnitude);

	t = (FixedMul(vx, cx) + FixedMul(vy, cy));

	// Return the point between �Line[0]� and �Line[1]�
	vx = FixedMul(vx, t);
	vy = FixedMul(vy, t);

	//Add (&Line[0], &V, out);
	result->x = startx + vx;
	result->y = starty + vy;
	return;
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
INT32 P_PointOnLineSide(fixed_t x, fixed_t y, line_t *line)
{
	const vertex_t *v1 = line->v1;
	fixed_t dx, dy, left, right;

	if (!line->dx)
	{
		if (x <= v1->x)
			return (line->dy > 0);

		return (line->dy < 0);
	}
	if (!line->dy)
	{
		if (y <= v1->y)
			return (line->dx < 0);

		return (line->dx > 0);
	}

	dx = (x - v1->x);
	dy = (y - v1->y);

	left = FixedMul(line->dy>>FRACBITS, dx);
	right = FixedMul(dy, line->dx>>FRACBITS);

	if (right < left)
		return 0; // front side
	return 1; // back side
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
INT32 P_BoxOnLineSide(fixed_t *tmbox, line_t *ld)
{
	INT32 p1, p2;

	switch (ld->slopetype)
	{
		case ST_HORIZONTAL:
			p1 = tmbox[BOXTOP] > ld->v1->y;
			p2 = tmbox[BOXBOTTOM] > ld->v1->y;
			if (ld->dx < 0)
			{
				p1 ^= 1;
				p2 ^= 1;
			}
			break;

		case ST_VERTICAL:
			p1 = tmbox[BOXRIGHT] < ld->v1->x;
			p2 = tmbox[BOXLEFT] < ld->v1->x;
			if (ld->dy < 0)
			{
				p1 ^= 1;
				p2 ^= 1;
			}
			break;

		case ST_POSITIVE:
			p1 = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld);
			p2 = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld);
			break;

		case ST_NEGATIVE:
			p1 = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld);
			p2 = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld);
			break;

		default:
			I_Error("P_BoxOnLineSide: unknown slopetype %d\n", ld->slopetype);
			return -1;
	}

	if (p1 == p2)
		return p1;
	return -1;
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
static INT32 P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t *line)
{
	fixed_t dx, dy, left, right;

	if (!line->dx)
	{
		if (x <= line->x)
			return line->dy > 0;

		return line->dy < 0;
	}
	if (!line->dy)
	{
		if (y <= line->y)
			return line->dx < 0;

		return line->dx > 0;
	}

	dx = (x - line->x);
	dy = (y - line->y);

	// try to quickly decide by looking at sign bits
	if ((line->dy ^ line->dx ^ dx ^ dy) & 0x80000000)
	{
		if ((line->dy ^ dx) & 0x80000000)
			return 1; // left is negative
		return 0;
	}

	left = FixedMul(line->dy>>8, dx>>8);
	right = FixedMul(dy>>8, line->dx>>8);

	if (right < left)
		return 0; // front side
	return 1; // back side
}

//
// P_MakeDivline
//
void P_MakeDivline(line_t *li, divline_t *dl)
{
	dl->x = li->v1->x;
	dl->y = li->v1->y;
	dl->dx = li->dx;
	dl->dy = li->dy;
}

//
// P_InterceptVector
// Returns the fractional intercept point along the first divline.
// This is only called by the addthings and addlines traversers.
//
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1)
{
	fixed_t frac, num, den;

	den = FixedMul(v1->dy>>8, v2->dx) - FixedMul(v1->dx>>8, v2->dy);

	if (!den)
		return 0;

	num = FixedMul((v1->x - v2->x)>>8, v1->dy) + FixedMul((v2->y - v1->y)>>8, v1->dx);
	frac = FixedDiv(num, den);

	return frac;
}

//
// P_LineOpening
// Sets opentop and openbottom to the window through a two sided line.
// OPTIMIZE: keep this precalculated
//
fixed_t opentop, openbottom, openrange, lowfloor;

// P_CameraLineOpening
// P_LineOpening, but for camera
// Tails 09-29-2002
void P_CameraLineOpening(line_t *linedef)
{
	sector_t *front;
	sector_t *back;
	fixed_t frontfloor, frontceiling, backfloor, backceiling;

	if (linedef->sidenum[1] == 0xffff)
	{
		// single sided line
		openrange = 0;
		return;
	}

	front = linedef->frontsector;
	back = linedef->backsector;

	// Cameras use the heightsec's heights rather then the actual sector heights.
	// If you can see through it, why not move the camera through it too?
	if (front->heightsec >= 0)
	{
		frontfloor = sectors[front->heightsec].floorheight;
		frontceiling = sectors[front->heightsec].ceilingheight;
	}
	else
	{
		frontfloor = front->floorheight;
		frontceiling = front->ceilingheight;
	}
	if (back->heightsec >= 0)
	{
		backfloor = sectors[back->heightsec].floorheight;
		backceiling = sectors[back->heightsec].ceilingheight;
	}
	else
	{
		backfloor = back->floorheight;
		backceiling = back->ceilingheight;
	}

	{
		fixed_t thingbot, thingtop;

		thingbot = camera.z;
		thingtop = thingbot + camera.height;

		if (frontceiling < backceiling)
			opentop = frontceiling;
		else
			opentop = backceiling;

		if (frontfloor > backfloor)
		{
			openbottom = frontfloor;
			lowfloor = backfloor;
		}
		else
		{
			openbottom = backfloor;
			lowfloor = frontfloor;
		}

		// Check for fake floors in the sector.
		if (front->ffloors || back->ffloors)
		{
			ffloor_t *rover;
			fixed_t lowestceiling = opentop;
			fixed_t highestfloor = openbottom;
			fixed_t lowestfloor = lowfloor;
			fixed_t delta1, delta2;

			thingtop = camera.z + camera.height;

			// Check for frontsector's fake floors
			if (front->ffloors)
				for (rover = front->ffloors; rover; rover = rover->next)
				{
					if (!(rover->flags & FF_BLOCKOTHERS) || !(rover->flags & FF_RENDERALL) || !(rover->flags & FF_EXISTS))
						continue;

					delta1 = abs(camera.z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
					delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
					if (*rover->bottomheight < lowestceiling && delta1 >= delta2)
						lowestceiling = *rover->bottomheight;

					if (*rover->topheight > highestfloor && delta1 < delta2)
						highestfloor = *rover->topheight;
					else if (*rover->topheight > lowestfloor && delta1 < delta2)
						lowestfloor = *rover->topheight;
				}

			// Check for backsectors fake floors
			if (back->ffloors)
				for (rover = back->ffloors; rover; rover = rover->next)
				{
					if (!(rover->flags & FF_BLOCKOTHERS) || !(rover->flags & FF_RENDERALL) || !(rover->flags & FF_EXISTS))
						continue;

					delta1 = abs(camera.z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
					delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
					if (*rover->bottomheight < lowestceiling && delta1 >= delta2)
						lowestceiling = *rover->bottomheight;

					if (*rover->topheight > highestfloor && delta1 < delta2)
						highestfloor = *rover->topheight;
					else if (*rover->topheight > lowestfloor && delta1 < delta2)
						lowestfloor = *rover->topheight;
				}

			if (highestfloor > openbottom)
				openbottom = highestfloor;

			if (lowestceiling < opentop)
				opentop = lowestceiling;

			if (lowestfloor > lowfloor)
				lowfloor = lowestfloor;
		}
		openrange = opentop - openbottom;
		return;
	}
}

void P_LineOpening(line_t *linedef)
{
	sector_t *front, *back;

	if (linedef->sidenum[1] == 0xffff)
	{
		// single sided line
		openrange = 0;
		return;
	}

	// Treat polyobjects kind of like 3D Floors
#ifdef POLYOBJECTS
	if (linedef->polyobj && (linedef->polyobj->flags & POF_TESTHEIGHT))
	{
		front = linedef->frontsector;
		back = linedef->frontsector;
	}
	else
#endif
	{
		front = linedef->frontsector;
		back = linedef->backsector;
	}

	I_Assert(front != NULL);
	I_Assert(back != NULL);

	if (tmthing)
	{
		fixed_t thingbot, thingtop;

		thingbot = tmthing->z;
		thingtop = thingbot + tmthing->height;

		if (front->ceilingheight < back->ceilingheight)
			opentop = front->ceilingheight;
		else
			opentop = back->ceilingheight;

		if (front->floorheight > back->floorheight)
		{
			openbottom = front->floorheight;
			lowfloor = back->floorheight;
		}
		else
		{
			openbottom = back->floorheight;
			lowfloor = front->floorheight;
		}

		// Check for fake floors in the sector.
		if (front->ffloors || back->ffloors
#ifdef POLYOBJECTS
		    || linedef->polyobj
#endif
		   )
		{
			ffloor_t *rover;

			fixed_t lowestceiling = opentop;
			fixed_t highestfloor = openbottom;
			fixed_t lowestfloor = lowfloor;
			fixed_t delta1;
			fixed_t delta2;

			if (!tmthing)
				goto no_thing;

			thingtop = tmthing->z + tmthing->height;

			// Check for frontsector's fake floors
			for (rover = front->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(((rover->flags & FF_BLOCKPLAYER) && tmthing->player)
				|| ((rover->flags & FF_BLOCKOTHERS) && !tmthing->player))) continue;

				delta1 = abs(tmthing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
				delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
				if (*rover->bottomheight < lowestceiling && delta1 >= delta2)
				{
					if (!(rover->flags & FF_PLATFORM))
						lowestceiling = *rover->bottomheight;
				}
				if (*rover->topheight < highestfloor && delta1 >= delta2)
				{
					if (!(rover->flags & FF_REVERSEPLATFORM))
						lowestceiling = *rover->topheight;
				}

				if (*rover->topheight > highestfloor && delta1 < delta2)
					highestfloor = *rover->topheight;
				else if (*rover->topheight > lowestfloor && delta1 < delta2)
					lowestfloor = *rover->topheight;
			}

			// Check for backsectors fake floors
			for (rover = back->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(((rover->flags & FF_BLOCKPLAYER) && tmthing->player)
				|| ((rover->flags & FF_BLOCKOTHERS) && !tmthing->player))) continue;

				delta1 = abs(tmthing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
				delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
				if (*rover->bottomheight < lowestceiling && delta1 >= delta2)
				{
					if (!(rover->flags & FF_PLATFORM))
						lowestceiling = *rover->bottomheight;
				}
				if (*rover->topheight < highestfloor && delta1 >= delta2)
				{
					if (!(rover->flags & FF_REVERSEPLATFORM))
						lowestceiling = *rover->topheight;
				}

				if (*rover->topheight > highestfloor && delta1 < delta2)
					highestfloor = *rover->topheight;
				else if (*rover->topheight > lowestfloor && delta1 < delta2)
					lowestfloor = *rover->topheight;
			}

#ifdef POLYOBJECTS
			// Treat polyobj's backsector like a 3D Floor
			if (linedef->polyobj && (linedef->polyobj->flags & POF_TESTHEIGHT))
			{
				const sector_t *polysec = linedef->backsector;

				delta1 = abs(tmthing->z - (polysec->floorheight + ((polysec->ceilingheight - polysec->floorheight)/2)));
				delta2 = abs(thingtop - (polysec->floorheight + ((polysec->ceilingheight - polysec->floorheight)/2)));
				if (polysec->floorheight < lowestceiling && delta1 >= delta2)
					lowestceiling = polysec->floorheight;

				if (polysec->ceilingheight > highestfloor && delta1 < delta2)
					highestfloor = polysec->ceilingheight;
				else if (polysec->ceilingheight > lowestfloor && delta1 < delta2)
					lowestfloor = polysec->ceilingheight;
			}
#endif

			if (highestfloor > openbottom)
				openbottom = highestfloor;

			if (lowestceiling < opentop)
				opentop = lowestceiling;

			if (lowestfloor > lowfloor)
				lowfloor = lowestfloor;
		}
		openrange = opentop - openbottom;
		return;
	}

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;

	if (front->floorheight > back->floorheight)
	{
		openbottom = front->floorheight;
		lowfloor = back->floorheight;
	}
	else
	{
		openbottom = back->floorheight;
		lowfloor = front->floorheight;
	}

no_thing:

	openrange = opentop - openbottom;
}


//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//
void P_UnsetThingPosition(mobj_t *thing)
{
	// Better safe than sorry!
	if (!thing)
	{
		CONS_Printf("P_SetUnsetThingPosition: Tried to unset a thing that doesn't exist!\n");
		return;
	}

	if (!(thing->flags & MF_NOSECTOR))
	{
		/* invisible things don't need to be in sector list
		* unlink from subsector
		*
		* killough 8/11/98: simpler scheme using pointers-to-pointers for prev
		* pointers, allows head node pointers to be treated like everything else
		*/

		mobj_t **sprev = thing->sprev;
		mobj_t  *snext = thing->snext;
		if ((*sprev = snext) != NULL)  // unlink from sector list
			snext->sprev = sprev;

		// phares 3/14/98
		//
		// Save the sector list pointed to by touching_sectorlist.
		// In P_SetThingPosition, we'll keep any nodes that represent
		// sectors the Thing still touches. We'll add new ones then, and
		// delete any nodes for sectors the Thing has vacated. Then we'll
		// put it back into touching_sectorlist. It's done this way to
		// avoid a lot of deleting/creating for nodes, when most of the
		// time you just get back what you deleted anyway.
		//
		// If this Thing is being removed entirely, then the calling
		// routine will clear out the nodes in sector_list.

		sector_list = thing->touching_sectorlist;
		thing->touching_sectorlist = NULL; //to be restored by P_SetThingPosition
	}

	if (!(thing->flags & MF_NOBLOCKMAP))
	{
		/* inert things don't need to be in blockmap
		*
		* killough 8/11/98: simpler scheme using pointers-to-pointers for prev
		* pointers, allows head node pointers to be treated like everything else
		*
		* Also more robust, since it doesn't depend on current position for
		* unlinking. Old method required computing head node based on position
		* at time of unlinking, assuming it was the same position as during
		* linking.
		*/

		mobj_t *bnext, **bprev = thing->bprev;
		if (bprev && (*bprev = bnext = thing->bnext) != NULL)  // unlink from block map
			bnext->bprev = bprev;
	}
}

void P_UnsetPrecipThingPosition(precipmobj_t *thing)
{
	precipmobj_t **sprev = thing->sprev;
	precipmobj_t  *snext = thing->snext;
	if ((*sprev = snext) != NULL)  // unlink from sector list
		snext->sprev = sprev;

	precipsector_list = thing->touching_sectorlist;
	thing->touching_sectorlist = NULL; //to be restored by P_SetPrecipThingPosition
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
void P_SetThingPosition(mobj_t *thing)
{                                                      // link into subsector
	subsector_t *ss;
	sector_t *oldsec = NULL;

	// Better safe than sorry!
	if (!thing)
	{
		CONS_Printf("P_SetThingPosition: Tried to set a thing that doesn't exist!\n");
		return;
	}

	if (thing->player && thing->z <= thing->floorz && thing->subsector)
		oldsec = thing->subsector->sector;

	ss = thing->subsector = R_PointInSubsector(thing->x, thing->y);

	if (!(thing->flags & MF_NOSECTOR))
	{
		// invisible things don't go into the sector links

		// killough 8/11/98: simpler scheme using pointer-to-pointer prev
		// pointers, allows head nodes to be treated like everything else

		mobj_t **link = &ss->sector->thinglist;
		mobj_t *snext = *link;
		if ((thing->snext = snext) != NULL)
			snext->sprev = &thing->snext;
		thing->sprev = link;
		*link = thing;

		// phares 3/16/98
		//
		// If sector_list isn't NULL, it has a collection of sector
		// nodes that were just removed from this Thing.

		// Collect the sectors the object will live in by looking at
		// the existing sector_list and adding new nodes and deleting
		// obsolete ones.

		// When a node is deleted, its sector links (the links starting
		// at sector_t->touching_thinglist) are broken. When a node is
		// added, new sector links are created.

		P_CreateSecNodeList(thing,thing->x,thing->y);
		thing->touching_sectorlist = sector_list; // Attach to Thing's mobj_t
		sector_list = NULL; // clear for next time
	}

	// link into blockmap
	if (!(thing->flags & MF_NOBLOCKMAP))
	{
		// inert things don't need to be in blockmap
		const INT32 blockx = (unsigned)(thing->x - bmaporgx)>>MAPBLOCKSHIFT;
		const INT32 blocky = (unsigned)(thing->y - bmaporgy)>>MAPBLOCKSHIFT;
		if (blockx >= 0 && blockx < bmapwidth
			&& blocky >= 0 && blocky < bmapheight)
		{
			// killough 8/11/98: simpler scheme using
			// pointer-to-pointer prev pointers --
			// allows head nodes to be treated like everything else

			mobj_t **link = &blocklinks[blocky*bmapwidth + blockx];
			mobj_t *bnext = *link;
			if ((thing->bnext = bnext) != NULL)
				bnext->bprev = &thing->bnext;
			thing->bprev = link;
			*link = thing;
		}
		else // thing is off the map
			thing->bnext = NULL, thing->bprev = NULL;
	}

	// Allows you to 'step' on a new linedef exec when the previous
	// sector's floor is the same height.
	if (thing->player && oldsec != NULL && thing->subsector
		&& oldsec != thing->subsector->sector
		&& thing->z <= thing->subsector->sector->floorheight)
	{
		thing->eflags |= MFE_JUSTSTEPPEDDOWN;
	}
}

void P_SetPrecipitationThingPosition(precipmobj_t *thing)
{
	subsector_t *ss = thing->subsector = R_PointInSubsector(thing->x, thing->y);

	precipmobj_t **link = &ss->sector->preciplist;
	precipmobj_t *snext = *link;
	if ((thing->snext = snext) != NULL)
		snext->sprev = &thing->snext;
	thing->sprev = link;
	*link = thing;

	P_CreatePrecipSecNodeList(thing, thing->x, thing->y);
	thing->touching_sectorlist = precipsector_list; // Attach to Thing's precipmobj_t
	precipsector_list = NULL; // clear for next time
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//


//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
boolean P_BlockLinesIterator(INT32 x, INT32 y, boolean (*func)(line_t *))
{
	INT32 offset;
	const INT32 *list; // Big blockmap
#ifdef POLYOBJECTS
	polymaplink_t *plink; // haleyjd 02/22/06
#endif
	line_t *ld;

	if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
		return true;

	offset = y*bmapwidth + x;

#ifdef POLYOBJECTS
	// haleyjd 02/22/06: consider polyobject lines
	plink = polyblocklinks[offset];

	while (plink)
	{
		polyobj_t *po = plink->po;

		if (po->validcount != validcount) // if polyobj hasn't been checked
		{
			size_t i;
			po->validcount = validcount;

			for (i = 0; i < po->numLines; ++i)
			{
				if (po->lines[i]->validcount == validcount) // line has been checked
					continue;
				po->lines[i]->validcount = validcount;
				if (!func(po->lines[i]))
					return false;
			}
		}
		plink = (polymaplink_t *)(plink->link.next);
	}
#endif

	offset = *(blockmap + offset); // offset = blockmap[y*bmapwidth+x];

	// First index is really empty, so +1 it.
	for (list = blockmaplump + offset + 1; *list != -1; list++)
	{
		ld = &lines[*list];

		if (ld->validcount == validcount)
			continue; // Line has already been checked.

		ld->validcount = validcount;

		if (!func(ld))
			return false;
	}
	return true; // Everything was checked.
}


//
// P_BlockThingsIterator
//
boolean P_BlockThingsIterator(INT32 x, INT32 y, boolean (*func)(mobj_t *))
{
	mobj_t *mobj;

	if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
		return true;

	// Check interaction with the objects in the blockmap.
	for (mobj = blocklinks[y*bmapwidth + x]; mobj; mobj = mobj->bnext)
	{
		if (!func(mobj))
			return false;
	}
	return true;
}

//
// INTERCEPT ROUTINES
//

//SoM: 4/6/2000: Limit removal
static intercept_t *intercepts = NULL;
static intercept_t *intercept_p = NULL;

divline_t trace;
static boolean earlyout;

//SoM: 4/6/2000: Remove limit on intercepts.
static void P_CheckIntercepts(void)
{
	static size_t max_intercepts = 0;
	size_t count = intercept_p - intercepts;

	if (max_intercepts <= count)
	{
		if (!max_intercepts)
			max_intercepts = 128;
		else
			max_intercepts *= 2;

		intercepts = Z_Realloc(intercepts, sizeof (*intercepts) * max_intercepts, PU_STATIC, NULL);

		intercept_p = intercepts + count;
	}
}

//
// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
static boolean PIT_AddLineIntercepts(line_t *ld)
{
	INT32 s1, s2;
	fixed_t frac;
	divline_t dl;

	// avoid precision problems with two routines
	if (trace.dx > FRACUNIT*16 || trace.dy > FRACUNIT*16
		|| trace.dx < -FRACUNIT*16 || trace.dy < -FRACUNIT*16)
	{
		// Hurdler: crash here with phobia when you shoot
		// on the door next the stone bridge
		// stack overflow???
		s1 = P_PointOnDivlineSide(ld->v1->x, ld->v1->y, &trace);
		s2 = P_PointOnDivlineSide(ld->v2->x, ld->v2->y, &trace);
	}
	else
	{
		s1 = P_PointOnLineSide(trace.x, trace.y, ld);
		s2 = P_PointOnLineSide(trace.x+trace.dx, trace.y+trace.dy, ld);
	}

	if (s1 == s2)
		return true; // Line isn't crossed.

	// Hit the line.
	P_MakeDivline(ld, &dl);
	frac = P_InterceptVector(&trace, &dl);

	if (frac < 0)
		return true; // Behind source.

	// Try to take an early out of the check.
	if (earlyout && frac < FRACUNIT && !ld->backsector)
		return false; // stop checking

	P_CheckIntercepts();

	intercept_p->frac = frac;
	intercept_p->isaline = true;
	intercept_p->d.line = ld;
	intercept_p++;

	return true; // continue
}

//
// PIT_AddThingIntercepts
//
static boolean PIT_AddThingIntercepts(mobj_t *thing)
{
	fixed_t px1, py1, px2, py2, frac;
	INT32 s1, s2;
	boolean tracepositive;
	divline_t dl;

	tracepositive = (trace.dx ^ trace.dy) > 0;

	// check a corner to corner crossection for hit
	if (tracepositive)
	{
		px1 = thing->x - thing->radius;
		py1 = thing->y + thing->radius;

		px2 = thing->x + thing->radius;
		py2 = thing->y - thing->radius;
	}
	else
	{
		px1 = thing->x - thing->radius;
		py1 = thing->y - thing->radius;

		px2 = thing->x + thing->radius;
		py2 = thing->y + thing->radius;
	}

	s1 = P_PointOnDivlineSide(px1, py1, &trace);
	s2 = P_PointOnDivlineSide(px2, py2, &trace);

	if (s1 == s2)
		return true; // Line isn't crossed.

	dl.x = px1;
	dl.y = py1;
	dl.dx = px2 - px1;
	dl.dy = py2 - py1;

	frac = P_InterceptVector(&trace, &dl);

	if (frac < 0)
		return true; // Behind source.

	P_CheckIntercepts();

	intercept_p->frac = frac;
	intercept_p->isaline = false;
	intercept_p->d.thing = thing;
	intercept_p++;

	return true; // Keep going.
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
static boolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac)
{
	size_t count;
	fixed_t dist;
	intercept_t *scan, *in = NULL;

	count = intercept_p - intercepts;

	while (count--)
	{
		dist = INT32_MAX;
		for (scan = intercepts; scan < intercept_p; scan++)
		{
			if (scan->frac < dist)
			{
				dist = scan->frac;
				in = scan;
			}
		}

		if (dist > maxfrac)
			return true; // Checked everything in range.

		if (!func(in))
			return false; // Don't bother going farther.

		in->frac = INT32_MAX;
	}

	return true; // Everything was traversed.
}

//
// P_PathTraverse
// Traces a line from x1, y1 to x2, y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
boolean P_PathTraverse(fixed_t px1, fixed_t py1, fixed_t px2, fixed_t py2,
	INT32 flags, traverser_t trav)
{
	fixed_t xt1, yt1, xt2, yt2;
	fixed_t xstep, ystep, partial, xintercept, yintercept;
	INT32 mapx, mapy, mapxstep, mapystep, count;

	earlyout = flags & PT_EARLYOUT;

	validcount++;
	intercept_p = intercepts;

	if (((px1 - bmaporgx) & (MAPBLOCKSIZE-1)) == 0)
		px1 += FRACUNIT; // Don't side exactly on a line.

	if (((py1 - bmaporgy) & (MAPBLOCKSIZE-1)) == 0)
		py1 += FRACUNIT; // Don't side exactly on a line.

	trace.x = px1;
	trace.y = py1;
	trace.dx = px2 - px1;
	trace.dy = py2 - py1;

	px1 -= bmaporgx;
	py1 -= bmaporgy;
	xt1 = (unsigned)px1>>MAPBLOCKSHIFT;
	yt1 = (unsigned)py1>>MAPBLOCKSHIFT;

	px2 -= bmaporgx;
	py2 -= bmaporgy;
	xt2 = (unsigned)px2>>MAPBLOCKSHIFT;
	yt2 = (unsigned)py2>>MAPBLOCKSHIFT;

	if (xt2 > xt1)
	{
		mapxstep = 1;
		partial = FRACUNIT - ((px1>>MAPBTOFRAC) & FRACMASK);
		ystep = FixedDiv(py2 - py1, abs(px2 - px1));
	}
	else if (xt2 < xt1)
	{
		mapxstep = -1;
		partial = (px1>>MAPBTOFRAC) & FRACMASK;
		ystep = FixedDiv(py2 - py1, abs(px2 - px1));
	}
	else
	{
		mapxstep = 0;
		partial = FRACUNIT;
		ystep = 256*FRACUNIT;
	}

	yintercept = (py1>>MAPBTOFRAC) + FixedMul(partial, ystep);

	if (yt2 > yt1)
	{
		mapystep = 1;
		partial = FRACUNIT - ((py1>>MAPBTOFRAC) & FRACMASK);
		xstep = FixedDiv(px2 - px1, abs(py2 - py1));
	}
	else if (yt2 < yt1)
	{
		mapystep = -1;
		partial = (py1>>MAPBTOFRAC) & FRACMASK;
		xstep = FixedDiv(px2 - px1, abs(py2 - py1));
	}
	else
	{
		mapystep = 0;
		partial = FRACUNIT;
		xstep = 256*FRACUNIT;
	}
	xintercept = (px1>>MAPBTOFRAC) + FixedMul(partial, xstep);

	// Step through map blocks.
	// Count is present to prevent a round off error
	// from skipping the break.
	mapx = xt1;
	mapy = yt1;

	for (count = 0; count < 64; count++)
	{
		if (flags & PT_ADDLINES)
			if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
				return false; // early out

		if (flags & PT_ADDTHINGS)
			if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
				return false; // early out

		if (mapx == xt2 && mapy == yt2)
			break;

		if ((yintercept >> FRACBITS) == mapy)
		{
			yintercept += ystep;
			mapx += mapxstep;
		}
		else if ((xintercept >> FRACBITS) == mapx)
		{
			xintercept += xstep;
			mapy += mapystep;
		}
	}
	// Go through the sorted list
	return P_TraverseIntercepts(trav, FRACUNIT);
}


// =========================================================================
//                                                        BLOCKMAP ITERATORS
// =========================================================================

// blockmap iterator for all sorts of use
// your routine must return FALSE to exit the loop earlier
// returns FALSE if the loop exited early after a false return
// value from your user function

//abandoned, maybe I'll need it someday..
/*
boolean P_RadiusLinesCheck(fixed_t radius, fixed_t x, fixed_t y,
	boolean (*func)(line_t *))
{
	INT32 xl, xh, yl, yh;
	INT32 bx, by;

	tmbbox[BOXTOP] = y + radius;
	tmbbox[BOXBOTTOM] = y - radius;
	tmbbox[BOXRIGHT] = x + radius;
	tmbbox[BOXLEFT] = x - radius;

	// check lines
	xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, func))
				return false;
	return true;
}
*/
