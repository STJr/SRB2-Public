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
/// \brief Movement, collision handling
///
///	Shooting and aiming

#include "doomdef.h"
#include "g_game.h"
#include "m_bbox.h"
#include "m_random.h"
#include "p_local.h"
#include "r_state.h"
#include "r_main.h"
#include "r_sky.h"
#include "s_sound.h"
#include "w_wad.h"

#include "r_splats.h"

#include "z_zone.h"

fixed_t tmbbox[4];
mobj_t *tmthing;
static INT32 tmflags;
static fixed_t tmx;
static fixed_t tmy;

static precipmobj_t *tmprecipthing;
static fixed_t preciptmx;
static fixed_t preciptmy;
static fixed_t preciptmbbox[4];
static INT32 preciptmflags;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean floatok;

fixed_t tmfloorz, tmceilingz;
static fixed_t tmdropoffz;
mobj_t *tmfloorthing; // the thing corresponding to tmfloorz or NULL if tmfloorz is from a sector

// used at P_ThingHeightClip() for moving sectors
static fixed_t tmsectorfloorz;
fixed_t tmsectorceilingz;

// turned on or off in PIT_CheckThing
boolean tmsprung;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t *ceilingline;

// set by PIT_CheckLine() for any line that stopped the PIT_CheckLine()
// that is, for any line which is 'solid'
line_t *blockingline;

msecnode_t *sector_list = NULL;
mprecipsecnode_t *precipsector_list = NULL;
static camera_t *mapcampointer;

//
// TELEPORT MOVE
//

//
// P_TeleportMove
//
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z)
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

	P_SetThingPosition(thing);

	P_CheckPosition(thing, thing->x, thing->y);

	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;

	return true;
}

// =========================================================================
//                       MOVEMENT ITERATOR FUNCTIONS
// =========================================================================

static void P_DoSpring(mobj_t *spring, mobj_t *object)
{
	// Spectators don't trigger springs.
	if (object->player && object->player->spectator)
		return;

	if (object->player && (object->player->pflags & PF_NIGHTSMODE))
	{
		/*Someone want to make these work like bumpers?*/
		return;
	}

	spring->flags &= ~MF_SOLID; // De-solidify

	if (spring->info->damage) // Mimic SA
	{
		object->momx = object->momy = 0;
		P_UnsetThingPosition(object);
		object->x = spring->x;
		object->y = spring->y;
		P_SetThingPosition(object);
	}

	if (spring->info->speed > 0)
		object->z = spring->z + spring->height + 1;
	else
		object->z = spring->z - object->height - 1;

	// You could have a non-ceiling spring flipped upside down, if you really wanted to...
	if (!(spring->eflags & MFE_VERTICALFLIP))
		object->momz = FIXEDSCALE(spring->info->speed,(object->scale+spring->scale)/2);
	else
	{
		object->momz = FIXEDSCALE(-(spring->info->speed),(object->scale+spring->scale)/2);
		object->z -= FIXEDSCALE(spring->height, spring->scale) + object->height;
	}

	object->momz = FIXEDSCALE(object->momz, spring->scale);

	P_TryMove(object, object->x, object->y, true);

	if (spring->info->damage)
		P_InstaThrustEvenIn2D(object, spring->angle, FIXEDSCALE(spring->info->damage,(object->scale+spring->scale)/2));

	P_SetMobjState(spring, spring->info->seestate);

	spring->flags |= MF_SOLID; // Re-solidify
	if (object->player)
	{
		if (spring->info->damage && !(object->player->cmd.forwardmove != 0 || object->player->cmd.sidemove != 0))
		{
			object->player->mo->angle = spring->angle;

			if (object->player == &players[consoleplayer])
				localangle = spring->angle;
			else if (splitscreen && object->player == &players[secondarydisplayplayer])
				localangle2 = spring->angle;
		}

		P_ResetPlayer(object->player);

		if (spring->info->speed > 0)
			P_SetPlayerMobjState(object, S_PLAY_PLG1);
		else
			P_SetPlayerMobjState(object, S_PLAY_FALL1);

		if (spring->info->painchance)
		{
			object->player->pflags |= PF_JUMPED;
			P_SetPlayerMobjState(object, S_PLAY_ATK1);
		}
	}
}

//
// PIT_CheckThing
//
static boolean PIT_CheckThing(mobj_t *thing)
{
	fixed_t blockdist, topz, tmtopz;

	// don't clip against self
	tmsprung = false;

	// Ignore spectators, except when using a spring.
	if ((tmthing->player && tmthing->player->spectator && !(thing->flags & MF_SPRING)) ||
		(thing->player && thing->player->spectator))
		return true;

	if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
		return true;

	if (!tmthing || !thing || thing == tmthing || !thing->state || thing->state == &states[S_DISS])
		return true;

	// Don't collide with your buddies while NiGHTS-flying.
	if (tmthing->player && thing->player && (maptol & TOL_NIGHTS)
		&& ((tmthing->player->pflags & PF_NIGHTSMODE) || (thing->player->pflags & PF_NIGHTSMODE)))
		return true;

	blockdist = thing->radius + tmthing->radius;

	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
		return true; // didn't hit it

	if (thing->type == MT_HOOPCOLLIDE)
	{
		if (thing->flags & MF_SPECIAL)
			P_TouchSpecialThing(thing, tmthing, true);
		return true;
	}
	if (tmthing->type == MT_HOOPCOLLIDE)
	{
		if (tmthing->flags & MF_SPECIAL)
			P_TouchSpecialThing(tmthing, thing, true);
		return true;
	}

	// check for skulls slamming into things
	if (tmthing->flags2 & MF2_SKULLFLY)
	{
		if (tmthing->type == MT_EGGMOBILE) // Don't make Eggman stop!
		{
			tmthing->momx = -tmthing->momx;
			tmthing->momy = -tmthing->momy;
			tmthing->momz = -tmthing->momz;
		}
		else
		{
			tmthing->flags2 &= ~MF2_SKULLFLY;
			tmthing->momx = tmthing->momy = tmthing->momz = 0;
			return false; // stop moving
		}
	}

	// missiles can hit other things
	if (tmthing->flags & MF_MISSILE || tmthing->type == MT_SHELL || tmthing->type == MT_FIREBALL)
	{
		// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return true; // overhead
		if (tmthing->z + tmthing->height < thing->z)
			return true; // underneath

		if (tmthing->type != MT_SHELL && tmthing->target && tmthing->target->type == thing->type)
		{
			// Don't hit same species as originator.
			if (thing == tmthing->target)
				return true;

			if (thing->type != MT_PLAYER)
			{
				// Explode, but do no damage.
				// Let players missile other players.
				return false;
			}
		}

		// Special case for bounce rings so they don't get caught behind solid objects.
		if ((tmthing->type == MT_THROWNBOUNCE && tmthing->fuse > 8*TICRATE) && thing->flags & MF_SOLID)
			return true;

		if (tmthing->type == MT_BLACKEGGMAN_MISSILE
			&& thing->type == MT_BLACKEGGMAN
			&& tmthing->target != thing)
		{
			P_SetMobjState(thing, thing->info->painstate);
			return false;
		}

		if (!(thing->flags & MF_SHOOTABLE))
		{
			// didn't do any damage
			return !(thing->flags & MF_SOLID);
		}

		if (tmthing->type == MT_SHELL && tmthing->threshold > TICRATE)
			return true;
#ifdef SEENAMES
		if (tmthing->type == MT_NAMECHECK)
		{
			if (thing->player && tmthing->target->player && thing->player != tmthing->target->player) // Don't hit yourself
				if (!thing->player->spectator) // Can't see spectators
					seenplayer = thing->player;
			return false;
		}
#endif
		// damage / explode
		if (tmthing->flags & MF_ENEMY) // An actual ENEMY! (Like the deton, for example)
			P_DamageMobj(thing, tmthing, tmthing, 1);
		else if (tmthing->type == MT_BLACKEGGMAN_MISSILE && thing->player
			&& (thing->player->pflags & PF_JUMPED)
			&& !thing->player->powers[pw_flashing]
			&& thing->tracer != tmthing
			&& tmthing->target != thing)
		{
			// Hop on the missile for a ride!
			thing->player->pflags |= PF_ITEMHANG;
			thing->player->pflags &= ~PF_JUMPED;
			P_SetTarget(&thing->tracer, tmthing);
			P_SetTarget(&tmthing->target, thing); // Set owner to the player
			P_SetTarget(&tmthing->tracer, NULL); // Disable homing-ness
			tmthing->momz = 0;

			thing->angle = tmthing->angle;

			if (thing->player == &players[consoleplayer])
				localangle = thing->angle;
			else if (thing->player == &players[secondarydisplayplayer])
				localangle2 = thing->angle;

			return true;
		}
		else if (tmthing->type == MT_BLACKEGGMAN_MISSILE && thing->player && ((thing->player->pflags & PF_ITEMHANG) || (thing->player->pflags & PF_JUMPED)))
		{
			// Ignore
		}
		else if (tmthing->type == MT_BLACKEGGMAN_GOOPFIRE)
		{
			P_UnsetThingPosition(tmthing);
			tmthing->x = thing->x;
			tmthing->y = thing->y;
			P_SetThingPosition(tmthing);
		}
		else
			P_DamageMobj(thing, tmthing, tmthing->target, 1);

		// don't traverse any more

		if (tmthing->type == MT_SHELL)
			return true;
		else
			return false;
	}

	if ((thing->flags & MF_PUSHABLE) && (tmthing->player || tmthing->flags & MF_PUSHABLE)
		&& tmthing->z + tmthing->height > thing->z && tmthing->z < thing->z + thing->height
		&& !(netgame && tmthing->player && tmthing->player->spectator)) // Push thing!
	{
		if (thing->flags2 & MF2_SLIDEPUSH) // Make it slide
		{
			if (tmthing->momy > 0 && tmthing->momy > 4*FRACUNIT && tmthing->momy > thing->momy)
			{
				thing->momy += PUSHACCEL;
				tmthing->momy -= PUSHACCEL;
			}
			else if (tmthing->momy < 0 && tmthing->momy < -4*FRACUNIT
				&& tmthing->momy < thing->momy)
			{
				thing->momy -= PUSHACCEL;
				tmthing->momy += PUSHACCEL;
			}
			if (tmthing->momx > 0 && tmthing->momx > 4*FRACUNIT
				&& tmthing->momx > thing->momx)
			{
				thing->momx += PUSHACCEL;
				tmthing->momx -= PUSHACCEL;
			}
			else if (tmthing->momx < 0 && tmthing->momx < -4*FRACUNIT
				&& tmthing->momx < thing->momx)
			{
				thing->momx -= PUSHACCEL;
				tmthing->momx += PUSHACCEL;
			}

			if (thing->momx > thing->info->speed)
				thing->momx = thing->info->speed;
			else if (thing->momx < -(thing->info->speed))
				thing->momx = -(thing->info->speed);
			if (thing->momy > thing->info->speed)
				thing->momy = thing->info->speed;
			else if (thing->momy < -(thing->info->speed))
				thing->momy = -(thing->info->speed);
		}
		else
		{
			if (tmthing->momy > 0 && tmthing->momy > 4*FRACUNIT)
				tmthing->momy = 4*FRACUNIT;
			else if (tmthing->momy < 0 && tmthing->momy < -4*FRACUNIT)
				tmthing->momy = -4*FRACUNIT;
			if (tmthing->momx > 0 && tmthing->momx > 4*FRACUNIT)
				tmthing->momx = 4*FRACUNIT;
			else if (tmthing->momx < 0 && tmthing->momx < -4*FRACUNIT)
				tmthing->momx = -4*FRACUNIT;

			thing->momx = tmthing->momx;
			thing->momy = tmthing->momy;
		}

		S_StartSound(thing, thing->info->activesound);

		P_SetTarget(&thing->target, tmthing);
	}

	if (tmthing->type == MT_SPIKEBALL && thing->player)
		P_TouchSpecialThing(tmthing, thing, true);
	else if (thing->type == MT_SPIKEBALL && tmthing->player)
		P_TouchSpecialThing(thing, tmthing, true);

	// check for special pickup
	if (thing->flags & MF_SPECIAL)
	{
		if (tmthing->player)
		{
			P_TouchSpecialThing(thing, tmthing, true); // can remove thing
		}
		return true;
	}
	// check again for special pickup
	if (tmthing->flags & MF_SPECIAL)
	{
		if (thing->player)
		{
			P_TouchSpecialThing(tmthing, thing, true); // can remove thing
		}
		return true;
	}

	if (tmthing->type == MT_EGGGUARD && thing->player)
		return true;

	// Sprite Spikes!
	if (tmthing->type == MT_CEILINGSPIKE)
	{
		if (thing->z + thing->height == tmthing->z && thing->momz >= 0)
			P_DamageMobj(thing, tmthing, tmthing, 1); // Ouch!
	}
	else if (thing->type == MT_CEILINGSPIKE)
	{
		if (tmthing->z + tmthing->height == thing->z && tmthing->momz >= 0)
			P_DamageMobj(tmthing, thing, thing, 1);
	}
	else if (tmthing->type == MT_FLOORSPIKE)
	{
		if (thing->z == tmthing->z + tmthing->height + FRACUNIT && thing->momz <= 0)
		{
			tmthing->threshold = 43;
			P_DamageMobj(thing, tmthing, tmthing, 1);
		}
	}
	else if (thing->type == MT_FLOORSPIKE)
	{
		if (tmthing->z == thing->z + thing->height + FRACUNIT
			&& tmthing->momz <= 0)
		{
			thing->threshold = 43;
			P_DamageMobj(tmthing, thing, thing, 1);
		}
	}

	if ((tmthing->flags & MF_PUSHABLE) && tmthing->z <= (thing->z + thing->height + FRACUNIT)
		&& (tmthing->z + tmthing->height) >= thing->z)
	{
		if (thing->flags & MF_SPRING)
		{
			if (tmthing->player && (tmthing->player->pflags & PF_NIGHTSMODE));
			else
			{
				P_DoSpring(thing, tmthing);
				tmsprung = true;
			}
		}
	}

	// Damage other players when invincible
	if (tmthing->player && thing->player
	// Make sure they aren't able to damage you ANYWHERE along the Z axis, you have to be TOUCHING the person.
		&& !(thing->z + thing->height < tmthing->z || thing->z > tmthing->z + tmthing->height))
	{
		if ((gametype == GT_MATCH && (!cv_matchtype.value))
			|| ((gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value)) && tmthing->player->ctfteam != thing->player->ctfteam)
			|| (gametype == GT_RACE))
		{
			if ((tmthing->player->powers[pw_invulnerability] || tmthing->player->powers[pw_super])
				&& !thing->player->powers[pw_super])
				P_DamageMobj(thing, tmthing, tmthing, 1);
			else if ((thing->player->powers[pw_invulnerability] || thing->player->powers[pw_super])
				&& !tmthing->player->powers[pw_super])
				P_DamageMobj(tmthing, thing, thing, 1);
		}

		// If players are using touch tag, seekers damage hiders.
		if (gametype == GT_TAG && cv_touchtag.value &&
			((thing->player->pflags & PF_TAGIT) != (tmthing->player->pflags & PF_TAGIT)))
		{
			if ((tmthing->player->pflags & PF_TAGIT) && !(thing->player->pflags & PF_TAGIT))
				P_DamageMobj(thing, tmthing, tmthing, 1);
			else if ((thing->player->pflags & PF_TAGIT) && !(tmthing->player->pflags & PF_TAGIT))
				P_DamageMobj(tmthing, thing, tmthing, 1);
		}
	}

	// Force solid players in hide and seek to avoid corner stacking.
	if (cv_tailspickup.value && !(gametype == GT_TAG && cv_tagtype.value))
	{
		if (tmthing->player && thing->player)
		{
			if ((tmthing->player->pflags & PF_CARRIED) && tmthing->tracer == thing)
				return true;
			else if ((thing->player->pflags & PF_CARRIED) && thing->tracer == tmthing)
				return true;
			else if (tmthing->player->powers[pw_tailsfly]
					|| (tmthing->player->charability == CA_FLY && (tmthing->state == &states[S_PLAY_SPC1] || tmthing->state == &states[S_PLAY_SPC2] || tmthing->state == &states[S_PLAY_SPC3] || tmthing->state == &states[S_PLAY_SPC4])))
			{
				if (thing->player->pflags & PF_NIGHTSMODE)
					return true;

				if ((thing->eflags & MFE_VERTICALFLIP) && !(tmthing->eflags & MFE_VERTICALFLIP))
					return true;

				if (!(thing->eflags & MFE_VERTICALFLIP) && (tmthing->eflags & MFE_VERTICALFLIP))
					return true;

				if ((!(tmthing->eflags & MFE_VERTICALFLIP) && (tmthing->z <= thing->z + thing->height + FRACUNIT)
					&& tmthing->z > thing->z + thing->height*2/3
					&& thing->momz <= 0)
					|| ((tmthing->eflags & MFE_VERTICALFLIP) && (tmthing->z + tmthing->height >= thing->z - FRACUNIT)
					&& tmthing->z + tmthing->height < thing->z + thing->height - thing->height*2/3
					&& thing->momz >= 0))
				{
					if (gametype == GT_RACE
						|| (netgame && (tmthing->player->spectator || thing->player->spectator))
						|| (gametype == GT_TAG && (!(tmthing->player->pflags & PF_TAGIT) != !(thing->player->pflags & PF_TAGIT)))
						|| (gametype == GT_MATCH && !cv_matchtype.value)
						|| ((gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value)) && tmthing->player->ctfteam != thing->player->ctfteam))
						thing->player->pflags &= ~PF_CARRIED;
					else
					{
						P_ResetPlayer(thing->player);
						P_ResetScore(thing->player);
						P_SetTarget(&thing->tracer, tmthing);
						thing->player->pflags |= PF_CARRIED;
						S_StartSound(thing->player, sfx_s3k_25);
						P_UnsetThingPosition(thing);
						if (sector_list)
						{
							P_DelSeclist(sector_list);
							sector_list = NULL;
						}
						thing->x = tmthing->x;
						thing->y = tmthing->y;
						P_SetThingPosition(thing);
					}
				}
				else
					thing->player->pflags &= ~PF_CARRIED;
			}
			else
				thing->player->pflags &= ~PF_CARRIED;

			return true;
		}
	}
	else if (thing->player)
		thing->player->pflags &= ~PF_CARRIED;

	if (thing->player)
	{
		if (thing->eflags & MFE_VERTICALFLIP && tmthing->eflags & MFE_VERTICALFLIP)
		{
			// Objects kill you if it falls from above.
			if (tmthing->z + tmthing->height + tmthing->momz >= thing->z
				&& tmthing->z + tmthing->height + tmthing->momz < thing->z + thing->height
				&& P_IsObjectOnGround(thing))
			{
				if ((tmthing->flags & MF_MONITOR) || (tmthing->flags & MF_PUSHABLE))
				{
					if (thing != tmthing->target)
						P_DamageMobj(thing, tmthing, tmthing->target, 10000);

					tmthing->momz = -tmthing->momz/2; // Bounce, just for fun!
					// The tmthing->target allows the pusher of the object
					// to get the point if he topples it on an opponent.
				}
			}

			if (thing->z + thing->height <= tmthing->z + tmthing->height && !(thing->state == &states[thing->info->painstate])) // Stuff where da player don't gotta move
			{
				switch (tmthing->type)
				{
					case MT_FAN: // fan
						if (thing->z + thing->height >= tmthing->z + tmthing->height - (tmthing->health << FRACBITS) && thing->momz > -tmthing->info->speed && !(thing->player->climbing || (thing->player->pflags & PF_GLIDING)))
						{
							thing->momz -= tmthing->info->speed/4;

							if (thing->momz < -tmthing->info->speed)
								thing->momz = -tmthing->info->speed;

							if (!thing->player->powers[pw_tailsfly])
							{
								P_ResetPlayer(thing->player);
								if (!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
									P_SetPlayerMobjState(thing, S_PLAY_FALL1);
							}
						}
						break;
					case MT_STEAM: // Steam
						if (tmthing->state == &states[S_STEAM1] && thing->z + thing->height >= tmthing->z + tmthing->height - 16*FRACUNIT) // Only when it bursts
						{
							thing->momz = -tmthing->info->speed;
							P_ResetPlayer(thing->player);
							if (!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
								P_SetPlayerMobjState(thing, S_PLAY_FALL1);
						}
						break;
					default:
						break;
				}
			}
		}
		else
		{
			// Objects kill you if it falls from above.
			if (tmthing->z + tmthing->momz <= thing->z + thing->height
				&& tmthing->z + tmthing->momz > thing->z
				&& P_IsObjectOnGround(thing)
				&& (tmthing->flags & MF_SOLID))
			{
				if ((tmthing->flags & MF_MONITOR) || (tmthing->flags & MF_PUSHABLE))
				{
					if (thing != tmthing->target)
						P_DamageMobj(thing, tmthing, tmthing->target, 10000);

					tmthing->momz = -tmthing->momz/2; // Bounce, just for fun!
					// The tmthing->target allows the pusher of the object
					// to get the point if he topples it on an opponent.
				}
			}

			if (thing->z >= tmthing->z && !(thing->state == &states[thing->info->painstate])) // Stuff where da player don't gotta move
			{
				switch (tmthing->type)
				{
					case MT_FAN: // fan
						if (thing->z <= tmthing->z + (tmthing->health << FRACBITS) && thing->momz < tmthing->info->speed && !(thing->player->climbing || (thing->player->pflags & PF_GLIDING)))
						{
							thing->momz += tmthing->info->speed/4;

							if (thing->momz > tmthing->info->speed)
								thing->momz = tmthing->info->speed;

							if (!thing->player->powers[pw_tailsfly])
							{
								P_ResetPlayer(thing->player);
								if (!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
									P_SetPlayerMobjState(thing, S_PLAY_FALL1);
							}
						}
						break;
					case MT_STEAM: // Steam
						if (tmthing->state == &states[S_STEAM1] && thing->z <= tmthing->z + 16*FRACUNIT) // Only when it bursts
						{
							thing->momz = tmthing->info->speed;
							P_ResetPlayer(thing->player);
							if (!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
								P_SetPlayerMobjState(thing, S_PLAY_FALL1);
						}
						break;
					default:
						break;
				}
			}
		}
	}

	if (tmthing->player) // Is the moving/interacting object the player?
	{
		if (tmthing->z >= thing->z && !(tmthing->state == &states[tmthing->info->painstate]))
		{
			switch (thing->type)
			{
				case MT_FAN: // fan
					if (tmthing->z <= thing->z + (thing->health << FRACBITS) && tmthing->momz < thing->info->speed && !(tmthing->player->climbing || (tmthing->player->pflags & PF_GLIDING)))
					{
						tmthing->momz += thing->info->speed/4;

						if (tmthing->momz > thing->info->speed)
							tmthing->momz = thing->info->speed;

						if (!tmthing->player->powers[pw_tailsfly])
						{
							P_ResetPlayer(tmthing->player);
							if (!(tmthing->state == &states[S_PLAY_FALL1] || tmthing->state == &states[S_PLAY_FALL2]))
								P_SetPlayerMobjState(tmthing, S_PLAY_FALL1);
						}
					}
					break;
				case MT_STEAM: // Steam
					if (thing->state == &states[S_STEAM1] && tmthing->z <= thing->z + 16*FRACUNIT) // Only when it bursts
					{
						tmthing->momz = thing->info->speed;
						P_ResetPlayer(tmthing->player);
						if (!(tmthing->state == &states[S_PLAY_FALL1] || tmthing->state == &states[S_PLAY_FALL2]))
							P_SetPlayerMobjState(tmthing, S_PLAY_FALL1);
					}
					break;
				default:
					break;
			}
		}

		// Are you touching the side of the object you're interacting with?
		if ((thing->z <= tmthing->z + tmthing->height
			&& thing->z + thing->height >= tmthing->z)
			|| ((!(tmthing->eflags & MFE_VERTICALFLIP) && tmthing->z == thing->z + thing->height + FRACUNIT)
				|| ((tmthing->eflags & MFE_VERTICALFLIP) && tmthing->z + tmthing->height == thing->z - FRACUNIT)))
		{
			if (thing->flags & MF_SPRING)
			{
				if (!(tmthing->player && (tmthing->player->pflags & PF_NIGHTSMODE)))
				{
					P_DoSpring(thing, tmthing);
					tmsprung = true;
				}
			}
			else if (thing->flags & MF_MONITOR
				&& ((tmthing->player->pflags & PF_JUMPED) || (tmthing->player->pflags & PF_SPINNING)))
			{
				// Going down? Then bounce back up.
				if (tmthing->eflags & MFE_VERTICALFLIP)
				{
					if (tmthing->momz > 0)
						tmthing->momz = -tmthing->momz;
				}
				else
				{
					if (tmthing->momz < 0)
						tmthing->momz = -tmthing->momz;
				}

				P_DamageMobj(thing, tmthing, tmthing, 1); // break the monitor
			}
			else if (thing->flags & MF_BOSS
				&& ((tmthing->player->pflags & PF_JUMPED) || (tmthing->player->pflags & PF_SPINNING)
				|| tmthing->player->powers[pw_invulnerability]
				|| tmthing->player->powers[pw_super]))
			{
				// Going down? Then bounce back up.
				if (tmthing->eflags & MFE_VERTICALFLIP)
				{
					if (tmthing->momz > 0)
						tmthing->momz = -tmthing->momz;
				}
				else
				{
					if (tmthing->momz < 0)
						tmthing->momz = -tmthing->momz;
				}

				// Also, bounce back.
				tmthing->momx = -tmthing->momx;
				tmthing->momy = -tmthing->momy;
				P_DamageMobj(thing, tmthing, tmthing, 1); // fight the boss!
			}
		}
	}

	// compatibility with old demos, it used to return with...
	// for version 112+, nonsolid things pass through other things
	if (!(tmthing->flags & MF_SOLID))
		return !(thing->flags & MF_SOLID);

	// z checking at last
	// Treat noclip things as non-solid!
	if ((thing->flags & MF_SOLID) && (tmthing->flags & MF_SOLID) &&
		!(thing->flags & MF_NOCLIP) && !(tmthing->flags & MF_NOCLIP))
	{
		if (tmthing->eflags & MFE_VERTICALFLIP)
		{
			// pass under
			tmtopz = tmthing->z;

			if (tmtopz > thing->z + thing->height)
			{
				if (thing->z + thing->height > tmfloorz)
				{
					tmfloorz = thing->z + thing->height;
				}
				return true;
			}

			topz = thing->z - FRACUNIT;

			// block only when jumping not high enough,
			// (dont climb max. 24units while already in air)
			// if not in air, let P_TryMove() decide if it's not too high
			if (tmthing->player && tmthing->z + tmthing->height > topz
				&& tmthing->z + tmthing->height < tmthing->ceilingz)
				return false; // block while in air

			if (topz < tmceilingz && !(thing->flags & MF_SPRING))
			{
				tmceilingz = topz;
				tmfloorthing = thing; // thing we may stand on
			}
		}
		else
		{
			// pass under
			tmtopz = tmthing->z + tmthing->height;

			if (tmtopz < thing->z)
			{
				if (thing->z < tmceilingz)
				{
					tmceilingz = thing->z;
				}
				return true;
			}

			topz = thing->z + thing->height + FRACUNIT;

			// block only when jumping not high enough,
			// (dont climb max. 24units while already in air)
			// if not in air, let P_TryMove() decide if it's not too high
			if (tmthing->player && tmthing->z < topz && tmthing->z > tmthing->floorz)
				return false; // block while in air

			if (topz > tmfloorz && !(thing->flags & MF_SPRING))
			{
				tmfloorz = topz;
				tmfloorthing = thing; // thing we may stand on
			}
		}
	}

	// not solid not blocked
	return true;
}

// PIT_CheckCameraLine
// Adjusts tmfloorz and tmceilingz as lines are contacted - FOR CAMERA ONLY
static boolean PIT_CheckCameraLine(line_t *ld)
{
	if (ld->polyobj && !(ld->polyobj->flags & POF_SOLID))
		return true;

	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
		|| tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	{
		return true;
	}

	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;

	// A line has been hit

	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.

	// this line is out of the if so upper and lower textures can be hit by a splat
	blockingline = ld;
	if (!ld->backsector)
		return false; // one sided line

	// set openrange, opentop, openbottom
	P_CameraLineOpening(ld);

	// adjust floor / ceiling heights
	if (opentop < tmceilingz)
	{
		tmsectorceilingz = tmceilingz = opentop;
		ceilingline = ld;
	}

	if (openbottom > tmfloorz)
	{
		tmsectorfloorz = tmfloorz = openbottom;
	}

	if (lowfloor < tmdropoffz)
		tmdropoffz = lowfloor;

	return true;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
static boolean PIT_CheckLine(line_t *ld)
{
	if (ld->polyobj && !(ld->polyobj->flags & POF_SOLID))
		return true;

	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
		|| tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	{
		return true;
	}

	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;

	// A line has been hit

	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.

	// this line is out of the if so upper and lower textures can be hit by a splat
	blockingline = ld;
	if (!ld->backsector)
		return false; // one sided line

	// missiles can cross uncrossable lines
	if (!(tmthing->flags & MF_MISSILE))
	{
		if (((tmthing->flags & MF_ENEMY) || (tmthing->flags & MF_BOSS) || (tmthing->type == MT_EGGGUARD)) && ld->flags & ML_BLOCKMONSTERS)
			return false; // block monsters only
	}

	// set openrange, opentop, openbottom
	P_LineOpening(ld);

	// adjust floor / ceiling heights
	if (opentop < tmceilingz)
	{
		tmsectorceilingz = tmceilingz = opentop;
		ceilingline = ld;
	}

	if (openbottom > tmfloorz)
	{
		tmsectorfloorz = tmfloorz = openbottom;
	}

	if (lowfloor < tmdropoffz)
		tmdropoffz = lowfloor;

	return true;
}

// =========================================================================
//                         MOVEMENT CLIPPING
// =========================================================================

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  tmfloorz
//  tmceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//

// tmfloorz
//     the nearest floor or thing's top under tmthing
// tmceilingz
//     the nearest ceiling or thing's bottom over tmthing
//
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y)
{
	INT32 xl, xh, yl, yh, bx, by;
	subsector_t *newsubsec;
	boolean blockval = true;

	tmthing = thing;
	tmflags = thing->flags;

	tmx = x;
	tmy = y;

	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;

	newsubsec = R_PointInSubsector(x, y);
	ceilingline = blockingline = NULL;

	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;

	// Check list of fake floors and see if tmfloorz/tmceilingz need to be altered.
	if (newsubsec->sector->ffloors)
	{
		ffloor_t *rover;
		fixed_t delta1, delta2;
		INT32 thingtop = thing->z + thing->height;

		for (rover = newsubsec->sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS))
				continue;

			if (thing->player && (rover->flags & FF_SWIMMABLE) && GETSECSPECIAL(rover->master->frontsector->special, 1) == 3
				&& !(rover->master->flags & ML_BLOCKMONSTERS) && ((rover->master->flags & ML_EFFECT3)
				|| thing->z-thing->momz > *rover->topheight - 16*FRACUNIT))
				;
			else if (!((((rover->flags & FF_BLOCKPLAYER) && thing->player)
				|| ((rover->flags & FF_BLOCKOTHERS) && !thing->player)) || rover->flags & FF_QUICKSAND))
			{
				if (!(thing->player && !(thing->player->pflags & PF_NIGHTSMODE) && (((thing->player->charability == CA_SWIM) || thing->player->powers[pw_super]) && thing->ceilingz-*rover->topheight >= thing->height)
					&& !(thing->player->pflags & PF_SPINNING) && thing->player->speed > thing->player->runspeed
/*					&& thing->ceilingz - *rover->topheight >= thing->height*/
					&& thing->z < *rover->topheight + 30*FRACUNIT
					&& thing->z > *rover->topheight - 30*FRACUNIT
					&& (rover->flags & FF_SWIMMABLE))
					&& (!(thing->type == MT_SKIM && (rover->flags & FF_SWIMMABLE))))
					continue;
			}

			if (rover->flags & FF_QUICKSAND)
			{
				if (thing->z < *rover->topheight && *rover->bottomheight < thingtop)
				{
					tmfloorz = thing->z;
					continue;
				}
			}

			delta1 = thing->z - (*rover->bottomheight
				+ ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight
				+ ((*rover->topheight - *rover->bottomheight)/2));

			if (*rover->topheight > tmfloorz && abs(delta1) < abs(delta2)
				&& (!(rover->flags & FF_REVERSEPLATFORM)))
			{
				tmfloorz = tmdropoffz = *rover->topheight;
			}
			if (*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2)
				&& (/*thing->z + thing->height <= *rover->bottomheight
					|| */!(rover->flags & FF_PLATFORM))
				&& !(thing->type == MT_SKIM	&& (rover->flags & FF_SWIMMABLE)))
			{
				tmceilingz = *rover->bottomheight;
			}
		}
	}

#ifdef POLYOBJECTS
	// Check polyobjects and see if tmfloorz/tmceilingz need to be altered
	{
		validcount++;

		xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
		xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
		yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
		yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

		for (by = yl; by <= yh; by++)
			for (bx = xl; bx <= xh; bx++)
			{
				INT32 offset;
				polymaplink_t *plink; // haleyjd 02/22/06

				if (bx < 0 || by < 0 || bx >= bmapwidth || by >= bmapheight)
					continue;

				offset = by*bmapwidth + bx;

				// haleyjd 02/22/06: consider polyobject lines
				plink = polyblocklinks[offset];

				while (plink)
				{
					polyobj_t *po = plink->po;

					if (po->validcount != validcount) // if polyobj hasn't been checked
					{
						sector_t *polysec;
						fixed_t delta1, delta2, thingtop;
						fixed_t polytop, polybottom;

						po->validcount = validcount;

						if (!P_BBoxInsidePolyobj(po, tmbbox)
							|| !(po->flags & POF_SOLID))
						{
							plink = (polymaplink_t *)(plink->link.next);
							continue;
						}

						// We're inside it! Yess...
						polysec = po->lines[0]->backsector;

						if (po->flags & POF_CLIPPLANES)
						{
							polytop = polysec->ceilingheight;
							polybottom = polysec->floorheight;
						}
						else
						{
							polytop = INT32_MAX;
							polybottom = INT32_MIN;
						}

						thingtop = thing->z + thing->height;
						delta1 = thing->z - (polybottom + ((polytop - polybottom)/2));
						delta2 = thingtop - (polybottom + ((polytop - polybottom)/2));

						if (polytop > tmfloorz && abs(delta1) < abs(delta2))
							tmfloorz = tmdropoffz = polytop;

						if (polybottom < tmceilingz && abs(delta1) >= abs(delta2))
							tmceilingz = polybottom;
					}
					plink = (polymaplink_t *)(plink->link.next);
				}
			}
	}
#endif

	// tmfloorthing is set when tmfloorz comes from a thing's top
	tmfloorthing = NULL;

	validcount++;

	if (tmflags & MF_NOCLIP)
		return true;

	// Check things first, possibly picking things up.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.

	// MF_NOCLIPTHING: used by camera to not be blocked by things
	if (!(thing->flags & MF_NOCLIPTHING))
	{
		xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
		xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
		yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
		yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
					blockval = false;
	}

	validcount++;

	// check lines
	xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx - MAXMOVE)>>MAPBLOCKSHIFT;
	xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx + MAXMOVE)>>MAPBLOCKSHIFT;
	yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy - MAXMOVE)>>MAPBLOCKSHIFT;
	yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy + MAXMOVE)>>MAPBLOCKSHIFT;

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, PIT_CheckLine))
				blockval = false;

	return blockval;
}

static const fixed_t hoopblockdist = 16*FRACUNIT + 8*FRACUNIT;
static const fixed_t hoophalfheight = (56*FRACUNIT)/2;

// P_CheckPosition optimized for the MT_HOOPCOLLIDE object. This needs to be as fast as possible!
void P_CheckHoopPosition(mobj_t *hoopthing, fixed_t x, fixed_t y, fixed_t z, fixed_t radius)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || !players[i].mo)
			continue;

		if (abs(players[i].mo->x - x) >= hoopblockdist ||
			abs(players[i].mo->y - y) >= hoopblockdist ||
			abs((players[i].mo->z+hoophalfheight) - z) >= hoopblockdist)
			continue; // didn't hit it

		// can remove thing
		P_TouchSpecialThing(hoopthing, players[i].mo, false);
		break;
	}

	radius = 0; //unused
	return;
}

//
// P_CheckCameraPosition
//
boolean P_CheckCameraPosition(fixed_t x, fixed_t y, camera_t *thiscam)
{
	INT32 xl, xh, yl, yh, bx, by;
	subsector_t *newsubsec;

	tmx = x;
	tmy = y;

	tmbbox[BOXTOP] = y + thiscam->radius;
	tmbbox[BOXBOTTOM] = y - thiscam->radius;
	tmbbox[BOXRIGHT] = x + thiscam->radius;
	tmbbox[BOXLEFT] = x - thiscam->radius;

	newsubsec = R_PointInSubsector(x, y);
	ceilingline = blockingline = NULL;

	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;

	// Cameras use the heightsec's heights rather then the actual sector heights.
	// If you can see through it, why not move the camera through it too?
	if (newsubsec->sector->heightsec >= 0)
	{
		tmfloorz = tmsectorfloorz = tmdropoffz = sectors[newsubsec->sector->heightsec].floorheight;
		tmceilingz = tmsectorceilingz = sectors[newsubsec->sector->heightsec].ceilingheight;
	}

	// Check list of fake floors and see if tmfloorz/tmceilingz need to be altered.
	if (newsubsec->sector->ffloors)
	{
		ffloor_t *rover;
		fixed_t delta1, delta2;
		INT32 thingtop = thiscam->z + thiscam->height;

		for (rover = newsubsec->sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_BLOCKOTHERS) || !(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERALL))
				continue;

			delta1 = thiscam->z - (*rover->bottomheight
				+ ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight
				+ ((*rover->topheight - *rover->bottomheight)/2));
			if (*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
			{
				tmfloorz = tmdropoffz = *rover->topheight;
			}
			if (*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
			{
				tmceilingz = *rover->bottomheight;
			}
		}
	}

#ifdef POLYOBJECTS
	// Check polyobjects and see if tmfloorz/tmceilingz need to be altered
	{
		validcount++;

		xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
		xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
		yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
		yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

		for (by = yl; by <= yh; by++)
			for (bx = xl; bx <= xh; bx++)
			{
				INT32 offset;
				polymaplink_t *plink; // haleyjd 02/22/06

				if (bx < 0 || by < 0 || bx >= bmapwidth || by >= bmapheight)
					continue;

				offset = by*bmapwidth + bx;

				// haleyjd 02/22/06: consider polyobject lines
				plink = polyblocklinks[offset];

				while (plink)
				{
					polyobj_t *po = plink->po;

					if (po->validcount != validcount) // if polyobj hasn't been checked
					{
						sector_t *polysec;
						fixed_t delta1, delta2, thingtop;
						fixed_t polytop, polybottom;

						po->validcount = validcount;

						if (!P_PointInsidePolyobj(po, x, y))
						{
							plink = (polymaplink_t *)(plink->link.next);
							continue;
						}

						// We're inside it! Yess...
						polysec = po->lines[0]->backsector;

						if (po->flags & POF_CLIPPLANES)
						{
							polytop = polysec->ceilingheight;
							polybottom = polysec->floorheight;
						}
						else
						{
							polytop = INT32_MAX;
							polybottom = INT32_MIN;
						}

						thingtop = thiscam->z + thiscam->height;
						delta1 = thiscam->z - (polybottom + ((polytop - polybottom)/2));
						delta2 = thingtop - (polybottom + ((polytop - polybottom)/2));

						if (polytop > tmfloorz && abs(delta1) < abs(delta2))
							tmfloorz = tmdropoffz = polytop;

						if (polybottom < tmceilingz && abs(delta1) >= abs(delta2))
							tmceilingz = polybottom;
					}
					plink = (polymaplink_t *)(plink->link.next);
				}
			}
	}
#endif

	// Check things.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.

	// check lines
	xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, PIT_CheckCameraLine))
				return false;

	return true;
}

//
// CheckMissileImpact
//
static void CheckMissileImpact(mobj_t *mobj)
{
	if (!(mobj->flags & MF_MISSILE) || !mobj->target)
		return;

	if (!mobj->target->player)
		return;
}

// The highest the camera will "step up" onto another floor.
#define MAXCAMERASTEPMOVE MAXSTEPMOVE

//
// P_TryCameraMove
//
// Attempt to move the camera to a new position
//
// Return 1 if the move succeeded and no sliding should be done.
//
INT32 P_TryCameraMove(fixed_t x, fixed_t y, camera_t *thiscam)
{
	fixed_t oldx, oldy;
	subsector_t *s = R_PointInSubsector(x, y);
	INT32 retval = 1;
	boolean itsatwodlevel = false;

	floatok = false;

	if (twodlevel
		|| (thiscam == &camera && players[displayplayer].mo && (players[displayplayer].mo->flags2 & MF2_TWOD))
		|| (thiscam == &camera2 && players[secondarydisplayplayer].mo && (players[secondarydisplayplayer].mo->flags2 & MF2_TWOD)))
		itsatwodlevel = true;

	if (!itsatwodlevel)
	{
		boolean cameranoclip;

		if ((thiscam == &camera && (players[displayplayer].pflags & PF_NOCLIP))
		|| (thiscam == &camera2 && (players[secondarydisplayplayer].pflags & PF_NOCLIP)))
		{ // Noclipping player camera noclips too!!
			cameranoclip = true;
		}
		else
			cameranoclip = false;

		if (!P_CheckCameraPosition(x, y, thiscam))
		{
			if (cameranoclip)
			{
				floatok = true;
				oldx = thiscam->x;
				oldy = thiscam->y;
				thiscam->floorz = thiscam->z;
				thiscam->ceilingz = thiscam->z + thiscam->height;
				thiscam->x = x;
				thiscam->y = y;
				thiscam->subsector = s;
				return true;
			}
			else
				return 0; // solid wall or thing
		}

		if (tmceilingz - tmfloorz < thiscam->height)
		{
			if (cameranoclip)
			{
				floatok = true;
				oldx = thiscam->x;
				oldy = thiscam->y;
				thiscam->floorz = thiscam->z;
				thiscam->ceilingz = thiscam->z + thiscam->height;
				thiscam->x = x;
				thiscam->y = y;
				thiscam->subsector = s;
				return true;
			}
			else
				return 0; // doesn't fit
		}

		floatok = true;

		if (tmceilingz - thiscam->z < thiscam->height)
		{
			if (cameranoclip)
			{
				floatok = true;
				oldx = thiscam->x;
				oldy = thiscam->y;
				thiscam->floorz = thiscam->z;
				thiscam->ceilingz = thiscam->z + thiscam->height;
				thiscam->x = x;
				thiscam->y = y;
				thiscam->subsector = s;
				return true;
			}
			else if (s == thiscam->subsector && tmceilingz >= thiscam->z)
			{
				floatok = true;
				oldx = thiscam->x;
				oldy = thiscam->y;
				thiscam->floorz = tmfloorz;
				thiscam->ceilingz = tmfloorz + thiscam->height;
				thiscam->x = x;
				thiscam->y = y;
				thiscam->subsector = s;
				return true;
			}
			else
			{
				return 0; // mobj must lower itself to fit
			}
		}

		if ((tmfloorz - thiscam->z > MAXCAMERASTEPMOVE))
		{
			if (cameranoclip)
			{
				floatok = true;
				oldx = thiscam->x;
				oldy = thiscam->y;
				thiscam->floorz = thiscam->z;
				thiscam->ceilingz = thiscam->z + thiscam->height;
				thiscam->x = x;
				thiscam->y = y;
				thiscam->subsector = s;
				return true;
			}
			else
				return 0; // too big a step up
		}
	}
	else
	{
		tmfloorz = thiscam->subsector->sector->floorheight;
		tmceilingz = thiscam->subsector->sector->ceilingheight;
	}

	// the move is ok,
	// so link the thing into its new position

	oldx = thiscam->x;
	oldy = thiscam->y;
	thiscam->floorz = tmfloorz;
	thiscam->ceilingz = tmceilingz;
	thiscam->x = x;
	thiscam->y = y;
	thiscam->subsector = s;

	return retval;
}

//
// PIT_PushableMoved
//
// Move things standing on top
// of pushable things being pushed.
//
static mobj_t *stand;
static fixed_t standx, standy;

boolean PIT_PushableMoved(mobj_t *thing)
{
	fixed_t blockdist;

	if (!(thing->flags & MF_SOLID)
		|| (thing->flags & MF_NOGRAVITY))
		return true; // Don't move something non-solid!

	// Only pushables are supported... for now.
	if (!(thing->flags & MF_PUSHABLE))
		return true;

	if (thing == stand)
		return true;

	blockdist = stand->radius + thing->radius;

	if (abs(thing->x - stand->x) >= blockdist || abs(thing->y - stand->y) >= blockdist)
		return true; // didn't hit it

	if (thing->z != stand->z + stand->height + FRACUNIT)
		return true; // Not standing on top

	if (!stand->momx && !stand->momy)
		return true;

	// Move this guy!
	thing->momx = stand->momx;
	thing->momy = stand->momy;
	return true;
}

//
// P_TryMove
// Attempt to move to a new position.
//
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, boolean allowdropoff)
{
	fixed_t oldx, oldy;

	floatok = false;

	if (!P_CheckPosition(thing, x, y))
	{
		CheckMissileImpact(thing);
		return false; // solid wall or thing
	}

	if (!(thing->flags & MF_NOCLIP))
	{
		//All things are affected by their scale.
		fixed_t maxstep = FIXEDSCALE(MAXSTEPMOVE, thing->scale);

		if (thing->player)
		{
			// Don't 'step up' while springing,
			// Only step up "if needed".
			if (thing->state == &states[S_PLAY_PLG1]
				&& ((!(thing->eflags & MFE_VERTICALFLIP) && thing->momz > FRACUNIT)
				|| ((thing->eflags & MFE_VERTICALFLIP && thing->momz < -FRACUNIT))))
				maxstep = 0;
		}

		if (thing->type == MT_SKIM)
			maxstep = 0;

		if (tmceilingz - tmfloorz < thing->height)
		{
			CheckMissileImpact(thing);
			return false; // doesn't fit
		}

		floatok = true;

		if (thing->eflags & MFE_VERTICALFLIP)
		{
			if (thing->z < tmfloorz)
			{
				CheckMissileImpact(thing);
				return false; // mobj must raise itself to fit
			}
		}
		else if (tmceilingz - thing->z < thing->height)
		{
			CheckMissileImpact(thing);
			return false; // mobj must lower itself to fit
		}

		// If using type Section1:13, double the maxstep.
		if (thing->player && (P_PlayerTouchingSectorSpecial(thing->player, 1, 13)
			|| GETSECSPECIAL(R_PointInSubsector(x, y)->sector->special, 1) == 13))
			maxstep <<= 1;

		// Ramp test
		if (thing->player && !P_PlayerTouchingSectorSpecial(thing->player, 1, 14)
							&& GETSECSPECIAL(R_PointInSubsector(x, y)->sector->special, 1) != 14)
		{
			// If the floor difference is MAXSTEPMOVE or less, and the sector isn't Section1:14, ALWAYS
			// step down! Formerly required a Section1:13 sector for the full MAXSTEPMOVE, but no more.

			if (thing->eflags & MFE_VERTICALFLIP)
			{
				if (thing->z+thing->height == thing->ceilingz && tmceilingz > thing->z+thing->height && tmceilingz - thing->z+thing->height <= maxstep)
				{
					thing->z = tmceilingz - thing->height;
					thing->eflags |= MFE_JUSTSTEPPEDDOWN;
				}
			}
			else if (thing->z == thing->floorz && tmfloorz < thing->z && thing->z - tmfloorz <= maxstep)
			{
				thing->z = tmfloorz;
				thing->eflags |= MFE_JUSTSTEPPEDDOWN;
			}
		}

		if (thing->eflags & MFE_VERTICALFLIP)
		{
			if (thing->z + thing->height > tmceilingz + maxstep)
			{
				CheckMissileImpact(thing);
				return false; // too big a step up
			}
		}
		else if (tmfloorz - thing->z > maxstep)
		{
			CheckMissileImpact(thing);
			return false; // too big a step up
		}

		if (tmfloorz > thing->z)
		{
			if ((thing->flags & MF_MISSILE))
				CheckMissileImpact(thing);
		}

		if (!allowdropoff)
			if (!(thing->flags & (MF_FLOAT)) && thing->type != MT_SKIM && !tmfloorthing
				&& tmfloorz - tmdropoffz > MAXSTEPMOVE)
				return false; // don't stand over a dropoff
	}

	// The move is ok!

	// If it's a pushable object, check if anything is
	// standing on top and move it, too.
	if (thing->flags & MF_PUSHABLE)
	{
		INT32 bx, by, xl, xh, yl, yh;

		yh = (unsigned)(thing->y + MAXRADIUS - bmaporgy)>>MAPBLOCKSHIFT;
		yl = (unsigned)(thing->y - MAXRADIUS - bmaporgy)>>MAPBLOCKSHIFT;
		xh = (unsigned)(thing->x + MAXRADIUS - bmaporgx)>>MAPBLOCKSHIFT;
		xl = (unsigned)(thing->x - MAXRADIUS - bmaporgx)>>MAPBLOCKSHIFT;

		stand = thing;
		standx = x;
		standy = y;

		for (by = yl; by <= yh; by++)
			for (bx = xl; bx <= xh; bx++)
				P_BlockThingsIterator(bx, by, PIT_PushableMoved);
	}

	// Link the thing into its new position
	P_UnsetThingPosition(thing);

	oldx = thing->x;
	oldy = thing->y;
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->x = x;
	thing->y = y;

	if (tmfloorthing)
		thing->eflags &= ~MFE_ONGROUND; // not on real floor
	else
		thing->eflags |= MFE_ONGROUND;

	P_SetThingPosition(thing);
	return true;
}

boolean P_SceneryTryMove(mobj_t *thing, fixed_t x, fixed_t y)
{
	fixed_t oldx, oldy;

	if (!P_CheckPosition(thing, x, y))
		return false; // solid wall or thing

	if (!(thing->flags & MF_NOCLIP))
	{
		const fixed_t maxstep = MAXSTEPMOVE;

		if (tmceilingz - tmfloorz < thing->height)
			return false; // doesn't fit

		if (tmceilingz - thing->z < thing->height)
			return false; // mobj must lower itself to fit

		if (tmfloorz - thing->z > maxstep)
			return false; // too big a step up
	}

	// the move is ok,
	// so link the thing into its new position
	P_UnsetThingPosition(thing);

	oldx = thing->x;
	oldy = thing->y;
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->x = x;
	thing->y = y;

	if (tmfloorthing)
		thing->eflags &= ~MFE_ONGROUND; // not on real floor
	else
		thing->eflags |= MFE_ONGROUND;

	P_SetThingPosition(thing);
	return true;
}

//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
static boolean P_ThingHeightClip(mobj_t *thing)
{
	fixed_t oldfloorz = thing->floorz;
	boolean onfloor = P_IsObjectOnGround(thing);//(thing->z <= thing->floorz);

	if (thing->flags & MF_NOCLIPHEIGHT)
		return true;

	// Have player fall through floor?
	if (thing->player && thing->player->playerstate == PST_DEAD)
		return true;

	P_CheckPosition(thing, thing->x, thing->y);

	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;

	// Ugly hack?!?! As long as just ceilingz is the lowest,
	// you'll still get crushed, right?
	if (tmfloorz > oldfloorz+thing->height)
		return true;

	if (!tmfloorthing && onfloor && !(thing->flags & MF_NOGRAVITY))
	{
		if (thing->eflags & MFE_VERTICALFLIP)
			thing->pmomz = thing->z + thing->height - thing->ceilingz;
		else
			thing->pmomz = thing->floorz - thing->z;

		if (thing->player)
		{
			if (splitscreen && cv_chasecam2.value && thing->player == &players[secondarydisplayplayer])
				camera2.z += thing->pmomz;
			else if (cv_chasecam.value && thing->player == &players[displayplayer])
				camera.z += thing->pmomz;
		}

		if (thing->eflags & MFE_VERTICALFLIP)
			thing->z = thing->ceilingz - thing->height;
		else
			thing->z = thing->floorz;
	}
	else if (!tmfloorthing)
	{
		// don't adjust a floating monster unless forced to
		if (thing->eflags & MFE_VERTICALFLIP)
		{
			if (!onfloor && thing->z < tmfloorz)
				thing->z = thing->floorz;
		}
		else if (!onfloor && thing->z + thing->height > tmceilingz)
			thing->z = thing->ceilingz - thing->height;
	}

	// debug: be sure it falls to the floor
	thing->eflags &= ~MFE_ONGROUND;

	if (thing->ceilingz - thing->floorz < thing->height && thing->z >= thing->floorz)
		// BP: i know that this code cause many trouble but this also fixes
		// a lot of problems, mainly this is implementation of the stepping
		// for mobj (walk on solid corpse without jumping or fake 3d bridge)
		// problem is imp into imp at map01 and monster going at top of others
		return false;

	return true;
}

//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
static fixed_t bestslidefrac, secondslidefrac;
static line_t *bestslideline;
static line_t *secondslideline;
static mobj_t *slidemo;
static fixed_t tmxmove, tmymove;

//
// P_HitCameraSlideLine
//
static void P_HitCameraSlideLine(line_t *ld, camera_t *thiscam)
{
	INT32 side;
	angle_t lineangle, moveangle, deltaangle;
	fixed_t movelen, newlen;

	if (ld->slopetype == ST_HORIZONTAL)
	{
		tmymove = 0;
		return;
	}

	if (ld->slopetype == ST_VERTICAL)
	{
		tmxmove = 0;
		return;
	}

	side = P_PointOnLineSide(thiscam->x, thiscam->y, ld);
	lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

	if (side == 1)
		lineangle += ANGLE_180;

	moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
	deltaangle = moveangle-lineangle;

	if (deltaangle > ANGLE_180)
		deltaangle += ANGLE_180;

	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;

	movelen = P_AproxDistance(tmxmove, tmymove);
	newlen = FixedMul(movelen, FINECOSINE(deltaangle));

	tmxmove = FixedMul(newlen, FINECOSINE(lineangle));
	tmymove = FixedMul(newlen, FINESINE(lineangle));
}

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
static void P_HitSlideLine(line_t *ld)
{
	INT32 side;
	angle_t lineangle, moveangle, deltaangle;
	fixed_t movelen, newlen;

	if (ld->slopetype == ST_HORIZONTAL)
	{
		tmymove = 0;
		return;
	}

	if (ld->slopetype == ST_VERTICAL)
	{
		tmxmove = 0;
		return;
	}

	side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

	lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

	if (side == 1)
		lineangle += ANGLE_180;

	moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
	deltaangle = moveangle-lineangle;

	if (deltaangle > ANGLE_180)
		deltaangle += ANGLE_180;

	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;

	movelen = P_AproxDistance(tmxmove, tmymove);
	newlen = FixedMul(movelen, FINECOSINE(deltaangle));

	tmxmove = FixedMul(newlen, FINECOSINE(lineangle));
	tmymove = FixedMul(newlen, FINESINE(lineangle));
}

//
// P_HitBounceLine
//
// Adjusts the xmove / ymove so that the next move will bounce off the wall.
//
static void P_HitBounceLine(line_t *ld)
{
	INT32 side;
	angle_t lineangle, moveangle, deltaangle;
	fixed_t movelen;

	if (ld->slopetype == ST_HORIZONTAL)
	{
		tmymove = -tmymove;
		return;
	}

	if (ld->slopetype == ST_VERTICAL)
	{
		tmxmove = -tmxmove;
		return;
	}

	side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

	lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

	if (lineangle >= ANGLE_180)
		lineangle -= ANGLE_180;

	moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
	deltaangle = moveangle + 2*(lineangle - moveangle);

	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;

	movelen = P_AproxDistance(tmxmove, tmymove);

	tmxmove = FixedMul(movelen, FINECOSINE(deltaangle));
	tmymove = FixedMul(movelen, FINESINE(deltaangle));

	deltaangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
}

//
// PTR_SlideCameraTraverse
//
static boolean PTR_SlideCameraTraverse(intercept_t *in)
{
	line_t *li;

	I_Assert(in->isaline);

	li = in->d.line;

	if (!(li->flags & ML_TWOSIDED))
	{
		if (P_PointOnLineSide(mapcampointer->x, mapcampointer->y, li))
			return true; // don't hit the back side
		goto isblocking;
	}

	// set openrange, opentop, openbottom
	P_CameraLineOpening(li);

	if (openrange < mapcampointer->height)
		goto isblocking; // doesn't fit

	if (opentop - mapcampointer->z < mapcampointer->height)
		goto isblocking; // mobj is too high

	if (openbottom - mapcampointer->z > 0) // We don't want to make the camera step up.
		goto isblocking; // too big a step up

	// this line doesn't block movement
	return true;

	// the line does block movement,
	// see if it is closer than best so far
isblocking:
	{
		if (in->frac < bestslidefrac)
		{
			secondslidefrac = bestslidefrac;
			secondslideline = bestslideline;
			bestslidefrac = in->frac;
			bestslideline = li;
		}
	}

	return false; // stop
}

//
// P_IsClimbingValid
//
static boolean P_IsClimbingValid(player_t *player, angle_t angle)
{
	fixed_t platx, platy;
	subsector_t *glidesector;
	boolean climb = true;

	platx = P_ReturnThrustX(player->mo, angle, player->mo->radius + FIXEDSCALE(8*FRACUNIT, player->mo->scale));
	platy = P_ReturnThrustY(player->mo, angle, player->mo->radius + FIXEDSCALE(8*FRACUNIT, player->mo->scale));

	glidesector = R_PointInSubsector(player->mo->x + platx, player->mo->y + platy);

	if (glidesector->sector != player->mo->subsector->sector)
	{
		boolean floorclimb = false, thrust = false, boostup = false;

		if (glidesector->sector->ffloors)
		{
			ffloor_t *rover;
			for (rover = glidesector->sector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_BLOCKPLAYER))
					continue;

				floorclimb = true;

				if (player->mo->eflags & MFE_VERTICALFLIP)
				{
					if ((*rover->topheight < player->mo->z + player->mo->height) && ((player->mo->z + player->mo->height + player->mo->momz) < *rover->topheight))
					{
						floorclimb = true;
						boostup = false;
					}
					if (*rover->topheight < player->mo->z) // Waaaay below the ledge.
					{
						floorclimb = false;
						boostup = false;
						thrust = false;
					}
					if (*rover->bottomheight > player->mo->z + player->mo->height - FIXEDSCALE(16*FRACUNIT,player->mo->scale))
					{
						floorclimb = false;
						thrust = true;
						boostup = true;
					}
				}
				else
				{
					if ((*rover->bottomheight > player->mo->z) && ((player->mo->z - player->mo->momz) > *rover->bottomheight))
					{
						floorclimb = true;
						boostup = false;
					}
					if (*rover->bottomheight > player->mo->z + player->mo->height) // Waaaay below the ledge.
					{
						floorclimb = false;
						boostup = false;
						thrust = false;
					}
					if (*rover->topheight < player->mo->z + FIXEDSCALE(16*FRACUNIT,player->mo->scale))
					{
						floorclimb = false;
						thrust = true;
						boostup = true;
					}
				}

				if (floorclimb)
					break;
			}
		}

		if (player->mo->eflags & MFE_VERTICALFLIP)
		{
			if ((glidesector->sector->floorheight <= player->mo->z + player->mo->height)
				&& ((player->mo->z + player->mo->momz) <= glidesector->sector->floorheight))
				floorclimb = true;

			if (!floorclimb && glidesector->sector->ceilingheight > player->mo->z - FIXEDSCALE(16*FRACUNIT,player->mo->scale)
				&& (glidesector->sector->floorpic == skyflatnum
				|| glidesector->sector->floorheight
				< (player->mo->z - FIXEDSCALE(8*FRACUNIT,player->mo->scale))))
			{
				thrust = true;
				boostup = true;
				// Play climb-up animation here
			}

			if ((glidesector->sector->floorheight > player->mo->z)
				&& glidesector->sector->floorpic == skyflatnum)
				return false;

			if ((player->mo->z + player->mo->height - FIXEDSCALE(16*FRACUNIT,player->mo->scale) > glidesector->sector->ceilingheight)
				|| (player->mo->z + player->mo->height <= glidesector->sector->floorheight))
				floorclimb = true;
		}
		else
		{
			if ((glidesector->sector->ceilingheight >= player->mo->z)
				&& ((player->mo->z - player->mo->momz) >= glidesector->sector->ceilingheight))
				floorclimb = true;

			if (!floorclimb && glidesector->sector->floorheight < player->mo->z + FIXEDSCALE(16*FRACUNIT,player->mo->scale)
				&& (glidesector->sector->ceilingpic == skyflatnum
				|| glidesector->sector->ceilingheight
				> (player->mo->z + player->mo->height + FIXEDSCALE(8*FRACUNIT,player->mo->scale))))
			{
				thrust = true;
				boostup = true;
				// Play climb-up animation here
			}

			if ((glidesector->sector->ceilingheight < player->mo->z+player->mo->height)
				&& glidesector->sector->ceilingpic == skyflatnum)
				return false;

			if ((player->mo->z + FIXEDSCALE(16*FRACUNIT,player->mo->scale) < glidesector->sector->floorheight)
				|| (player->mo->z >= glidesector->sector->ceilingheight))
				floorclimb = true;
		}

		climb = false;

		if (!floorclimb)
			return false;

		return true;
	}

	return false;
}

//
// PTR_SlideTraverse
//
static boolean PTR_SlideTraverse(intercept_t *in)
{
	line_t *li;

	I_Assert(in->isaline);

	li = in->d.line;

	if (!(li->flags & ML_TWOSIDED))
	{
		if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
			return true; // don't hit the back side
		goto isblocking;
	}

	// set openrange, opentop, openbottom
	P_LineOpening(li);

	if (openrange < slidemo->height)
		goto isblocking; // doesn't fit

	if (opentop - slidemo->z < slidemo->height)
		goto isblocking; // mobj is too high

	if (openbottom - slidemo->z > FIXEDSCALE(MAXSTEPMOVE, slidemo->scale))
		goto isblocking; // too big a step up

	// this line doesn't block movement
	return true;

	// the line does block movement,
	// see if it is closer than best so far
isblocking:
	if (li->polyobj && slidemo->player)
	{
		if ((li->polyobj->lines[0]->backsector->flags & SF_TRIGGERSPECIAL_TOUCH) && !(li->polyobj->flags & POF_NOSPECIALS))
			P_ProcessSpecialSector(slidemo->player, slidemo->subsector->sector, li->polyobj->lines[0]->backsector);
	}

	if (slidemo->player && ((slidemo->player->pflags & PF_GLIDING) || slidemo->player->climbing)
		&& (slidemo->player->charability == CA_GLIDEANDCLIMB))
	{
		line_t *checkline = li;
		sector_t *checksector;
		ffloor_t *rover;
		boolean fofline = false;
		INT32 side = P_PointOnLineSide(slidemo->x, slidemo->y, li);

		if (!side && li->backsector)
			checksector = li->backsector;
		else
			checksector = li->frontsector;

		if (checksector->ffloors)
		{
			for (rover = checksector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_BLOCKPLAYER) || (rover->flags & FF_BUSTUP))
					continue;

				if (*rover->topheight < slidemo->z)
					continue;

				if (*rover->bottomheight > slidemo->z + slidemo->height)
					continue;

				// Got this far, so I guess it's climbable.
				if (rover->master->flags & ML_TFERLINE)
				{
					size_t linenum = li-checksector->lines[0];
					checkline = rover->master->frontsector->lines[0] + linenum;
					fofline = true;
				}

				break;
			}
		}

		// see about climbing on the wall
		if (!(checkline->flags & ML_NOCLIMB))
		{
			angle_t climbangle, climbline = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y);
			INT32 whichside = P_PointOnLineSide(slidemo->x, slidemo->y, li);

			if (whichside) // on second side?
				climbline += ANGLE_180;

			if (((!slidemo->player->climbing
				&& abs(slidemo->angle - ANGLE_90 - climbline) < ANGLE_45) ||

				(slidemo->player->climbing == 1
				&& abs(slidemo->angle - climbline) < ANGLE_135))

				&& P_IsClimbingValid(slidemo->player, climbangle =
				R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y)
				+ (ANGLE_90 * (whichside ? -1 : 1))))
			{
				slidemo->angle = climbangle;
				if (slidemo->player == &players[consoleplayer])
					localangle = slidemo->angle;
				else if (splitscreen && slidemo->player ==
					&players[secondarydisplayplayer])
				{
					localangle2 = slidemo->angle;
				}

				if (!slidemo->player->climbing)
					slidemo->player->climbing = 5;

				slidemo->player->pflags &= ~PF_GLIDING;
				slidemo->player->pflags &= ~PF_SPINNING;
				slidemo->player->pflags &= ~PF_JUMPED;
				slidemo->player->pflags &= ~PF_THOKKED;
				slidemo->player->glidetime = 0;
				slidemo->player->secondjump = 0;

				if (slidemo->player->climbing > 1)
					slidemo->momz = slidemo->momx = slidemo->momy = 0;

				if (fofline)
					whichside = 0;

				if (!whichside)
				{
					slidemo->player->lastsidehit = checkline->sidenum[whichside];
					slidemo->player->lastlinehit = (INT16)(checkline - lines);
				}

				P_Thrust(slidemo, slidemo->angle, 5*FRACUNIT);
			}
		}
	}

	if (in->frac < bestslidefrac && (!slidemo->player || !slidemo->player->climbing))
	{
		secondslidefrac = bestslidefrac;
		secondslideline = bestslideline;
		bestslidefrac = in->frac;
		bestslideline = li;
	}

	return false; // stop
}

//
// P_SlideCameraMove
//
// Tries to slide the camera along a wall.
//
void P_SlideCameraMove(camera_t *thiscam)
{
	fixed_t leadx, leady, trailx, traily, newx, newy;
	INT32 hitcount = 0;
	INT32 retval = 0;

	bestslideline = NULL;

retry:
	if (++hitcount == 3)
		goto stairstep; // don't loop forever

	// trace along the three leading corners
	if (thiscam->momx > 0)
	{
		leadx = thiscam->x + thiscam->radius;
		trailx = thiscam->x - thiscam->radius;
	}
	else
	{
		leadx = thiscam->x - thiscam->radius;
		trailx = thiscam->x + thiscam->radius;
	}

	if (thiscam->momy > 0)
	{
		leady = thiscam->y + thiscam->radius;
		traily = thiscam->y - thiscam->radius;
	}
	else
	{
		leady = thiscam->y - thiscam->radius;
		traily = thiscam->y + thiscam->radius;
	}

	bestslidefrac = FRACUNIT+1;

	mapcampointer = thiscam;

	P_PathTraverse(leadx, leady, leadx + thiscam->momx, leady + thiscam->momy,
		PT_ADDLINES, PTR_SlideCameraTraverse);
	P_PathTraverse(trailx, leady, trailx + thiscam->momx, leady + thiscam->momy,
		PT_ADDLINES, PTR_SlideCameraTraverse);
	P_PathTraverse(leadx, traily, leadx + thiscam->momx, traily + thiscam->momy,
		PT_ADDLINES, PTR_SlideCameraTraverse);

	// move up to the wall
	if (bestslidefrac == FRACUNIT+1)
	{
		retval = P_TryCameraMove(thiscam->x, thiscam->y + thiscam->momy, thiscam);
		// the move must have hit the middle, so stairstep
stairstep:
		if (!retval) // Allow things to drop off.
			P_TryCameraMove(thiscam->x + thiscam->momx, thiscam->y, thiscam);
		return;
	}

	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0)
	{
		newx = FixedMul(thiscam->momx, bestslidefrac);
		newy = FixedMul(thiscam->momy, bestslidefrac);

		retval = P_TryCameraMove(thiscam->x + newx, thiscam->y + newy, thiscam);

		if (!retval)
			goto stairstep;
	}

	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = FRACUNIT - (bestslidefrac+0x800);

	if (bestslidefrac > FRACUNIT)
		bestslidefrac = FRACUNIT;

	if (bestslidefrac <= 0)
		return;

	tmxmove = FixedMul(thiscam->momx, bestslidefrac);
	tmymove = FixedMul(thiscam->momy, bestslidefrac);

	P_HitCameraSlideLine(bestslideline, thiscam); // clip the moves

	thiscam->momx = tmxmove;
	thiscam->momy = tmymove;

	retval = P_TryCameraMove(thiscam->x + tmxmove, thiscam->y + tmymove, thiscam);

	if (!retval)
		goto retry;
}

//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove(mobj_t *mo)
{
	fixed_t leadx, leady, trailx, traily, newx, newy;
	INT16 hitcount = 0;

	slidemo = mo;

	bestslideline = NULL;

retry:
	if (++hitcount == 3)
		goto stairstep; // don't loop forever

	// trace along the three leading corners
	if (mo->momx > 0)
	{
		leadx = mo->x + mo->radius;
		trailx = mo->x - mo->radius;
	}
	else
	{
		leadx = mo->x - mo->radius;
		trailx = mo->x + mo->radius;
	}

	if (mo->momy > 0)
	{
		leady = mo->y + mo->radius;
		traily = mo->y - mo->radius;
	}
	else
	{
		leady = mo->y - mo->radius;
		traily = mo->y + mo->radius;
	}

	bestslidefrac = FRACUNIT+1;

	P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
		PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
		PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
		PT_ADDLINES, PTR_SlideTraverse);

	// Some walls are bouncy even if you're not
	if (bestslideline && bestslideline->flags & ML_BOUNCY)
	{
		P_BounceMove(mo);
		return;
	}

	// move up to the wall
	if (bestslidefrac == FRACUNIT+1)
	{
		// the move must have hit the middle, so stairstep
stairstep:
		if (!P_TryMove(mo, mo->x, mo->y + mo->momy, true))
			P_TryMove(mo, mo->x + mo->momx, mo->y, true); //Allow things to drop off.
		return;
	}

	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0)
	{
		newx = FixedMul(mo->momx, bestslidefrac);
		newy = FixedMul(mo->momy, bestslidefrac);

		if (!P_TryMove(mo, mo->x + newx, mo->y + newy, true))
			goto stairstep;
	}

	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = FRACUNIT - (bestslidefrac+0x800);

	if (bestslidefrac > FRACUNIT)
		bestslidefrac = FRACUNIT;

	if (bestslidefrac <= 0)
		return;

	tmxmove = FixedMul(mo->momx, bestslidefrac);
	tmymove = FixedMul(mo->momy, bestslidefrac);

	P_HitSlideLine(bestslideline); // clip the moves

	if ((twodlevel || (mo->flags2 & MF2_TWOD)) && mo->player)
	{
		mo->momx = tmxmove;
		tmymove = 0;
	}
	else
	{
		mo->momx = tmxmove;
		mo->momy = tmymove;
	}

	if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, true))
		goto retry;
}

//
// P_BounceMove
//
// The momx / momy move is bad, so try to bounce off a wall.
//
void P_BounceMove(mobj_t *mo)
{
	fixed_t leadx, leady;
	fixed_t trailx, traily;
	fixed_t newx, newy;
	INT32 hitcount;
	fixed_t mmomx = 0, mmomy = 0;

	slidemo = mo;
	hitcount = 0;

retry:
	if (++hitcount == 3)
		goto bounceback; // don't loop forever

	if (mo->player)
	{
		mmomx = mo->player->rmomx;
		mmomy = mo->player->rmomy;
	}
	else
	{
		mmomx = mo->momx;
		mmomy = mo->momy;
	}

	// trace along the three leading corners
	if (mo->momx > 0)
	{
		leadx = mo->x + mo->radius;
		trailx = mo->x - mo->radius;
	}
	else
	{
		leadx = mo->x - mo->radius;
		trailx = mo->x + mo->radius;
	}

	if (mo->momy > 0)
	{
		leady = mo->y + mo->radius;
		traily = mo->y - mo->radius;
	}
	else
	{
		leady = mo->y - mo->radius;
		traily = mo->y + mo->radius;
	}

	bestslidefrac = FRACUNIT + 1;

	P_PathTraverse(leadx, leady, leadx + mmomx, leady + mmomy, PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(trailx, leady, trailx + mmomx, leady + mmomy, PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(leadx, traily, leadx + mmomx, traily + mmomy, PT_ADDLINES, PTR_SlideTraverse);

	// move up to the wall
	if (bestslidefrac == FRACUNIT + 1)
	{
		// the move must have hit the middle, so bounce straight back
bounceback:
		if (P_TryMove(mo, mo->x - mmomx, mo->y - mmomy, true))
		{
			mo->momx *= -1;
			mo->momy *= -1;
			mo->momx = FixedMul(mo->momx, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
			mo->momy = FixedMul(mo->momy, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));

			if (mo->player)
			{
				mo->player->cmomx *= -1;
				mo->player->cmomy *= -1;
				mo->player->cmomx = FixedMul(mo->player->cmomx,
					(FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
				mo->player->cmomy = FixedMul(mo->player->cmomy,
					(FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
			}
		}
		return;
	}

	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0)
	{
		newx = FixedMul(mmomx, bestslidefrac);
		newy = FixedMul(mmomy, bestslidefrac);

		if (!P_TryMove(mo, mo->x + newx, mo->y + newy, true))
			goto bounceback;
	}

	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = FRACUNIT - bestslidefrac;

	if (bestslidefrac > FRACUNIT)
		bestslidefrac = FRACUNIT;

	if (bestslidefrac <= 0)
		return;

	if (mo->type == MT_SHELL)
	{
		tmxmove = mmomx;
		tmymove = mmomy;
	}
	else if (mo->type == MT_THROWNBOUNCE)
	{
		tmxmove = FixedMul(mmomx, (FRACUNIT - (FRACUNIT>>6) - (FRACUNIT>>5)));
		tmymove = FixedMul(mmomy, (FRACUNIT - (FRACUNIT>>6) - (FRACUNIT>>5)));
	}
	else
	{
		tmxmove = FixedMul(mmomx, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
		tmymove = FixedMul(mmomy, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
	}

	P_HitBounceLine(bestslideline); // clip the moves

	mo->momx = tmxmove;
	mo->momy = tmymove;

	if (mo->player)
	{
		mo->player->cmomx = tmxmove;
		mo->player->cmomy = tmymove;
	}

	if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, true))
		goto retry;
}

mobj_t *linetarget; // who got hit (or NULL)
static mobj_t *shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
static fixed_t shootz;
static fixed_t lastz; // The last z height of the bullet when it crossed a line

// More intelligent autoaiming
static INT32 aim_nofriends; // stores (CTF team #) or (skincolor #+1) or (skinnum #+1)

fixed_t attackrange;
static fixed_t aimslope;

// slopes to top and bottom of target
// killough 4/20/98: make static instead of using ones in p_sight.c

fixed_t topslope;
fixed_t bottomslope;

//
// PTR_AimTraverse
// Sets linetarget and aimslope when a target is aimed at.
//
//added : 15-02-98: comment
// Returns true if the thing is not shootable, else continue through..
//
static boolean PTR_AimTraverse(intercept_t *in)
{
	line_t *li;
	mobj_t *th;
	fixed_t slope, thingtopslope, thingbottomslope, dist;
	INT32 dir;

	if (in->isaline)
	{
		li = in->d.line;

		if (!(li->flags & ML_TWOSIDED))
			return false; // stop

		// Crosses a two sided line.
		// A two sided line will restrict
		// the possible target ranges.
		tmthing = NULL;
		P_LineOpening(li);

		if (openbottom >= opentop)
			return false; // stop

		dist = FixedMul(attackrange, in->frac);

		if (li->frontsector->floorheight != li->backsector->floorheight)
		{
			slope = FixedDiv(openbottom - shootz, dist);
			if (slope > bottomslope)
				bottomslope = slope;
		}

		if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
		{
			slope = FixedDiv(opentop - shootz, dist);
			if (slope < topslope)
				topslope = slope;
		}

		if (topslope <= bottomslope)
			return false; // stop

		if (li->frontsector->ffloors || li->backsector->ffloors)
		{
			INT32 frontflag = P_PointOnLineSide(shootthing->x, shootthing->y, li);

			dir = aimslope > 0 ? 1 : aimslope < 0 ? -1 : 0;

			//SoM: Check 3D FLOORS!
			if (li->frontsector->ffloors)
			{
				ffloor_t *rover = li->frontsector->ffloors;
				fixed_t highslope, lowslope;

				for (; rover; rover = rover->next)
				{
					if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
						continue;

					highslope = FixedDiv(*rover->topheight - shootz, dist);
					lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
					if ((aimslope >= lowslope && aimslope <= highslope))
						return false;

					if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
						frontflag |= 0x2;

					if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
						frontflag |= 0x2;
				}
			}

			if (li->backsector->ffloors)
			{
				ffloor_t *rover = li->backsector->ffloors;
				fixed_t highslope, lowslope;

				for (; rover; rover = rover->next)
				{
					if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
						continue;

					highslope = FixedDiv(*rover->topheight - shootz, dist);
					lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
					if ((aimslope >= lowslope && aimslope <= highslope))
						return false;

					if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
						frontflag |= 0x4;

					if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
						frontflag |= 0x4;
				}
			}
			if ((!(frontflag & 0x1) && frontflag & 0x2) || (frontflag & 0x1 && frontflag & 0x4))
				return false;
		}

		lastz = FixedMul(aimslope, dist) + shootz;

		return true; // shot continues
	}

	// shoot a thing
	th = in->d.thing;
	if (th == shootthing)
		return true; // can't shoot self

	if (!(th->flags & MF_SHOOTABLE))
		return true; // corpse or something

	// friends don't autoaim at friends
	if (aim_nofriends
		&& th->player
		&& (((gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value)) && th->player->ctfteam == aim_nofriends)
		|| aim_nofriends == -1))
		return true;

	if (th->flags & MF_MONITOR)
		return true; // don't autoaim at monitors

	if (th->type == MT_THROWNGRENADE)
		return true; // don't autoaim at grenades

	if (netgame && th->player && th->player->spectator)
		return true; // don't autoaim at spectators

	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, in->frac);
	thingtopslope = FixedDiv(th->z+th->height - shootz, dist);

	//added : 15-02-98: bottomslope is negative!
	if (thingtopslope < bottomslope)
		return true; // shot over the thing

	thingbottomslope = FixedDiv(th->z - shootz, dist);

	if (thingbottomslope > topslope)
		return true; // shot under the thing

	// this thing can be hit!
	if (thingtopslope > topslope)
		thingtopslope = topslope;

	if (thingbottomslope < bottomslope)
		thingbottomslope = bottomslope;

	//added : 15-02-98: find the slope just in the middle(y) of the thing!
	aimslope = (thingtopslope + thingbottomslope)/2;
	linetarget = th;

	return false; // don't go any farther
}

//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance)
{
	fixed_t x2, y2;
	const fixed_t baseaiming = 10*FRACUNIT/16;

	I_Assert(t1 != NULL);

	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;

	topslope = baseaiming;
	bottomslope = -baseaiming;

	if (t1->player)
	{
		const angle_t aiming = t1->player->aiming>>ANGLETOFINESHIFT;
		const fixed_t cosineaiming = FINECOSINE(aiming);
		const fixed_t slopeaiming = FINETANGENT((FINEANGLES/4+aiming) & FINEMASK);
		x2 = t1->x + FixedMul(FixedMul(distance, FINECOSINE(angle)), cosineaiming);
		y2 = t1->y + FixedMul(FixedMul(distance, FINESINE(angle)), cosineaiming);

		topslope += slopeaiming;
		bottomslope += slopeaiming;

		if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value)) // Team
			aim_nofriends = t1->player->ctfteam;
		else if (gametype == GT_COOP)
			aim_nofriends = -1; // Don't shoot any players
	}
	else
	{
		x2 = t1->x + (distance>>FRACBITS)*FINECOSINE(angle);
		y2 = t1->y + (distance>>FRACBITS)*FINESINE(angle);

		//added : 15-02-98: Fab comments...
		// Doom's base engine says that at a distance of 160,
		// the 2d graphics on the plane x, y correspond 1/1 with plane units
		aim_nofriends = 0;
	}

	shootz = lastz = t1->z + (t1->height>>1) + 8*FRACUNIT;

	// can't shoot outside view angles
	attackrange = distance;
	linetarget = NULL;

	//added : 15-02-98: comments
	// traverse all linedefs and mobjs from the blockmap containing t1,
	// to the blockmap containing the dest. point.
	// Call the function for each mobj/line on the way,
	// starting with the mobj/linedef at the shortest distance...
	P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_AimTraverse);

	//added : 15-02-98: linetarget is only for mobjs, not for linedefs
	if (linetarget)
		return aimslope;

	return 0;
}

//
// RADIUS ATTACK
//
static INT32 bombdamage;
static mobj_t *bombsource;
static mobj_t *bombspot;

//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
static boolean PIT_RadiusAttack(mobj_t *thing)
{
	fixed_t dx, dy, dz, dist;

	if (!(thing->flags & MF_SHOOTABLE))
		return true;

	if (thing->flags & MF_BOSS)
		return true;

	switch (thing->type)
	{
		case MT_SKIM:
		case MT_JETTBOMBER: // Jetty-Syn Bomber
			return true;
		default:
			if (thing->flags & MF_MONITOR)
				return true;
			break;
	}

	dx = abs(thing->x - bombspot->x);
	dy = abs(thing->y - bombspot->y);
	dz = abs(thing->z + (thing->height>>1) - bombspot->z);

	dist = P_AproxDistance(P_AproxDistance(dx, dy), dz);
	dist -= thing->radius;

	dist >>= FRACBITS;

	if (dist < 0)
		dist = 0;

	if (dist >= bombdamage)
		return true; // out of range

	if (thing->floorz > bombspot->z && bombspot->ceilingz < thing->z)
		return true;

	if (thing->ceilingz < bombspot->z && bombspot->floorz > thing->z)
		return true;

	if (P_CheckSight(thing, bombspot))
	{
		INT32 damage = bombdamage - dist;
		INT32 momx = 0, momy = 0;
		if (dist)
		{
			momx = (thing->x - bombspot->x)/dist;
			momy = (thing->y - bombspot->y)/dist;
		}
		// must be in direct path
		P_DamageMobj(thing, bombspot, bombsource, damage); // Tails 01-11-2001
	}

	return true;
}

//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t *spot, mobj_t *source, INT32 damage)
{
	INT32 x, y;
	INT32 xl, xh, yl, yh;
	fixed_t dist;

	dist = (damage + MAXRADIUS)<<FRACBITS;
	yh = (unsigned)(spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
	yl = (unsigned)(spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
	xh = (unsigned)(spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
	xl = (unsigned)(spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
	bombspot = spot;

	bombsource = source;
	bombdamage = damage;

	for (y = yl; y <= yh; y++)
		for (x = xl; x <= xh; x++)
			P_BlockThingsIterator(x, y, PIT_RadiusAttack);
}

//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_CheckSector (? was P_ChangeSector - Graue) again
//  to undo the changes.
//
static boolean crushchange;
static boolean nofit;

//
// PIT_ChangeSector
//
static boolean PIT_ChangeSector(mobj_t *thing, boolean realcrush)
{
	if (P_ThingHeightClip(thing))
	{
		// keep checking
		return true;
	}

	if (!(thing->flags & MF_SHOOTABLE) && !(thing->flags & MF_PUSHABLE))
	{
		// assume it is bloody gibs or something
		return true;
	}

	// Crush the thing if necessary, and if it's a crumbling FOF that did it,
	// reward the player who made it crumble!
	if (thing->z + thing->height > thing->ceilingz && thing->z <= thing->ceilingz)
	{
		if (realcrush && thing->subsector->sector->ffloors)
		{
			ffloor_t *rover;
			fixed_t delta1, delta2;
			INT32 thingtop = thing->z + thing->height;

			for (rover = thing->subsector->sector->ffloors; rover; rover = rover->next)
			{
				if (!(((rover->flags & FF_BLOCKPLAYER) && thing->player)
				|| ((rover->flags & FF_BLOCKOTHERS) && !thing->player)) || !(rover->flags & FF_EXISTS))
					continue;

				delta1 = thing->z - (*rover->bottomheight + *rover->topheight)/2;
				delta2 = thingtop - (*rover->bottomheight + *rover->topheight)/2;
				if (*rover->bottomheight <= thing->ceilingz && abs(delta1) >= abs(delta2))
				{
					thinker_t *think;
					elevator_t *crumbler;

					for (think = thinkercap.next; think != &thinkercap; think = think->next)
					{
						if (think->function.acp1 != (actionf_p1)T_StartCrumble)
							continue;

						crumbler = (elevator_t *)think;

						if (crumbler->player && crumbler->player->mo
							&& crumbler->player->mo != thing
							&& crumbler->actionsector == thing->subsector->sector
							&& crumbler->sector == rover->master->frontsector
							&& (crumbler->type == elevateBounce
							|| crumbler->type == elevateContinuous))
						{
							if (netgame && thing->player && thing->player->spectator)
								P_DamageMobj(thing, NULL, NULL, 42000); // Respawn crushed spectators
							else
								P_DamageMobj(thing, crumbler->player->mo, crumbler->player->mo, 10000);
							return true;
						}
					}
				}
			}
		}

		if (thing->flags & MF_PUSHABLE)
		{
			nofit = true;
			return false;
		}

		if (realcrush)
		{
			// Instant-death, but no one to blame
			if (netgame && thing->player && thing->player->spectator)
				P_DamageMobj(thing, NULL, NULL, 42000); // Respawn crushed spectators
			else
				P_DamageMobj(thing, NULL, NULL, 10000);
		}
	}

	if (realcrush && crushchange)
		P_DamageMobj(thing, NULL, NULL, 1);

	// keep checking (crush other things)
	return true;
}

//
// P_CheckSector
//
boolean P_CheckSector(sector_t *sector, boolean crunch)
{
	msecnode_t *n;

	nofit = false;
	crushchange = crunch;

	// killough 4/4/98: scan list front-to-back until empty or exhausted,
	// restarting from beginning after each thing is processed. Avoids
	// crashes, and is sure to examine all things in the sector, and only
	// the things which are in the sector, until a steady-state is reached.
	// Things can arbitrarily be inserted and removed and it won't mess up.
	//
	// killough 4/7/98: simplified to avoid using complicated counter


	// First, let's see if anything will keep it from crushing.
	if (sector->numattached)
	{
		size_t i;
		sector_t *sec;
		for (i = 0; i < sector->numattached; i++)
		{
			sec = &sectors[sector->attached[i]];
			for (n = sec->touching_thinglist; n; n = n->m_snext)
				n->visited = false;

			sec->moved = true;

			P_RecalcPrecipInSector(sec);

			if (!sector->attachedsolid[i])
				continue;

			do
			{
				for (n = sec->touching_thinglist; n; n = n->m_snext)
				if (!n->visited)
				{
					n->visited = true;
					if (!(n->m_thing->flags & MF_NOBLOCKMAP))
					{
						if (!PIT_ChangeSector(n->m_thing, false))
						{
							nofit = true;
							return nofit;
						}
					}
					break;
				}
			} while (n);
		}
	}

	// Mark all things invalid
	sector->moved = true;

	for (n = sector->touching_thinglist; n; n = n->m_snext)
		n->visited = false;

	do
	{
		for (n = sector->touching_thinglist; n; n = n->m_snext) // go through list
			if (!n->visited) // unprocessed thing found
			{
				n->visited = true; // mark thing as processed
				if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
				{
					if (!PIT_ChangeSector(n->m_thing, false)) // process it
					{
						nofit = true;
						return nofit;
					}
				}
				break; // exit and start over
			}
	} while (n); // repeat from scratch until all things left are marked valid

	// Nothing blocked us, so lets crush for real!
	if (sector->numattached)
	{
		size_t i;
		sector_t *sec;
		for (i = 0; i < sector->numattached; i++)
		{
			sec = &sectors[sector->attached[i]];
			for (n = sec->touching_thinglist; n; n = n->m_snext)
				n->visited = false;

			sec->moved = true;

			P_RecalcPrecipInSector(sec);

			if (!sector->attachedsolid[i])
				continue;

			do
			{
				for (n = sec->touching_thinglist; n; n = n->m_snext)
				if (!n->visited)
				{
					n->visited = true;
					if (!(n->m_thing->flags & MF_NOBLOCKMAP))
					{
						PIT_ChangeSector(n->m_thing, true);
						return nofit;
					}
					break;
				}
			} while (n);
		}
	}

	// Mark all things invalid
	sector->moved = true;

	for (n = sector->touching_thinglist; n; n = n->m_snext)
		n->visited = false;

	do
	{
		for (n = sector->touching_thinglist; n; n = n->m_snext) // go through list
			if (!n->visited) // unprocessed thing found
			{
				n->visited = true; // mark thing as processed
				if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
				{
					PIT_ChangeSector(n->m_thing, true); // process it
					return nofit;
				}
				break; // exit and start over
			}
	} while (n); // repeat from scratch until all things left are marked valid

	return nofit;
}

/*
 SoM: 3/15/2000
 Lots of new Boom functions that work faster and add functionality.
*/

static msecnode_t *headsecnode = NULL;
static mprecipsecnode_t *headprecipsecnode = NULL;

void P_Initsecnode(void)
{
	headsecnode = NULL;
	headprecipsecnode = NULL;
}

// P_GetSecnode() retrieves a node from the freelist. The calling routine
// should make sure it sets all fields properly.

static msecnode_t *P_GetSecnode(void)
{
	msecnode_t *node;

	if (headsecnode)
	{
		node = headsecnode;
		headsecnode = headsecnode->m_snext;
	}
	else
		node = Z_Calloc(sizeof (*node), PU_LEVEL, NULL);
	return node;
}

static mprecipsecnode_t *P_GetPrecipSecnode(void)
{
	mprecipsecnode_t *node;

	if (headprecipsecnode)
	{
		node = headprecipsecnode;
		headprecipsecnode = headprecipsecnode->m_snext;
	}
	else
		node = Z_Calloc(sizeof (*node), PU_LEVEL, NULL);
	return node;
}

// P_PutSecnode() returns a node to the freelist.

static inline void P_PutSecnode(msecnode_t *node)
{
	node->m_snext = headsecnode;
	headsecnode = node;
}

// Tails 08-25-2002
static inline void P_PutPrecipSecnode(mprecipsecnode_t *node)
{
	node->m_snext = headprecipsecnode;
	headprecipsecnode = node;
}

// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.

static msecnode_t *P_AddSecnode(sector_t *s, mobj_t *thing, msecnode_t *nextnode)
{
	msecnode_t *node;

	node = nextnode;
	while (node)
	{
		if (node->m_sector == s) // Already have a node for this sector?
		{
			node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
			return nextnode;
		}
		node = node->m_tnext;
	}

	// Couldn't find an existing node for this sector. Add one at the head
	// of the list.

	node = P_GetSecnode();

	// mark new nodes unvisited.
	node->visited = 0;

	node->m_sector = s; // sector
	node->m_thing = thing; // mobj
	node->m_tprev = NULL; // prev node on Thing thread
	node->m_tnext = nextnode; // next node on Thing thread
	if (nextnode)
		nextnode->m_tprev = node; // set back link on Thing

	// Add new node at head of sector thread starting at s->touching_thinglist

	node->m_sprev = NULL; // prev node on sector thread
	node->m_snext = s->touching_thinglist; // next node on sector thread
	if (s->touching_thinglist)
		node->m_snext->m_sprev = node;
	s->touching_thinglist = node;
	return node;
}

// More crazy crap Tails 08-25-2002
static mprecipsecnode_t *P_AddPrecipSecnode(sector_t *s, precipmobj_t *thing, mprecipsecnode_t *nextnode)
{
	mprecipsecnode_t *node;

	node = nextnode;
	while (node)
	{
		if (node->m_sector == s) // Already have a node for this sector?
		{
			node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
			return nextnode;
		}
		node = node->m_tnext;
	}

	// Couldn't find an existing node for this sector. Add one at the head
	// of the list.

	node = P_GetPrecipSecnode();

	// mark new nodes unvisited.
	node->visited = 0;

	node->m_sector = s; // sector
	node->m_thing = thing; // mobj
	node->m_tprev = NULL; // prev node on Thing thread
	node->m_tnext = nextnode; // next node on Thing thread
	if (nextnode)
		nextnode->m_tprev = node; // set back link on Thing

	// Add new node at head of sector thread starting at s->touching_thinglist

	node->m_sprev = NULL; // prev node on sector thread
	node->m_snext = s->touching_preciplist; // next node on sector thread
	if (s->touching_preciplist)
		node->m_snext->m_sprev = node;
	s->touching_preciplist = node;
	return node;
}

// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

static msecnode_t *P_DelSecnode(msecnode_t *node)
{
	msecnode_t *tp; // prev node on thing thread
	msecnode_t *tn; // next node on thing thread
	msecnode_t *sp; // prev node on sector thread
	msecnode_t *sn; // next node on sector thread

	if (!node)
		return NULL;

	// Unlink from the Thing thread. The Thing thread begins at
	// sector_list and not from mobj_t->touching_sectorlist.

	tp = node->m_tprev;
	tn = node->m_tnext;
	if (tp)
		tp->m_tnext = tn;
	if (tn)
		tn->m_tprev = tp;

	// Unlink from the sector thread. This thread begins at
	// sector_t->touching_thinglist.

	sp = node->m_sprev;
	sn = node->m_snext;
	if (sp)
		sp->m_snext = sn;
	else
		node->m_sector->touching_thinglist = sn;
	if (sn)
		sn->m_sprev = sp;

	// Return this node to the freelist

	P_PutSecnode(node);
	return tn;
}

// Tails 08-25-2002
static mprecipsecnode_t *P_DelPrecipSecnode(mprecipsecnode_t *node)
{
	mprecipsecnode_t *tp; // prev node on thing thread
	mprecipsecnode_t *tn; // next node on thing thread
	mprecipsecnode_t *sp; // prev node on sector thread
	mprecipsecnode_t *sn; // next node on sector thread

	if (!node)
		return NULL;

	// Unlink from the Thing thread. The Thing thread begins at
	// sector_list and not from mobj_t->touching_sectorlist.

	tp = node->m_tprev;
	tn = node->m_tnext;
	if (tp)
		tp->m_tnext = tn;
	if (tn)
		tn->m_tprev = tp;

	// Unlink from the sector thread. This thread begins at
	// sector_t->touching_thinglist.

	sp = node->m_sprev;
	sn = node->m_snext;
	if (sp)
		sp->m_snext = sn;
	else
		node->m_sector->touching_preciplist = sn;
	if (sn)
		sn->m_sprev = sp;

	// Return this node to the freelist

	P_PutPrecipSecnode(node);
	return tn;
}

// Delete an entire sector list
void P_DelSeclist(msecnode_t *node)
{
	while (node)
		node = P_DelSecnode(node);
}

// Tails 08-25-2002
void P_DelPrecipSeclist(mprecipsecnode_t *node)
{
	while (node)
		node = P_DelPrecipSecnode(node);
}

// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

static inline boolean PIT_GetSectors(line_t *ld)
{
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	return true;

	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;

	// This line crosses through the object.

	// Collect the sector(s) from the line and add to the
	// sector_list you're examining. If the Thing ends up being
	// allowed to move to this position, then the sector_list
	// will be attached to the Thing's mobj_t at touching_sectorlist.

	sector_list = P_AddSecnode(ld->frontsector,tmthing,sector_list);

	// Don't assume all lines are 2-sided, since some Things
	// like MT_TFOG are allowed regardless of whether their radius takes
	// them beyond an impassable linedef.

	// Use sidedefs instead of 2s flag to determine two-sidedness.
	if (ld->backsector)
		sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

	return true;
}

// Tails 08-25-2002
static inline boolean PIT_GetPrecipSectors(line_t *ld)
{
	if (preciptmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		preciptmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		preciptmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		preciptmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	return true;

	if (P_BoxOnLineSide(preciptmbbox, ld) != -1)
		return true;

	// This line crosses through the object.

	// Collect the sector(s) from the line and add to the
	// sector_list you're examining. If the Thing ends up being
	// allowed to move to this position, then the sector_list
	// will be attached to the Thing's mobj_t at touching_sectorlist.

	precipsector_list = P_AddPrecipSecnode(ld->frontsector, tmprecipthing, precipsector_list);

	// Don't assume all lines are 2-sided, since some Things
	// like MT_TFOG are allowed regardless of whether their radius takes
	// them beyond an impassable linedef.

	// Use sidedefs instead of 2s flag to determine two-sidedness.
	if (ld->backsector)
		precipsector_list = P_AddPrecipSecnode(ld->backsector, tmprecipthing, precipsector_list);

	return true;
}

// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y)
{
	INT32 xl, xh, yl, yh, bx, by;
	msecnode_t *node = sector_list;
	mobj_t *saved_tmthing = tmthing; /* cph - see comment at func end */
	fixed_t saved_tmx = tmx, saved_tmy = tmy; /* ditto */

	// First, clear out the existing m_thing fields. As each node is
	// added or verified as needed, m_thing will be set properly. When
	// finished, delete all nodes where m_thing is still NULL. These
	// represent the sectors the Thing has vacated.

	while (node)
	{
		node->m_thing = NULL;
		node = node->m_tnext;
	}

	tmthing = thing;
	tmflags = thing->flags;

	tmx = x;
	tmy = y;

	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;

	validcount++; // used to make sure we only process a line once

	xl = (unsigned)(tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (unsigned)(tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (unsigned)(tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (unsigned)(tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			P_BlockLinesIterator(bx, by, PIT_GetSectors);

	// Add the sector of the (x, y) point to sector_list.
	sector_list = P_AddSecnode(thing->subsector->sector, thing, sector_list);

	// Now delete any nodes that won't be used. These are the ones where
	// m_thing is still NULL.
	node = sector_list;
	while (node)
	{
		if (!node->m_thing)
		{
			if (node == sector_list)
				sector_list = node->m_tnext;
			node = P_DelSecnode(node);
		}
		else
			node = node->m_tnext;
	}

	/* cph -
	* This is the strife we get into for using global variables. tmthing
	*  is being used by several different functions calling
	*  P_BlockThingIterator, including functions that can be called *from*
	*  P_BlockThingIterator. Using a global tmthing is not reentrant.
	* OTOH for Boom/MBF demos we have to preserve the buggy behavior.
	*  Fun. We restore its previous value unless we're in a Boom/MBF demo.
	*/
	tmthing = saved_tmthing;

	/* And, duh, the same for tmx/y - cph 2002/09/22
	* And for tmbbox - cph 2003/08/10 */
	tmx = saved_tmx, tmy = saved_tmy;

	if (tmthing)
	{
		tmbbox[BOXTOP]  = tmy + tmthing->radius;
		tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
		tmbbox[BOXRIGHT]  = tmx + tmthing->radius;
		tmbbox[BOXLEFT]   = tmx - tmthing->radius;
	}
}

// More crazy crap Tails 08-25-2002
void P_CreatePrecipSecNodeList(precipmobj_t *thing,fixed_t x,fixed_t y)
{
	INT32 xl, xh, yl, yh, bx, by;
	mprecipsecnode_t *node = precipsector_list;
	precipmobj_t *saved_tmthing = tmprecipthing; /* cph - see comment at func end */

	// First, clear out the existing m_thing fields. As each node is
	// added or verified as needed, m_thing will be set properly. When
	// finished, delete all nodes where m_thing is still NULL. These
	// represent the sectors the Thing has vacated.

	while (node)
	{
		node->m_thing = NULL;
		node = node->m_tnext;
	}

	tmprecipthing = thing;
	preciptmflags = thing->flags;

	preciptmx = x;
	preciptmy = y;

	preciptmbbox[BOXTOP] = y + 2*FRACUNIT;
	preciptmbbox[BOXBOTTOM] = y - 2*FRACUNIT;
	preciptmbbox[BOXRIGHT] = x + 2*FRACUNIT;
	preciptmbbox[BOXLEFT] = x - 2*FRACUNIT;

	validcount++; // used to make sure we only process a line once

	xl = (unsigned)(preciptmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (unsigned)(preciptmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (unsigned)(preciptmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (unsigned)(preciptmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			P_BlockLinesIterator(bx, by, PIT_GetPrecipSectors);

	// Add the sector of the (x, y) point to sector_list.
	precipsector_list = P_AddPrecipSecnode(thing->subsector->sector, thing, precipsector_list);

	// Now delete any nodes that won't be used. These are the ones where
	// m_thing is still NULL.
	node = precipsector_list;
	while (node)
	{
		if (!node->m_thing)
		{
			if (node == precipsector_list)
				precipsector_list = node->m_tnext;
			node = P_DelPrecipSecnode(node);
		}
		else
			node = node->m_tnext;
	}

	/* cph -
	* This is the strife we get into for using global variables. tmthing
	*  is being used by several different functions calling
	*  P_BlockThingIterator, including functions that can be called *from*
	*  P_BlockThingIterator. Using a global tmthing is not reentrant.
	* OTOH for Boom/MBF demos we have to preserve the buggy behavior.
	*  Fun. We restore its previous value unless we're in a Boom/MBF demo.
	*/
	tmprecipthing = saved_tmthing;
}

/* cphipps 2004/08/30 -
 * Must clear tmthing at tic end, as it might contain a pointer to a removed thinker, or the level might have ended/been ended and we clear the objects it was pointing too. Hopefully we don't need to carry this between tics for sync. */
void P_MapStart(void)
{
	if (tmthing)
		I_Error("P_MapStart: tmthing set!");
}

void P_MapEnd(void)
{
	tmthing = NULL;
}

// P_FloorzAtPos
// Returns the floorz of the XYZ position
// Tails 05-26-2003
fixed_t P_FloorzAtPos(fixed_t x, fixed_t y, fixed_t z, fixed_t height)
{
	sector_t *sec = R_PointInSubsector(x, y)->sector;
	fixed_t floorz = sec->floorheight;

	// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
	if (sec->ffloors)
	{
		ffloor_t *rover;
		fixed_t delta1, delta2, thingtop = z + height;

		for (rover = sec->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS))
				continue;

			if ((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) || (rover->flags & FF_SWIMMABLE)))
				continue;

			if (rover->flags & FF_QUICKSAND)
			{
				if (z < *rover->topheight && *rover->bottomheight < thingtop)
				{
					floorz = z;
					continue;
				}
			}

			delta1 = z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			if (*rover->topheight > floorz && abs(delta1) < abs(delta2))
				floorz = *rover->topheight;
		}
	}

	return floorz;
}

//
// P_FakeZMovement
//
// Fake the zmovement so that we can check if a move is legal
//
static void P_FakeZMovement(mobj_t *mo)
{
	fixed_t dist, delta;

	if (!mo->health)
		return;

	// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
	if (mo->subsector->sector->ffloors)
	{
		ffloor_t *rover;
		fixed_t delta1, delta2;
		fixed_t thingtop = mo->z + mo->height;

		for (rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS))
				continue;
/*
			if ((rover->flags & FF_SWIMMABLE) && GETSECSPECIAL(rover->master->frontsector->special, 1) == 3
				&&  ((rover->master->flags & ML_EFFECT3)
				|| thing->z-thing->momz > *rover->topheight - 16*FRACUNIT))
				;
			else */if ((!((((rover->flags & FF_BLOCKPLAYER) && mo->player)
				|| ((rover->flags & FF_BLOCKOTHERS) && !mo->player)) || rover->flags & FF_QUICKSAND) && !(mo->player && !(mo->player->pflags & PF_NIGHTSMODE) && !mo->player->homing && (((mo->player->charability == CA_SWIM) || mo->player->powers[pw_super]) && mo->ceilingz-*rover->topheight >= mo->height) && (rover->flags & FF_SWIMMABLE) && !(mo->player->pflags & PF_SPINNING) && mo->player->speed > mo->player->runspeed && /*mo->ceilingz - *rover->topheight >= mo->height && */mo->z < *rover->topheight + 30*FRACUNIT && mo->z > *rover->topheight - 30*FRACUNIT)))
				continue;

			if (rover->flags & FF_QUICKSAND)
			{
				if (mo->z < *rover->topheight && *rover->bottomheight < thingtop)
				{
					mo->floorz = mo->z;
				}
				continue; // This is so you can jump/spring up through quicksand from below.
			}

			delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			if (*rover->topheight > mo->floorz && abs(delta1) < abs(delta2)
				&& (!(rover->flags & FF_REVERSEPLATFORM)))
			{
				mo->floorz = *rover->topheight;
			}
			if (*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2)
				&& (/*mo->z + mo->height <= *rover->bottomheight ||*/ !(rover->flags & FF_PLATFORM)))
			{
				mo->ceilingz = *rover->bottomheight;
			}
		}
	}

	// adjust height
	mo->z += mo->momz;
	if (mo->flags & MF_FLOAT && mo->target && mo->type != MT_JETJAW && mo->type != MT_EGGMOBILE
		&& mo->type != MT_EGGMOBILE2 && mo->type != MT_RING && mo->type != MT_COIN) // Tails
	{ // float down towards target if too close
		if (!(mo->flags2&MF2_SKULLFLY) && !(mo->flags2&MF2_INFLOAT))
		{
			dist = P_AproxDistance(mo->x-mo->target->x, mo->y-mo->target->y);
			delta = (mo->target->z + (mo->height>>1)) - mo->z;
			if (delta < 0 && dist < -(delta*3))
				mo->z -= FLOATSPEED;
			else if (delta > 0 && dist < (delta*3))
				mo->z += FLOATSPEED;
		}
	}

	// clip movement
	if (mo->z <= mo->floorz && !(mo->flags & MF_NOCLIPHEIGHT))
	{ // Hit the floor
		mo->z = mo->floorz;
		if (mo->momz < 0)
			mo->momz = 0;

		if (mo->flags2 & MF2_SKULLFLY) // The skull slammed into something
			mo->momz = -mo->momz;
	}
	else if (!(mo->flags & MF_NOGRAVITY))
	{
		P_CheckGravity (mo, true);
	}

	if (mo->z + mo->height > mo->ceilingz && !(mo->flags & MF_NOCLIPHEIGHT)) // hit the ceiling
	{
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
		if (mo->flags2 & MF2_SKULLFLY) // the skull slammed into something
			mo->momz = -mo->momz;
	}
}

// P_CheckOnmobj
//
// Checks if the new Z position is legal
//
mobj_t *P_CheckOnmobj(mobj_t *thing)
{
	subsector_t *newsubsec;
	fixed_t x = thing->x, y = thing->y;
	mobj_t oldmo = *thing;

	tmthing = thing;
	tmflags = thing->flags;
	P_FakeZMovement(tmthing);

	tmx = x;
	tmy = y;

	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;

	newsubsec = R_PointInSubsector(x, y);
	ceilingline = NULL;

	//
	// the base floor / ceiling is from the subsector that contains the
	// point. Any contacted lines the step closer together will adjust them
	//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;

	validcount++;

	//Exempt ghosts from momentum truncation. Jazz 2/21/09
	if (tmflags & MF_NOCLIP && tmthing->type != MT_GHOST)
		return NULL;

	*tmthing = oldmo;
	return NULL;
}
