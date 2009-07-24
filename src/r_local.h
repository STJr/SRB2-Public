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
/// \brief Refresh (R_*) module, global header. All the rendering/drawing stuff is here

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Screen size related parameters.
#include "doomdef.h"

// Binary Angles, sine/cosine/atan lookups.
#include "tables.h"

// this one holds the max vid sizes and standard aspect
#include "screen.h"

#include "m_bbox.h"

#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_sky.h"
#include "r_data.h"
#include "r_things.h"
#include "r_draw.h"

extern drawseg_t *firstseg;

void SplitScreen_OnChange(void);

#endif // __R_LOCAL__
