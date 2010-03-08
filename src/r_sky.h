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
/// \brief Sky rendering

#ifndef __R_SKY__
#define __R_SKY__

#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

/// \brief SKY, store the number for name.
#define SKYFLATNAME "F_SKY1"

/// \brief The sky map is 256*128*4 maps.
#define ANGLETOSKYSHIFT 22

extern INT32 skytexture, skytexturemid;
extern fixed_t skyscale;

extern INT32 skyflatnum;
extern INT32 levelskynum;
extern INT32 globallevelskynum;

// call after skytexture is set to adapt for old/new skies
void R_SetupSkyDraw(void);

void R_SetSkyScale(void);

#endif
