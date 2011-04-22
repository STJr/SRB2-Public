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
#include "m_fixed.h"

// M_Random functions pull random numbers of various types that aren't network synced.
// P_Random functions pulls random bytes from a 65535 byte lookup table that is network synced.

// Returns a number from 0 to 255, from rand()
// pulls a random byte like P_Random() but not synced across network.
UINT8 M_Random(void);
INT32 M_SignedRandom(void);
INT32 M_RandomRange(INT32 a, INT32 b);
// Pulls a random fixed_t, not synced across network.
fixed_t M_RandomPrecip(void);

#ifdef DEBUGRANDOM
#define P_Random() P_Random2(__FILE__, __LINE__)
#define P_SignedRandom() P_SignedRandom2(__FILE__, __LINE__)
UINT8 P_Random2(const char *a, INT32 b);
INT32 P_SignedRandom2(const char *a, INT32 b);
#else
UINT8 P_Random(void);
INT32 P_SignedRandom(void);
#endif

void M_ClearRandom(void);
UINT8 P_GetRandIndex(void);
void P_SetRandIndex(UINT8 newrindex);

#endif
