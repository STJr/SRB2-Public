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

#include "doomdef.h"
#include "doomtype.h"
#include "m_random.h"

/**	\brief M_Random

	Returns a 0-255 number
*/

static byte rndtable[256] =
{
	0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249
};
/**	\brief M_Random seed
*/

static byte rndindex = 0;
/**	\brief P_Random seed
*/

static byte prndindex = 0;

#ifndef DEBUGRANDOM

/** Provides a random byte.
  * Used throughout all the p_xxx game code.
  *
  * \return A random byte, 0 to 255.
  * \sa P_SignedRandom, M_Random
  */
byte P_Random(void)
{
	return rndtable[++prndindex];
}

/** Provides a random number from -255 to 255.
  * Back in the day, a lot of code used P_Random() - P_Random() for this. Since
  * C doesn't define evaluation order, it was compiler dependent, so this
  * function allows network play between different compilers.
  *
  * \return A random number, -255 to 255.
  * \sa P_Random
  */
int P_SignedRandom(void)
{
	int r = P_Random();
	return r - P_Random();
}

#else

byte P_Random2(char *a, int b)
{
	CONS_Printf("P_Random at: %sp %d\n", a, b);
	return rndtable[++prndindex];
}

int P_SignedRandom2(char *a, int b)
{
	int r;
	CONS_Printf("P_SignedRandom at: %sp %d\n",a,b);
	r = rndtable[++prndindex];
	return r - rndtable[++prndindex];
}

#endif

/** Provides a random byte.
  * Used outside the p_xxx game code and not synchronized in netgames. This is
  * for anything that doesn't need to be synced. In practice many applications
  * just use rand() instead, e.g. precipitation.
  *
  * \return A random byte, 0 to 255.
  * \sa P_Random
  * \todo Consider replacing in favor of rand().
  */
byte M_Random(void)
{
	return rndtable[++rndindex];
}

/** Resets both the random number indices.
  * Used when loading savegames (not net savegames).
  *
  * \sa M_Random, P_Random
  */
void M_ClearRandom(void)
{
	rndindex = prndindex = 0;
}


/** Returns the current random number index used by p_xxx game code.
  * Used by servers when archiving netgames, as well as for debugging purposes.
  *
  * \return Current random index.
  * \sa P_SetRandIndex
  * \author Graue <graue@oceanbase.org>
  */
byte P_GetRandIndex(void)
{
	return prndindex;
}

/** Sets the random number index for p_xxx game code.
  * Used in order to make random events less predictable. On a level load in a
  * local game, the index is set with a value derived from ::totalplaytime. In
  * a netgame, the server sends an ::XD_RANDOMSEED message with a value derived
  * from its ::totalnetplaytime, and that value ends up here.
  *
  * \param rindex New random index.
  * \sa P_GetRandIndex
  * \author Graue <graue@oceanbase.org>
  */
void P_SetRandIndex(byte newrindex)
{
	prndindex = newrindex;
}
