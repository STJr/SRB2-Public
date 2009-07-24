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
/// \brief AutoMap module

#ifndef __AMMAP_H__
#define __AMMAP_H__

#include "d_event.h"

typedef struct
{
	int x, y;
} fpoint_t;

typedef struct
{
	fpoint_t a, b;
} fline_t;

// Used by ST StatusBar stuff.
#define AM_MSGHEADER (('a'<<24)+('m'<<16))
#define AM_MSGENTERED (AM_MSGHEADER | ('e'<<8))
#define AM_MSGEXITED (AM_MSGHEADER | ('x'<<8))

extern boolean am_recalc; // true if screen size changes
extern boolean automapactive; // In AutoMap mode?

// Called by main loop.
boolean AM_Responder(event_t *ev);

// Called by main loop.
void AM_Ticker(void);

// Called by main loop, instead of view drawer if automap is active.
void AM_Drawer(void);

// Called to force the automap to quit if the level is completed while it is up.
void AM_Stop(void);

#endif
