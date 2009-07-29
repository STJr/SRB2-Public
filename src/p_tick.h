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
/// \brief Thinkers, Tickers

#ifndef __P_TICK__
#define __P_TICK__

#ifdef __GNUG__
#pragma interface
#endif

extern tic_t leveltime;

// Called by G_Ticker. Carries out all thinking of enemies and players.
void Command_Numthinkers_f(void);
void Command_CountMobjs_f(void);

void P_Ticker(void);
void P_DoTeamscrambling(void);
void P_RemoveThinkerDelayed(void *pthinker); //killed
mobj_t *P_SetTarget(mobj_t **mo, mobj_t *target);   // killough 11/98

#endif
