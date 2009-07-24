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

#ifndef __M_BBOX__
#define __M_BBOX__

#include "m_fixed.h"

/**	\brief	Bounding box coordinate storage
*/

enum
{
	BOXTOP, /// top side of bbox
	BOXBOTTOM, /// bottom side of bbox
	BOXLEFT, /// left side of bbox
	BOXRIGHT /// right side of bbox
}; /// bbox coordinates

// Bounding box functions.
void M_ClearBox(fixed_t *box);

void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y);
boolean M_PointInBox(fixed_t *box, fixed_t x, fixed_t y);
boolean M_CircleTouchBox(fixed_t *box, fixed_t circlex, fixed_t circley, fixed_t circleradius);

#endif
