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
///
///	The SRB2 sky is a texture map like any
///	wall, wrapping around. A 1024 columns equal 360 degrees.
///	The default sky map is 256 columns and repeats 4 times
///	on a 320 screen.

#include "doomdef.h"
#include "doomstat.h"
#include "r_sky.h"
#include "r_local.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_maputl.h" // P_PointOnLineSide

//
// sky mapping
//

/**	\brief Needed to store the number of the dummy sky flat.
	Used for rendering, as well as tracking projectiles etc.
*/
INT32 skyflatnum;

/**	\brief the lump number of the sky texture
*/
INT32 skytexture;

/**	\brief the horizon line in a 256x128 sky texture
*/
INT32 skytexturemid;

/**	\brief the scale of the sky
*/
fixed_t skyscale;

/** \brief used for keeping track of the current sky
*/
INT32 levelskynum;
INT32 globallevelskynum;

/**	\brief	The R_SetupSkyDraw function

	Called at loadlevel after skytexture is set, or when sky texture changes.

	\warning wallcolfunc should be set at R_ExecuteSetViewSize()
	I don't bother because we don't use low detail anymore

	\return	void
*/
void R_SetupSkyDraw(void)
{
	// the horizon line in a 256x128 sky texture
	skytexturemid = (textures[skytexture]->height/2)<<FRACBITS;

	// get the right drawer, it was set by screen.c, depending on the
	// current video mode bytes per pixel (quick fix)
	wallcolfunc = walldrawerfunc;

	R_SetSkyScale();
}

/**	\brief	The R_SetSkyScale function

	set the correct scale for the sky at setviewsize

	\return void
*/
void R_SetSkyScale(void)
{
	skyscale = FixedDiv(FRACUNIT/2, (((vid.height*viewwidth)/vid.width)<<FRACBITS)/BASEVIDHEIGHT)<<1;
}
