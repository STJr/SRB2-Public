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
/// \brief Teleportation

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "r_main.h"

/**	\brief	The P_MixUp function

	\param	thing	mobj_t to mix up
	\param	x	new x pos
	\param	y	new y pos
	\param	z	new y pos
	\param	angle	new angle to look at

	\return	void


*/
void P_MixUp(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle)
{
	// the move is ok,
	// so link the thing into its new position
	P_UnsetThingPosition(thing);

	// Remove touching_sectorlist from mobj.
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}

	thing->x = x;
	thing->y = y;
	thing->z = z;

	if (thing->player)
	{
		if (thing->eflags & MFE_VERTICALFLIP)
			thing->player->viewz = thing->z + thing->height - thing->player->viewheight;
		else
			thing->player->viewz = thing->z + thing->player->viewheight;

		if (!thing->tracer)
			thing->reactiontime = TICRATE/2; // don't move for about half a second

		// absolute angle position
		if (thing == players[consoleplayer].mo)
			localangle = angle;
		if (splitscreen && thing == players[secondarydisplayplayer].mo)
			localangle2 = angle;

		// move chasecam at new player location
		if (splitscreen && cv_chasecam2.value
			&& thing->player == &players[secondarydisplayplayer])
		{
			P_ResetCamera(thing->player, &camera2);
		}
		else if (cv_chasecam.value && thing->player == &players[displayplayer])
			P_ResetCamera(thing->player, &camera);

		// don't run in place after a teleport
		thing->player->cmomx = thing->player->cmomy = 0;
		thing->player->rmomx = thing->player->rmomy = 0;
		if (!thing->tracer)
			thing->player->speed = 0;

		P_ResetPlayer(thing->player);
		P_SetPlayerMobjState(thing, S_PLAY_STND);

		thing->player->bonuscount = 10; // flash the palette
	}

	thing->angle = angle;

	thing->momx = thing->momy = thing->momz = 0;
}

/**	\brief	The P_Teleport function

	\param	thing	mobj_t to teleport
	\param	x	new x pos
	\param	y	new y pos
	\param	z	new y pos
	\param	angle	new angle to look at

	\return	if true, the thing "teleported"


*/
boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle, boolean flash, boolean dontstopmove)
{
	if (!P_TeleportMove(thing, x, y, z))
		return false;

	thing->angle = angle;

	if (!dontstopmove)
		thing->momx = thing->momy = thing->momz = 0;
	else // Change speed to match direction
		P_InstaThrust(thing, thing->angle, P_AproxDistance(thing->momx, thing->momy));

	if (thing->player)
	{
		if (thing->eflags & MFE_VERTICALFLIP)
			thing->player->viewz = thing->z + thing->height - thing->player->viewheight;
		else
			thing->player->viewz = thing->z + thing->player->viewheight;

		if (!dontstopmove)
			thing->reactiontime = TICRATE/2; // don't move for about half a second

		// absolute angle position
		if (thing->player == &players[consoleplayer])
			localangle = angle;
		if (splitscreen && thing->player == &players[secondarydisplayplayer])
			localangle2 = angle;

		// move chasecam at new player location
		if (splitscreen && cv_chasecam2.value
			&& thing->player == &players[secondarydisplayplayer])
		{
			P_ResetCamera(thing->player, &camera2);
		}
		else if (cv_chasecam.value && thing->player == &players[displayplayer])
			P_ResetCamera(thing->player, &camera);

		// don't run in place after a teleport
		if (!dontstopmove)
		{
			thing->player->cmomx = thing->player->cmomy = 0;
			thing->player->rmomx = thing->player->rmomy = 0;
			thing->player->speed = 0;
			P_ResetPlayer(thing->player);
			P_SetPlayerMobjState(thing, S_PLAY_STND);
		}

		if (flash)
			thing->player->bonuscount = 10; // flash the palette
	}

	return true;
}
