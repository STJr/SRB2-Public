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
/// \brief Random number LUT

#ifndef __M_RANDOM__
#define __M_RANDOM__

#include "doomtype.h"

// Returns a number from 0 to 255, from a lookup table.
byte M_Random(void);

#ifdef DEBUGRANDOM
#define P_Random() P_Random2(__FILE__, __LINE__)
#define P_SignedRandom() P_SignedRandom2(__FILE__, __LINE__)
byte P_Random2(char *a, int b);
int P_SignedRandom2(char *a, int b);
#else
// As M_Random, but used only by the play simulation.
byte P_Random(void);
int P_SignedRandom(void);
#endif

// Fix randoms for demos.
void M_ClearRandom(void);
byte P_GetRandIndex(void);
void P_SetRandIndex(byte newrindex);

#endif
