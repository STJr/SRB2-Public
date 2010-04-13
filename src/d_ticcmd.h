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
/// \brief Button/action code definitions, ticcmd_t

#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "m_fixed.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

// Button/action code definitions.
typedef enum
{
	// First 3 bits are weapon change info, DO NOT USE!
	BT_WEAPONMASK   = (1+2+4), //our first three bits.
	BT_UNUSED       =   8,

	BT_WEAPONNEXT   =  16,
	BT_WEAPONPREV   =  32,

	BT_ATTACK     =   256, // shoot rings
	BT_USE        =   512, // spin
	BT_TAUNT      =  1024,
	BT_CAMLEFT    =  2048, // turn camera left
	BT_CAMRIGHT   =  4096, // turn camera right
	BT_TOSSFLAG   =  8192,
	BT_JUMP       = 16384,
	BT_FIRENORMAL = 32768, // Fire a normal ring no matter what
} buttoncode_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1
#define TICCMD_XY 2

#if defined(_MSC_VER)
#pragma pack(1)
#endif

typedef struct
{
	SINT8 forwardmove; // *2048 for move
	SINT8 sidemove; // *2048 for move
	INT16 angleturn; // <<16 for angle delta - SAVED AS A BYTE into demos
	INT16 aiming; // mouse aiming, see G_BuildTicCmd
	UINT16 buttons;
} ATTRPACK ticcmd_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

#endif
