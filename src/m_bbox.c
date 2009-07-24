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
/// \brief bounding boxes

#include "doomtype.h"
#include "m_bbox.h"
/**	\brief	The M_ClearBox function

	\param	box	a fixed_t array of 4 to be cleaned

	\return	void


*/

void M_ClearBox(fixed_t *box)
{
	box[BOXTOP] = box[BOXRIGHT] = MININT;
	box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}
/**	\brief	The M_AddToBox function

	\param	box	a fixed_t array of 4 to be added with
	\param	x	x of box
	\param	y	y of box

	\return	void


*/

void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y)
{
	if (x < box[BOXLEFT])
		box[BOXLEFT] = x;
	if (x > box[BOXRIGHT])
		box[BOXRIGHT] = x;

	if (y < box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	if (y > box[BOXTOP])
		box[BOXTOP] = y;
}
/**	\brief	The M_PointInBox function

	\param	box	a fixed_t array of 4 to be checked with
	\param	x	x of box
	\param	y	y of box

	\return	if it's in the box


*/

boolean M_PointInBox(fixed_t *box, fixed_t x, fixed_t y)
{
	if (x < box[BOXLEFT])
		return false;
	if (x > box[BOXRIGHT])
		return false;
	if (y < box[BOXBOTTOM])
		return false;
	if (y > box[BOXTOP])
		return false;

	return true;
}
/**	\brief	The M_CircleTouchBox function

	\param	box	a parameter of type fixed_t *
	\param	circlex	a parameter of type fixed_t
	\param	circley	a parameter of type fixed_t
	\param	circleradius	a parameter of type fixed_t

	\return	boolean


*/

boolean M_CircleTouchBox(fixed_t *box, fixed_t circlex, fixed_t circley, fixed_t circleradius)
{
	if (box[BOXLEFT] - circleradius > circlex)
		return false;
	if (box[BOXRIGHT] + circleradius < circlex)
		return false;
	if (box[BOXBOTTOM] - circleradius > circley)
		return false;
	if (box[BOXTOP] + circleradius < circley)
		return false;
	return true;
}
