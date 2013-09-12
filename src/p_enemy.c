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
/// \brief Enemy thinking, AI
///
///	Action Pointer Functions that are associated with states/frames

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "m_misc.h"
#include "r_things.h"
#include "dstrings.h"

#ifdef HW3SOUND
#include "hardware/hw3sound.h"
#endif

player_t *stplyr;
INT32 var1;
INT32 var2;

typedef enum
{
	DI_NODIR = -1,
	DI_EAST = 0,
	DI_NORTHEAST = 1,
	DI_NORTH = 2,
	DI_NORTHWEST = 3,
	DI_WEST = 4,
	DI_SOUTHWEST = 5,
	DI_SOUTH = 6,
	DI_SOUTHEAST = 7,
	NUMDIRS = 8,
} dirtype_t;

//
// P_NewChaseDir related LUT.
//
static dirtype_t opposite[] =
{
	DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
	DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

static dirtype_t diags[] =
{
	DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};

//Real Prototypes to A_*
void A_Fall(mobj_t *actor);
void A_Look(mobj_t *actor);
void A_Chase(mobj_t *actor);
void A_JetJawRoam(mobj_t *actor);
void A_JetJawChomp(mobj_t *actor);
void A_PointyThink(mobj_t *actor);
void A_CheckBuddy(mobj_t *actor);
void A_HoodThink(mobj_t *actor);
void A_ArrowCheck(mobj_t *actor);
void A_SnailerThink(mobj_t *actor);
void A_SharpChase(mobj_t *actor);
void A_SharpSpin(mobj_t *actor);
void A_VultureVtol(mobj_t *actor);
void A_VultureCheck(mobj_t *actor);
void A_SkimChase(mobj_t *actor);
void A_FaceTarget(mobj_t *actor);
void A_LobShot(mobj_t *actor);
void A_CannonLook(mobj_t *actor);
void A_FireShot(mobj_t *actor);
void A_SuperFireShot(mobj_t *actor);
void A_BossFireShot(mobj_t *actor);
void A_SkullAttack(mobj_t *actor);
void A_BossZoom(mobj_t *actor);
void A_BossScream(mobj_t *actor);
void A_Scream(mobj_t *actor);
void A_Pain(mobj_t *actor);
void A_1upThinker(mobj_t *actor);
void A_MonitorPop(mobj_t *actor);
void A_Explode(mobj_t *actor);
void A_BossDeath(mobj_t *actor);
void A_CustomPower(mobj_t *actor);
void A_GiveWeapon(mobj_t *actor);
void A_JumpShield(mobj_t *actor);
void A_RingShield(mobj_t *actor);
void A_RingBox(mobj_t *actor);
void A_Invincibility(mobj_t *actor);
void A_SuperSneakers(mobj_t *actor);
void A_ExtraLife(mobj_t *actor);
void A_BombShield(mobj_t *actor);
void A_WaterShield(mobj_t *actor);
void A_ForceShield(mobj_t *actor);
void A_GravityBox(mobj_t *actor);
void A_ScoreRise(mobj_t *actor);
void A_ParticleSpawn(mobj_t *actor);
void A_BunnyHop(mobj_t *actor);
void A_BubbleSpawn(mobj_t *actor);
void A_BubbleRise(mobj_t *actor);
void A_BubbleCheck(mobj_t *actor);
void A_AttractChase(mobj_t *actor);
void A_DropMine(mobj_t *actor);
void A_FishJump(mobj_t *actor);
void A_ThrownRing(mobj_t *actor);
void A_GrenadeRing(mobj_t *actor);
void A_SetSolidSteam(mobj_t *actor);
void A_UnsetSolidSteam(mobj_t *actor);
void A_SignPlayer(mobj_t *actor);
void A_JetChase(mobj_t *actor);
void A_JetbThink(mobj_t *actor);
void A_JetgShoot(mobj_t *actor);
void A_JetgThink(mobj_t *actor);
void A_ShootBullet(mobj_t *actor);
void A_MinusDigging(mobj_t *actor);
void A_MinusPopup(mobj_t *actor);
void A_MinusCheck(mobj_t *actor);
void A_ChickenCheck(mobj_t *actor);
void A_MouseThink(mobj_t *actor);
void A_DetonChase(mobj_t *actor);
void A_CapeChase(mobj_t *actor);
void A_RotateSpikeBall(mobj_t *actor);
void A_MaceRotate(mobj_t *actor);
void A_RockSpawn(mobj_t *actor);
void A_SnowBall(mobj_t *actor);
void A_CrawlaCommanderThink(mobj_t *actor);
void A_RingExplode(mobj_t *actor);
void A_OldRingExplode(mobj_t *actor);
void A_MixUp(mobj_t *actor);
void A_RecyclePowers(mobj_t *actor);
void A_Invinciblerize(mobj_t *actor);
void A_DeInvinciblerize(mobj_t *actor);
void A_GoopSplat(mobj_t *actor);
void A_Boss2PogoSFX(mobj_t *actor);
void A_EggmanBox(mobj_t *actor);
void A_TurretFire(mobj_t *actor);
void A_SuperTurretFire(mobj_t *actor);
void A_TurretStop(mobj_t *actor);
void A_SparkFollow(mobj_t *actor);
void A_BuzzFly(mobj_t *actor);
void A_GuardChase(mobj_t *actor);
void A_SetReactionTime(mobj_t *actor);
void A_Boss3TakeDamage(mobj_t *actor);
void A_LinedefExecute(mobj_t *actor);
void A_PlaySeeSound(mobj_t *actor);
void A_PlayAttackSound(mobj_t *actor);
void A_PlayActiveSound(mobj_t *actor);
void A_SmokeTrailer(mobj_t *actor);
void A_SpawnObjectAbsolute(mobj_t *actor);
void A_SpawnObjectRelative(mobj_t *actor);
void A_ChangeAngleRelative(mobj_t *actor);
void A_ChangeAngleAbsolute(mobj_t *actor);
void A_PlaySound(mobj_t *actor);
void A_FindTarget(mobj_t *actor);
void A_FindTracer(mobj_t *actor);
void A_SetTics(mobj_t *actor);
void A_SetRandomTics(mobj_t *actor);
void A_ChangeColorRelative(mobj_t *actor);
void A_ChangeColorAbsolute(mobj_t *actor);
void A_MoveRelative(mobj_t *actor);
void A_MoveAbsolute(mobj_t *actor);
void A_Thrust(mobj_t *actor);
void A_ZThrust(mobj_t *actor);
void A_SetTargetsTarget(mobj_t *actor);
void A_SetObjectFlags(mobj_t *actor);
void A_SetObjectFlags2(mobj_t *actor);
void A_RandomState(mobj_t *actor);
void A_RandomStateRange(mobj_t *actor);
void A_DualAction(mobj_t *actor);
void A_RemoteAction(mobj_t *actor);
void A_ToggleFlameJet(mobj_t *actor);
//for p_enemy.c
void A_Boss1Chase(mobj_t *actor);
void A_Boss2Chase(mobj_t *actor);
void A_Boss2Pogo(mobj_t *actor);
void A_BossJetFume(mobj_t *actor);

typedef fixed_t TVector[4];
typedef fixed_t TMatrix[4][4];

static TVector *VectorMatrixMultiply(TVector v, TMatrix m)
{
	static TVector ret;

	ret[0] = FixedMul(v[0],m[0][0]) + FixedMul(v[1],m[1][0]) + FixedMul(v[2],m[2][0]) + FixedMul(v[3],m[3][0]);
	ret[1] = FixedMul(v[0],m[0][1]) + FixedMul(v[1],m[1][1]) + FixedMul(v[2],m[2][1]) + FixedMul(v[3],m[3][1]);
	ret[2] = FixedMul(v[0],m[0][2]) + FixedMul(v[1],m[1][2]) + FixedMul(v[2],m[2][2]) + FixedMul(v[3],m[3][2]);
	ret[3] = FixedMul(v[0],m[0][3]) + FixedMul(v[1],m[1][3]) + FixedMul(v[2],m[2][3]) + FixedMul(v[3],m[3][3]);

	return &ret;
}

static TMatrix *RotateXMatrix(angle_t rad)
{
	static TMatrix ret;
	const angle_t fa = rad>>ANGLETOFINESHIFT;
	const fixed_t cosrad = FINECOSINE(fa), sinrad = FINESINE(fa);

	ret[0][0] = FRACUNIT; ret[0][1] =       0; ret[0][2] = 0;        ret[0][3] = 0;
	ret[1][0] =        0; ret[1][1] =  cosrad; ret[1][2] = sinrad;   ret[1][3] = 0;
	ret[2][0] =        0; ret[2][1] = -sinrad; ret[2][2] = cosrad;   ret[2][3] = 0;
	ret[3][0] =        0; ret[3][1] =       0; ret[3][2] = 0;        ret[3][3] = FRACUNIT;

	return &ret;
}

#if 0
static TMatrix *RotateYMatrix(angle_t rad)
{
	static TMatrix ret;
	const angle_t fa = rad>>ANGLETOFINESHIFT;
	const fixed_t cosrad = FINECOSINE(fa), sinrad = FINESINE(fa);

	ret[0][0] = cosrad;   ret[0][1] =        0; ret[0][2] = -sinrad;   ret[0][3] = 0;
	ret[1][0] = 0;        ret[1][1] = FRACUNIT; ret[1][2] = 0;         ret[1][3] = 0;
	ret[2][0] = sinrad;   ret[2][1] =        0; ret[2][2] = cosrad;    ret[2][3] = 0;
	ret[3][0] = 0;        ret[3][1] =        0; ret[3][2] = 0;         ret[3][3] = FRACUNIT;

	return &ret;
}
#endif

static TMatrix *RotateZMatrix(angle_t rad)
{
	static TMatrix ret;
	const angle_t fa = rad>>ANGLETOFINESHIFT;
	const fixed_t cosrad = FINECOSINE(fa), sinrad = FINESINE(fa);

	ret[0][0] = cosrad;    ret[0][1] = sinrad;   ret[0][2] =        0; ret[0][3] = 0;
	ret[1][0] = -sinrad;   ret[1][1] = cosrad;   ret[1][2] =        0; ret[1][3] = 0;
	ret[2][0] = 0;         ret[2][1] = 0;        ret[2][2] = FRACUNIT; ret[2][3] = 0;
	ret[3][0] = 0;         ret[3][1] = 0;        ret[3][2] =        0; ret[3][3] = FRACUNIT;

	return &ret;
}

//
// ENEMY THINKING
// Enemies are always spawned with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players, but some can be made preaware.
//

//
// P_CheckMeleeRange
//
static boolean P_CheckMeleeRange(mobj_t *actor)
{
	mobj_t *pl;
	fixed_t dist;

	if (!actor->target)
		return false;

	pl = actor->target;
	dist = P_AproxDistance(pl->x-actor->x, pl->y-actor->y);

	switch (actor->type)
	{
		case MT_JETTBOMBER:
			if (dist >= (actor->radius + pl->radius)*2)
				return false;
			break;
		case MT_FACESTABBER:
			if (dist >= (actor->radius + pl->radius)*4)
				return false;
			break;
		default:
			if (dist >= MELEERANGE - 20*FRACUNIT + pl->radius)
				return false;
			break;
	}

	// check height now, so that damn crawlas cant attack
	// you if you stand on a higher ledge.
	if (actor->type == MT_JETTBOMBER)
	{
		if (pl->z + pl->height > actor->z - (40<<FRACBITS))
			return false;
	}
	else if (actor->type == MT_SKIM)
	{
		if (pl->z + pl->height > actor->z - (24<<FRACBITS))
			return false;
	}
	else
	{
		if ((pl->z > actor->z + actor->height) || (actor->z > pl->z + pl->height))
			return false;

		if (actor->type != MT_JETTBOMBER && actor->type != MT_SKIM
			&& !P_CheckSight(actor, actor->target))
		{
			return false;
		}
	}

	return true;
}

//
// P_CheckMissileRange
//
static boolean P_CheckMissileRange(mobj_t *actor)
{
	fixed_t dist;

	if (!P_CheckSight(actor, actor->target))
		return false;

	if (actor->reactiontime)
		return false; // do not attack yet

	// OPTIMIZE: get this from a global checksight
	dist = P_AproxDistance(actor->x-actor->target->x, actor->y-actor->target->y) - 64*FRACUNIT;

	if (!actor->info->meleestate)
		dist -= 128*FRACUNIT; // no melee attack, so fire more

	dist >>= 16;

	if (actor->type == MT_EGGMOBILE)
		dist >>= 1;

	if (dist > 200)
		dist = 200;

	if (actor->type == MT_EGGMOBILE && dist > 160)
		dist = 160;

	if (P_Random() < dist)
		return false;

	return true;
}

/** Checks for water in a sector.
  * Used by Skim movements.
  *
  * \param x X coordinate on the map.
  * \param y Y coordinate on the map.
  * \return True if there's water at this location, false if not.
  * \sa ::MT_SKIM
  */
static boolean P_WaterInSector(mobj_t *mobj, fixed_t x, fixed_t y)
{
	sector_t *sector;
	fixed_t height = -1;

	sector = R_PointInSubsector(x, y)->sector;

	if (sector->ffloors)
	{
		ffloor_t *rover;

		for (rover = sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS))
				continue;

			if (rover->flags & FF_SWIMMABLE)
			{
				if (*rover->topheight >= mobj->floorz
					&& *rover->topheight <= mobj->z)
					height = *rover->topheight;
			}
		}
	}

	if (height != -1)
		return true;

	return false;
}

static const fixed_t xspeed[NUMDIRS] = {FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000};
static const fixed_t yspeed[NUMDIRS] = {0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000};

/** Moves an actor in its current direction.
  *
  * \param actor Actor object to move.
  * \return False if the move is blocked, otherwise true.
  */
static boolean P_Move(mobj_t *actor, fixed_t speed)
{
	fixed_t tryx, tryy;
	dirtype_t movedir = actor->movedir;

	if (movedir == DI_NODIR || !actor->health)
		return false;

	I_Assert((unsigned)movedir < 8);

	tryx = actor->x + speed*xspeed[movedir];
	tryy = actor->y + speed*yspeed[movedir];

	if (actor->type == MT_SKIM && !P_WaterInSector(actor, tryx, tryy)) // bail out if sector lacks water
		return false;

	if (!P_TryMove(actor, tryx, tryy, false))
	{
		if (actor->flags & MF_FLOAT && floatok)
		{
			// must adjust height
			if (actor->z < tmfloorz)
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;

			if (actor->type == MT_JETJAW && actor->z + actor->height > actor->watertop)
				actor->z = actor->watertop - actor->height;

			actor->flags2 |= MF2_INFLOAT;
			return true;
		}

		return false;
	}
	else
		actor->flags2 &= ~MF2_INFLOAT;

	return true;
}

/** Attempts to move an actor on in its current direction.
  * If the move succeeds, the actor's move count is reset
  * randomly to a value from 0 to 15.
  *
  * \param actor Actor to move.
  * \return True if the move succeeds, false if the move is blocked.
  */
static boolean P_TryWalk(mobj_t *actor)
{
	if (!P_Move(actor, actor->info->speed))
		return false;
	actor->movecount = P_Random() & 15;
	return true;
}

static void P_NewChaseDir(mobj_t *actor)
{
	fixed_t deltax, deltay;
	dirtype_t d[3];
	dirtype_t tdir = DI_NODIR, olddir, turnaround;

#ifdef PARANOIA
	if (!actor->target)
		I_Error("P_NewChaseDir: called with no target");
#endif

	olddir = actor->movedir;

	if (olddir >= NUMDIRS)
		olddir = DI_NODIR;

	if (olddir != DI_NODIR)
		turnaround = opposite[olddir];
	else
		turnaround = olddir;

	deltax = actor->target->x - actor->x;
	deltay = actor->target->y - actor->y;

	if (deltax > 10*FRACUNIT)
		d[1] = DI_EAST;
	else if (deltax < -10*FRACUNIT)
		d[1] = DI_WEST;
	else
		d[1] = DI_NODIR;

	if (deltay < -10*FRACUNIT)
		d[2] = DI_SOUTH;
	else if (deltay > 10*FRACUNIT)
		d[2] = DI_NORTH;
	else
		d[2] = DI_NODIR;

	// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		dirtype_t newdir = diags[((deltay < 0)<<1) + (deltax > 0)];

		actor->movedir = newdir;
		if ((newdir != turnaround) && P_TryWalk(actor))
			return;
	}

	// try other directions
	if (P_Random() > 200 || abs(deltay) > abs(deltax))
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] == turnaround)
		d[1] = DI_NODIR;
	if (d[2] == turnaround)
		d[2] = DI_NODIR;

	if (d[1] != DI_NODIR)
	{
		actor->movedir = d[1];

		if (P_TryWalk(actor))
			return; // either moved forward or attacked
	}

	if (d[2] != DI_NODIR)
	{
		actor->movedir = d[2];

		if (P_TryWalk(actor))
			return;
	}

	// there is no direct path to the player, so pick another direction.
	if (olddir != DI_NODIR)
	{
		actor->movedir =olddir;

		if (P_TryWalk(actor))
			return;
	}

	// randomly determine direction of search
	if (P_Random() & 1)
	{
		for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
		{
			if (tdir != turnaround)
			{
				actor->movedir = tdir;

				if (P_TryWalk(actor))
					return;
			}
		}
	}
	else
	{
		for (tdir = DI_SOUTHEAST; tdir >= DI_EAST; tdir--)
		{
			if (tdir != turnaround)
			{
				actor->movedir = tdir;

				if (P_TryWalk(actor))
					return;
			}
		}
	}

	if (turnaround != DI_NODIR)
	{
		actor->movedir = turnaround;

		if (P_TryWalk(actor))
			return;
	}

	actor->movedir = (angle_t)DI_NODIR; // cannot move
}

/** Looks for players to chase after, aim at, or whatever.
  *
  * \param actor     The object looking for flesh.
  * \param allaround Look all around? If false, only players in a 180-degree
  *                  range in front will be spotted.
  * \param dist      If > 0, checks distance
  * \return True if a player is found, otherwise false.
  * \sa P_SupermanLook4Players
  */
boolean P_LookForPlayers(mobj_t *actor, boolean allaround, boolean tracer, fixed_t dist)
{
	INT32 c = 0, stop;
	player_t *player;
	sector_t *sector;
	angle_t an;

	if (P_FreezeObjectplace())
		return false;

	sector = actor->subsector->sector;

	// BP: first time init, this allow minimum lastlook changes
	if (actor->lastlook < 0)
		actor->lastlook = P_Random();

	actor->lastlook %= MAXPLAYERS;

	stop = (actor->lastlook - 1) & PLAYERSMASK;

	for (; ; actor->lastlook = (actor->lastlook + 1) & PLAYERSMASK)
	{
		// done looking
		if (actor->lastlook == stop)
			return false;

		if (!playeringame[actor->lastlook])
			continue;

		if (c++ == 2)
			return false;

		player = &players[actor->lastlook];

		if (player->health <= 0)
			continue; // dead

		if (!player->mo)
			continue;

		if (!P_CheckSight(actor, player->mo))
			continue; // out of sight

		if ((netgame || multiplayer) && player->spectator)
			continue;

		if (dist > 0
			&& P_AproxDistance(P_AproxDistance(player->mo->x - actor->x, player->mo->y - actor->y), player->mo->z - actor->z) > dist)
			continue; // Too far away

		if (!allaround)
		{
			an = R_PointToAngle2(actor->x, actor->y, player->mo->x, player->mo->y) - actor->angle;
			if (an > ANGLE_90 && an < ANGLE_270)
			{
				dist = P_AproxDistance(player->mo->x - actor->x, player->mo->y - actor->y);
				// if real close, react anyway
				if (dist > MELEERANGE)
					continue; // behind back
			}
		}

		if (tracer)
			P_SetTarget(&actor->tracer, player->mo);
		else
			P_SetTarget(&actor->target, player->mo);
		return true;
	}

	//return false;
}

/** Looks for a player with a ring shield.
  * Used by rings.
  *
  * \param actor Ring looking for a shield to be attracted to.
  * \return True if a player with ring shield is found, otherwise false.
  * \sa A_AttractChase
  */
static boolean P_LookForShield(mobj_t *actor)
{
	INT32 c = 0, stop;
	player_t *player;
	sector_t *sector;

	sector = actor->subsector->sector;

	// BP: first time init, this allow minimum lastlook changes
	if (actor->lastlook < 0)
		actor->lastlook = P_Random();

	actor->lastlook %= MAXPLAYERS;

	stop = (actor->lastlook - 1) & PLAYERSMASK;

	for (; ; actor->lastlook = ((actor->lastlook + 1) & PLAYERSMASK))
	{
		// done looking
		if (actor->lastlook == stop)
			return false;

		if (!playeringame[actor->lastlook])
			continue;

		if (c++ == 2)
			return false;

		player = &players[actor->lastlook];

		if (player->health <= 0 || !player->mo)
			continue; // dead

		//When in CTF, don't pull rings that you cannot pick up.
		if ((actor->type == MT_REDTEAMRING && player->ctfteam != 1) ||
			(actor->type == MT_BLUETEAMRING && player->ctfteam != 2))
			continue;

		if (player->powers[pw_ringshield]
			&& (P_AproxDistance(P_AproxDistance(actor->x-player->mo->x, actor->y-player->mo->y), actor->z-player->mo->z) < RING_DIST))
		{
			P_SetTarget(&actor->tracer, player->mo);
			return true;
		}
	}

	//return false;
}

//
// ACTION ROUTINES
//

// Function: A_Look
//
// Description: Look for a player and set your target to them.
//
// var1:
//		lower 16 bits = look all around
//		upper 16 bits = distance limit
// var2 = If 1, only change to seestate. If 2, only play seesound. If 0, do both.
//
void A_Look(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!P_LookForPlayers(actor, locvar1 & 65535, false , (locvar1 >> 16)*FRACUNIT))
		return;

	// go into chase state
	if (!locvar2)
	{
		P_SetMobjState(actor, actor->info->seestate);
		A_PlaySeeSound(actor);
	}
	else if (locvar2 == 1) // Only go into seestate
		P_SetMobjState(actor, actor->info->seestate);
	else if (locvar2 == 2) // Only play seesound
		A_PlaySeeSound(actor);
}

// Function: A_Chase
//
// Description: Chase after your target.
//
// var1:
//		1 = don't check meleestate
//		2 = don't check missilestate
//		3 = don't check meleestate and missilestate
// var2 = unused
//
void A_Chase(mobj_t *actor)
{
	INT32 delta;
	INT32 locvar1 = var1;

	if (actor->reactiontime)
		actor->reactiontime--;

	// modify target threshold
	if (actor->threshold)
	{
		if (!actor->target || actor->target->health <= 0)
			actor->threshold = 0;
		else
			actor->threshold--;
	}

	// turn towards movement direction if not there yet
	if (actor->movedir < NUMDIRS)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANGLE_45;
		else if (delta < 0)
			actor->angle += ANGLE_45;
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		P_SetMobjStateNF(actor, actor->info->spawnstate);
		return;
	}

	// do not attack twice in a row
	if (actor->flags2 & MF2_JUSTATTACKED)
	{
		actor->flags2 &= ~MF2_JUSTATTACKED;
		P_NewChaseDir(actor);
		return;
	}

	// check for melee attack
	if (!(locvar1 & 1) && actor->info->meleestate && P_CheckMeleeRange(actor))
	{
		if (actor->info->attacksound)
			S_StartAttackSound(actor, actor->info->attacksound);

		P_SetMobjState(actor, actor->info->meleestate);
		return;
	}

	// check for missile attack
	if (!(locvar1 & 2) && actor->info->missilestate)
	{
		if (actor->movecount || !P_CheckMissileRange(actor))
			goto nomissile;

		P_SetMobjState(actor, actor->info->missilestate);
		actor->flags2 |= MF2_JUSTATTACKED;
		return;
	}

nomissile:
	// possibly choose another target
	if (multiplayer && !actor->threshold && (actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
		&& P_LookForPlayers(actor, true, false, 0))
		return; // got a new target

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor, actor->info->speed))
		P_NewChaseDir(actor);
}

// Function: A_JetJawRoam
//
// Description: Roaming routine for JetJaw
//
// var1 = unused
// var2 = unused
//
void A_JetJawRoam(mobj_t *actor)
{
	if (actor->reactiontime)
	{
		actor->reactiontime--;
		P_InstaThrust(actor, actor->angle, actor->info->speed*FRACUNIT/4);
	}
	else
	{
		actor->reactiontime = actor->info->reactiontime;
		actor->angle += ANGLE_180;
	}

	if (P_LookForPlayers(actor, false, false, actor->radius * 16))
		P_SetMobjState(actor, actor->info->seestate);
}

// Function: A_JetJawChomp
//
// Description: Chase and chomp at the target, as long as it is in view
//
// var1 = unused
// var2 = unused
//
void A_JetJawChomp(mobj_t *actor)
{
	INT32 delta;

	// turn towards movement direction if not there yet
	if (actor->movedir < NUMDIRS)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANGLE_45;
		else if (delta < 0)
			actor->angle += ANGLE_45;
	}

	// Stop chomping if target's dead or you can't see it
	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE)
		|| actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
	{
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor, actor->info->speed))
		P_NewChaseDir(actor);
}

// Function: A_PointyThink
//
// Description: Thinker function for Pointy
//
// var1 = unused
// var2 = unused
//
void A_PointyThink(mobj_t *actor)
{
	INT32 i;
	player_t *player = NULL;
	mobj_t *ball;
	TVector v;
	TVector *res;
	angle_t fa;
	fixed_t radius = actor->info->radius*actor->info->reactiontime;
	boolean firsttime = true;
	INT32 sign;

	actor->momx = actor->momy = actor->momz = 0;

	// Find nearest player
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		if (!players[i].mo)
			continue;

		if (!players[i].mo->health)
			continue;

		if (!P_CheckSight(actor, players[i].mo))
			continue;

		if (firsttime)
		{
			firsttime = false;
			player = &players[i];
		}
		else
		{
			if (P_AproxDistance(players[i].mo->x - actor->x, players[i].mo->y - actor->y) <
				P_AproxDistance(player->mo->x - actor->x, player->mo->y - actor->y))
				player = &players[i];
		}
	}

	if (!player)
		return;

	// Okay, we found the closest player. Let's move based on his movement.
	P_SetTarget(&actor->target, player->mo);
	A_FaceTarget(actor);

	if (P_AproxDistance(player->mo->x - actor->x, player->mo->y - actor->y) < P_AproxDistance(player->mo->x + player->mo->momx - actor->x, player->mo->y + player->mo->momy - actor->y))
		sign = -1; // Player is moving away
	else
		sign = 1; // Player is moving closer

	if (player->mo->momx || player->mo->momy)
	{
		P_InstaThrust(actor, R_PointToAngle2(actor->x, actor->y, player->mo->x, player->mo->y), actor->info->speed*sign);

		// Rotate our spike balls
		actor->lastlook += actor->info->damage;
		actor->lastlook %= FINEANGLES/4;
	}

	if (!actor->tracer) // For some reason we do not have spike balls...
		return;

	// Position spike balls relative to the value of 'lastlook'.
	ball = actor->tracer;

	i = 0;
	while (ball)
	{
		fa = actor->lastlook+i;
		v[0] = FixedMul(FINECOSINE(fa),radius);
		v[1] = 0;
		v[2] = FixedMul(FINESINE(fa),radius);
		v[3] = FRACUNIT;

		res = VectorMatrixMultiply(v, *RotateXMatrix(FixedAngle(actor->lastlook+i)));
		M_Memcpy(&v, res, sizeof (v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(actor->angle+ANGLE_180));
		M_Memcpy(&v, res, sizeof (v));

		P_UnsetThingPosition(ball);
		ball->x = actor->x + v[0];
		ball->y = actor->y + v[1];
		ball->z = actor->z + (actor->height>>1) + v[2];
		P_SetThingPosition(ball);

		ball = ball->tracer;
		i += ANGLE_90 >> ANGLETOFINESHIFT;
	}
}

// Function: A_CheckBuddy
//
// Description: Checks if target/tracer exists/has health. If not, the object removes itself.
//
// var1:
//		0 = target
//		1 = tracer
// var2 = unused
//
void A_CheckBuddy(mobj_t *actor)
{
	INT32 locvar1 = var1;

	if (locvar1 && (!actor->tracer || actor->tracer->health <= 0))
		P_SetMobjState(actor, S_DISS);
	else if (!locvar1 && (!actor->target || actor->target->health <= 0))
		P_SetMobjState(actor, S_DISS);
}

// Function: A_HoodThink
//
// Description: Thinker for Robo-Hood
//
// var1 = unused
// var2 = unused
//
void A_HoodThink(mobj_t *actor)
{
	// Currently in the air...
	if (actor->z > actor->floorz)
	{
		if (actor->momz > 0)
			P_SetMobjStateNF(actor, actor->info->xdeathstate); // Rising
		else
			P_SetMobjStateNF(actor, actor->info->raisestate); // Falling

		return;
	}

	if (actor->state == &states[actor->info->xdeathstate]
		|| actor->state == &states[actor->info->raisestate])
		P_SetMobjStateNF(actor, actor->info->seestate);

	if (!actor->target)
	{
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	A_FaceTarget(actor); // Aiming... aiming...

	if (--actor->reactiontime > 0)
		return;

	// Shoot, if not too close (cheap shots are lame)
	if (P_AproxDistance(actor->x-actor->target->x, actor->y-actor->target->y) > 64*FRACUNIT)
	{
		P_SetMobjState(actor, actor->info->missilestate);

		if (actor->spawnpoint && (actor->spawnpoint->options & MTF_AMBUSH)) // Don't jump
			actor->state->nextstate = actor->info->seestate;
	}
	else if (!(actor->flags & MF_AMBUSH))// But we WILL jump!
	{
		P_SetMobjState(actor, actor->info->painstate);
		var1 = 8;
		var2 = 5;
		A_BunnyHop(actor);
	}

	actor->reactiontime = actor->info->reactiontime;
}

// Function: A_ArrowCheck
//
// Description: Checks arrow direction and adjusts sprite accordingly
//
// var1 = unused
// var2 = unused
//
void A_ArrowCheck(mobj_t *actor)
{
	fixed_t x,y,z;
	angle_t angle;
	fixed_t dist;

	// Movement vector
	x = actor->momx;
	y = actor->momy;
	z = actor->momz;

	// Calculate the angle of movement.
	/*
	       Z
	     / |
	   /   |
	 /     |
	0------dist(X,Y)
	*/

	dist = P_AproxDistance(x, y);

	angle = R_PointToAngle2(0, 0, dist, z);

	if (angle > ANG20 && angle <= ANGLE_180)
		P_SetMobjStateNF(actor, actor->info->raisestate);
	else if (angle < ANG340 && angle > ANGLE_180)
		P_SetMobjStateNF(actor, actor->info->xdeathstate);
	else
		P_SetMobjStateNF(actor, actor->info->spawnstate);
}

// Function: A_SnailerThink
//
// Description: Thinker function for Snailer
//
// var1 = unused
// var2 = unused
//
void A_SnailerThink(mobj_t *actor)
{
	if (!actor->target)
	{
		// look for a new target
		if (!P_LookForPlayers(actor, true, false, 0))
			return;
	}

	// We now have a target. Oh bliss, rapture, and contentment!

	if (actor->target->z > actor->z - 32*FRACUNIT
		&& actor->target->z < actor->z + actor->height + 32*FRACUNIT
		&& !(leveltime % (TICRATE*2)))
	{
		var1 = MT_ROCKET;
		var2 = 0;
		A_FireShot(actor);
	}

	if (actor->target->z > actor->z)
		actor->momz += actor->info->speed;
	else if (actor->target->z < actor->z)
		actor->momz -= actor->info->speed;

	actor->momz /= 2;
}

// Function: A_SharpChase
//
// Description: Thinker/Chase routine for Sharps
//
// var1 = unused
// var2 = unused
//
void A_SharpChase(mobj_t *actor)
{
	if (!actor->health)
	{
		P_SetMobjState(actor, actor->info->deathstate);
		return;
	}

	if (actor->reactiontime)
	{
		INT32 delta;

		actor->reactiontime--;

		// turn towards movement direction if not there yet
		if (actor->movedir < NUMDIRS)
		{
			actor->angle &= (7<<29);
			delta = actor->angle - (actor->movedir << 29);

			if (delta > 0)
				actor->angle -= ANGLE_45;
			else if (delta < 0)
				actor->angle += ANGLE_45;
		}

		if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
		{
			// look for a new target
			if (P_LookForPlayers(actor, true, false, 0))
				return; // got a new target

			P_SetMobjState(actor, actor->info->spawnstate);
			return;
		}

		// chase towards player
		if (--actor->movecount < 0 || !P_Move(actor, actor->info->speed))
			P_NewChaseDir(actor);
	}
	else
	{
		actor->threshold = actor->info->painchance;
		P_SetMobjState(actor, actor->info->missilestate);
		S_StartSound(actor, actor->info->attacksound);
	}
}

// Function: A_SharpSpin
//
// Description: Spin chase routine for Sharps
//
// var1 = unused
// var2 = unused
//
void A_SharpSpin(mobj_t *actor)
{
	if (!actor->health)
	{
		P_SetMobjState(actor, actor->info->deathstate);
		return;
	}

	if (actor->threshold && actor->target)
	{
		actor->angle += ANGLE_22h;
		P_Thrust(actor, R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y), actor->info->speed*FRACUNIT);
		actor->threshold--;
	}
	else
	{
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
		actor->reactiontime = actor->info->reactiontime;
		P_SetMobjState(actor, actor->info->spawnstate);
		var1 = 1;
		A_Look(actor);
	}
}

// Function: A_VultureVtol
//
// Description: Vulture rising up to match target's height
//
// var1 = unused
// var2 = unused
//
void A_VultureVtol(mobj_t *actor)
{
	if (!actor->target)
		return;

	actor->flags |= MF_NOGRAVITY;
	actor->flags |= MF_FLOAT;

	A_FaceTarget(actor);

	S_StopSound(actor);

	if (actor->z < actor->target->z+(actor->target->height/4) && actor->z + actor->height < actor->ceilingz)
		actor->momz = 2*FRACUNIT;
	else if (actor->z > (actor->target->z+(actor->target->height/4)*3) && actor->z > actor->floorz)
		actor->momz = -2*FRACUNIT;
	else
	{
		// Attack!
		actor->momz = 0;
		P_SetMobjState(actor, actor->info->missilestate);
		S_StartSound(actor, actor->info->activesound);
	}
}

// Function: A_VultureCheck
//
// Description: If the vulture is stopped, look for a new target
//
// var1 = unused
// var2 = unused
//
void A_VultureCheck(mobj_t *actor)
{
	if (actor->momx || actor->momy)
		return;

	actor->flags &= ~MF_NOGRAVITY; // Fall down

	if (actor->z <= actor->floorz)
	{
		actor->angle -= ANGLE_180; // turn around
		P_SetMobjState(actor, actor->info->spawnstate);
	}
}

// Function: A_SkimChase
//
// Description: Thinker/Chase routine for Skims
//
// var1 = unused
// var2 = unused
//
void A_SkimChase(mobj_t *actor)
{
	INT32 delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	// modify target threshold
	if (actor->threshold)
	{
		if (!actor->target || actor->target->health <= 0)
			actor->threshold = 0;
		else
			actor->threshold--;
	}

	// turn towards movement direction if not there yet
	if (actor->movedir < NUMDIRS)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANGLE_45;
		else if (delta < 0)
			actor->angle += ANGLE_45;
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		P_LookForPlayers(actor, true, false, 0);

		// the spawnstate for skims already calls this function so just return either way
		// without changing state
		return;
	}

	// do not attack twice in a row
	if (actor->flags2 & MF2_JUSTATTACKED)
	{
		actor->flags &= ~MF2_JUSTATTACKED;
		P_NewChaseDir(actor);
		return;
	}

	// check for melee attack
	if (actor->info->meleestate && P_CheckMeleeRange(actor))
	{
		if (actor->info->attacksound)
			S_StartAttackSound(actor, actor->info->attacksound);

		P_SetMobjState(actor, actor->info->meleestate);
		return;
	}

	// check for missile attack
	if (actor->info->missilestate)
	{
		if (actor->movecount || !P_CheckMissileRange(actor))
			goto nomissile;

		P_SetMobjState(actor, actor->info->missilestate);
		actor->flags2 |= MF2_JUSTATTACKED;
		return;
	}

nomissile:
	// possibly choose another target
	if (multiplayer && !actor->threshold && (actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
		&& P_LookForPlayers(actor, true, false, 0))
		return; // got a new target

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor, actor->info->speed))
		P_NewChaseDir(actor);
}

// Function: A_FaceTarget
//
// Description: Immediately turn to face towards your target.
//
// var1 = unused
// var2 = unused
//
void A_FaceTarget(mobj_t *actor)
{
	if (!actor->target)
		return;

	actor->flags &= ~MF_AMBUSH;

	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
}

// Function: A_LobShot
//
// Description: Lob an object at your target.
//
// var1 = object # to lob
// var2:
//		var2 >> 16 = height offset
//		var2 & 65535 = airtime
//
void A_LobShot(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2 >> 16;
	mobj_t *shot, *hitspot;
	angle_t an;
	fixed_t dist;
	fixed_t vertical, horizontal;
	fixed_t airtime = var2 & 65535;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	shot = P_SpawnMobj(actor->x, actor->y, actor->z + locvar2*FRACUNIT, locvar1);

	// Keep track of where it's going to land
	hitspot = P_SpawnMobj(actor->target->x&(64*FRACUNIT-1), actor->target->y&(64*FRACUNIT-1), actor->target->subsector->sector->floorheight, MT_DISS);
	hitspot->tics = airtime;
	P_SetTarget(&shot->tracer, hitspot);

	P_SetTarget(&shot->target, actor); // where it came from

	shot->angle = an = actor->angle;
	an >>= ANGLETOFINESHIFT;

	dist = P_AproxDistance(actor->target->x - shot->x, actor->target->y - shot->y);

	horizontal = dist / airtime;
	vertical = FIXEDSCALE((gravity*airtime)/2, shot->scale);

	shot->momx = FixedMul(horizontal, FINECOSINE(an));
	shot->momy = FixedMul(horizontal, FINESINE(an));
	shot->momz = vertical;

/* Try to adjust when destination is not the same height
	if (actor->z != actor->target->z)
	{
		fixed_t launchhyp;
		fixed_t diff;
		fixed_t orig;

		diff = actor->z - actor->target->z;
		{
			launchhyp = P_AproxDistance(horizontal, vertical);

			orig = FixedMul(FixedDiv(vertical, horizontal), diff);

			CONS_Printf("orig: %d\n", (orig)>>FRACBITS);

			horizontal = dist / airtime;
			vertical = (gravity*airtime)/2;
		}
		dist -= orig;
		shot->momx = FixedMul(horizontal, FINECOSINE(an));
		shot->momy = FixedMul(horizontal, FINESINE(an));
		shot->momz = vertical;
*/

	if (shot->info->seesound)
		S_StartSound(shot, shot->info->seesound);

	if (!(actor->flags & MF_BOSS))
	{
		if (ultimatemode)
			actor->reactiontime = actor->info->reactiontime*TICRATE;
		else
			actor->reactiontime = actor->info->reactiontime*TICRATE*2;
	}
}

// Function: A_CannonLook
//
// Description: Looks for closest player, with a z offset.
//
// var1 = unused
// var2 = unused
//
void A_CannonLook(mobj_t *actor)
{
	P_LookForPlayers(actor, true, false , 16384*FRACUNIT);
}

// Function: A_FireShot
//
// Description: Shoot an object at your target.
//
// var1 = object # to shoot
// var2 = height offset
//
void A_FireShot(mobj_t *actor)
{
	fixed_t z;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	z = actor->z + 48*FRACUNIT + locvar2*FRACUNIT;

	P_SpawnXYZMissile(actor, actor->target, locvar1, actor->x, actor->y, z);

	if (!(actor->flags & MF_BOSS))
	{
		if (ultimatemode)
			actor->reactiontime = actor->info->reactiontime*TICRATE;
		else
			actor->reactiontime = actor->info->reactiontime*TICRATE*2;
	}
}

// Function: A_SuperFireShot
//
// Description: Shoot an object at your target that will even stall Super Sonic.
//
// var1 = object # to shoot
// var2 = height offset
//
void A_SuperFireShot(mobj_t *actor)
{
	fixed_t z;
	mobj_t *mo;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	z = actor->z + 48*FRACUNIT + locvar2*FRACUNIT;

	mo = P_SpawnXYZMissile(actor, actor->target, locvar1, actor->x, actor->y, z);

	if (mo)
		mo->flags2 |= MF2_SUPERFIRE;

	if (!(actor->flags & MF_BOSS))
	{
		if (ultimatemode)
			actor->reactiontime = actor->info->reactiontime*TICRATE;
		else
			actor->reactiontime = actor->info->reactiontime*TICRATE*2;
	}
}

// Function: A_BossFireShot
//
// Description: Shoot an object at your target ala Bosses:
//
// var1 = object # to shoot
// var2:
//		0 - Boss 1 Left side
//		1 - Boss 1 Right side
//		2 - Boss 3 Left side upper
//		3 - Boss 3 Left side lower
//		4 - Boss 3 Right side upper
//		5 - Boss 3 Right side lower
//
void A_BossFireShot(mobj_t *actor)
{
	fixed_t x, y, z;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	switch (locvar2)
	{
		case 0:
			x = actor->x + P_ReturnThrustX(actor, actor->angle-ANGLE_90, 43*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle-ANGLE_90, 43*FRACUNIT);
			z = actor->z + 48*FRACUNIT;
			break;
		case 1:
			x = actor->x + P_ReturnThrustX(actor, actor->angle+ANGLE_90, 43*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle+ANGLE_90, 43*FRACUNIT);
			z = actor->z + 48*FRACUNIT;
			break;
		case 2:
			x = actor->x + P_ReturnThrustX(actor, actor->angle-ANGLE_90, 56*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle-ANGLE_90, 56*FRACUNIT);
			z = actor->z + 42*FRACUNIT;
			break;
		case 3:
			x = actor->x + P_ReturnThrustX(actor, actor->angle-ANGLE_90, 58*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle-ANGLE_90, 58*FRACUNIT);
			z = actor->z + 30*FRACUNIT;
			break;
		case 4:
			x = actor->x + P_ReturnThrustX(actor, actor->angle+ANGLE_90, 56*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle+ANGLE_90, 56*FRACUNIT);
			z = actor->z + 42*FRACUNIT;
			break;
		case 5:
			x = actor->x + P_ReturnThrustX(actor, actor->angle+ANGLE_90, 58*FRACUNIT);
			y = actor->y + P_ReturnThrustY(actor, actor->angle+ANGLE_90, 58*FRACUNIT);
			z = actor->z + 30*FRACUNIT;
			break;
		default:
			x = actor->x;
			y = actor->y;
			z = actor->z + actor->height/2;
			break;
	}

	P_SpawnXYZMissile(actor, actor->target, locvar1, x, y, z);
}

// Function: A_SkullAttack
//
// Description: Fly at the player like a missile.
//
// var1:
//		0 - Fly at the player
//		1 - Fly away from the player
//		2 - Strafe in relation to the player
// var2 = unused
//
#define SKULLSPEED (20*FRACUNIT)

void A_SkullAttack(mobj_t *actor)
{
	mobj_t *dest;
	angle_t an;
	INT32 dist;
	INT32 speed;
	INT32 locvar1 = var1;
	//INT32 locvar2 = var2;

	if (!actor->target)
		return;

	speed = SKULLSPEED;

	dest = actor->target;
	actor->flags2 |= MF2_SKULLFLY;
	if (actor->info->activesound)
		S_StartSound(actor, actor->info->activesound);
	A_FaceTarget(actor);

	if (locvar1 == 1)
		actor->angle += ANGLE_180;
	else if (locvar1 == 2)
	{
		if (P_Random() & 1)
			actor->angle += ANGLE_90;
		else
			actor->angle -= ANGLE_90;
	}

	an = actor->angle >> ANGLETOFINESHIFT;

	actor->momx = FixedMul(speed, FINECOSINE(an));
	actor->momy = FixedMul(speed, FINESINE(an));
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	dist = dist / speed;

	if (dist < 1)
		dist = 1;

	actor->momz = (dest->z + (dest->height>>1) - actor->z) / dist;

	if (locvar1 == 1)
		actor->momz = -actor->momz;
}

// Function: A_BossZoom
//
// Description: Like A_SkullAttack, but used by Boss 1.
//
// var1 = unused
// var2 = unused
//
void A_BossZoom(mobj_t *actor)
{
	mobj_t *dest;
	angle_t an;
	INT32 dist;

	if (!actor->target)
		return;

	dest = actor->target;
	actor->flags2 |= MF2_SKULLFLY;
	if (actor->info->attacksound)
		S_StartAttackSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed*5*FRACUNIT, FINECOSINE(an));
	actor->momy = FixedMul(actor->info->speed*5*FRACUNIT, FINESINE(an));
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	dist = dist / (actor->info->speed*5*FRACUNIT);

	if (dist < 1)
		dist = 1;
	actor->momz = (dest->z + (dest->height>>1) - actor->z) / dist;
}

// Function: A_BossScream
//
// Description: Spawns explosions and plays appropriate sounds around the defeated boss.
//
// var1 = unused
// var2 = unused
//
void A_BossScream(mobj_t *actor)
{
	fixed_t x, y, z;
	angle_t fa;

	actor->movecount += actor->info->speed*16;
	actor->movecount %= 360;
	fa = (FixedAngle(actor->movecount*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = actor->x + FixedMul(FINECOSINE(fa),actor->radius);
	y = actor->y + FixedMul(FINESINE(fa),actor->radius);

	z = actor->z - 8*FRACUNIT + (P_Random()<<(FRACBITS-2));
	if (actor->info->deathsound) S_StartSound(P_SpawnMobj(x, y, z, MT_BOSSEXPLODE), actor->info->deathsound);
}

// Function: A_Scream
//
// Description: Starts the death sound of the object.
//
// var1 = unused
// var2 = unused
//
void A_Scream(mobj_t *actor)
{
	if (actor->tracer && (actor->tracer->type == MT_SHELL || actor->tracer->type == MT_FIREBALL))
		S_StartScreamSound(actor, sfx_lose);
	else if (actor->info->deathsound)
		S_StartScreamSound(actor, actor->info->deathsound);
}

// Function: A_Pain
//
// Description: Starts the pain sound of the object.
//
// var1 = unused
// var2 = unused
//
void A_Pain(mobj_t *actor)
{
	if (actor->info->painsound)
		S_StartSound(actor, actor->info->painsound);
}

// Function: A_Fall
//
// Description: Changes a dying object's flags to reflect its having fallen to the ground.
//
// var1 = unused
// var2 = unused
//
void A_Fall(mobj_t *actor)
{
	// actor is on ground, it can be walked over
	actor->flags &= ~MF_SOLID;

	actor->flags |= MF_NOCLIP;
	actor->flags |= MF_NOGRAVITY;
	actor->flags |= MF_FLOAT;

	// So change this if corpse objects
	// are meant to be obstacles.
}

#define LIVESBOXDISPLAYPLAYER // Use displayplayer instead of closest player

// Function: A_1upThinker
//
// Description: Used by the 1up box to show the player's face.
//
// var1 = unused
// var2 = unused
//
void A_1upThinker(mobj_t *actor)
{
	#ifdef LIVESBOXDISPLAYPLAYER
	if (!splitscreen)
	{
		actor->frame = states[S_PLAY_BOX1A].frame;
		actor->skin = &skins[players[displayplayer].skin];
	}
	else
	{
		INT32 i;
		fixed_t dist = INT32_MAX;
		fixed_t temp;
		INT32 closestplayer = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (!players[i].mo)
				continue;

			temp = P_AproxDistance(players[i].mo->x-actor->x, players[i].mo->y-actor->y);

			if (temp < dist)
			{
				closestplayer = i;
				dist = temp;
			}
		}

		P_SetMobjStateNF(actor, S_PLAY_BOX1A);
		actor->skin = &skins[players[closestplayer].skin];
	}
	#else
	INT32 i;
	fixed_t dist = INT32_MAX;
	fixed_t temp;
	INT32 closestplayer = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		if (!players[i].mo)
			continue;

		temp = P_AproxDistance(players[i].mo->x-actor->x, players[i].mo->y-actor->y);

		if (temp < dist)
		{
			closestplayer = i;
			dist = temp;
		}
	}

	P_SetMobjStateNF(actor, S_PLAY_BOX1A);
	actor->skin = &skins[players[closestplayer].skin];
	#endif
}

// Function: A_MonitorPop
//
// Description: Used by monitors when they explode.
//
// var1 = unused
// var2 = unused
//
void A_MonitorPop(mobj_t *actor)
{
	mobj_t *remains;
	mobjtype_t item = 0;
	INT32 prandom;
	mobjtype_t newbox;

	// de-solidify
	P_UnsetThingPosition(actor);
	actor->flags &= ~MF_SOLID;
	actor->flags |= MF_NOCLIP;
	P_SetThingPosition(actor);

	remains = P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->speed);
	remains->type = actor->type; // Transfer type information
	P_UnsetThingPosition(remains);
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}
	remains->flags = actor->flags; // Transfer flags
	P_SetThingPosition(remains);
	remains->flags2 = actor->flags2; // Transfer flags2
	remains->fuse = actor->fuse; // Transfer respawn timer
	remains->threshold = 68;
	remains->skin = NULL;

	actor->flags2 |= MF2_BOSSNOTRAP; // Dummy flag to mark this as an exploded TV until it respawns
	tmthing = remains;

	if (actor->info->deathsound) S_StartSound(remains, actor->info->deathsound);

	switch (actor->type)
	{
		case MT_QUESTIONBOX: // Random!
		{
			mobjtype_t spawnchance[48];
			INT32 i = 0;
			INT32 oldi = 0;
			INT32 numchoices = 0;

			prandom = P_Random(); // Gotta love those random numbers!

			if (cv_superring.value)
			{
				oldi = i;

				for (; i < oldi + cv_superring.value; i++)
				{
					spawnchance[i] = MT_SUPERRINGBOX;
					numchoices++;
				}
			}
			if (cv_supersneakers.value)
			{
				oldi = i;

				for (; i < oldi + cv_supersneakers.value; i++)
				{
					spawnchance[i] = MT_SNEAKERTV;
					numchoices++;
				}
			}
			if (cv_invincibility.value)
			{
				oldi = i;

				for (; i < oldi + cv_invincibility.value; i++)
				{
					spawnchance[i] = MT_INV;
					numchoices++;
				}
			}
			if (cv_jumpshield.value)
			{
				oldi = i;

				for (; i < oldi + cv_jumpshield.value; i++)
				{
					spawnchance[i] = MT_WHITETV;
					numchoices++;
				}
			}
			if (cv_watershield.value)
			{
				oldi = i;

				for (; i < oldi + cv_watershield.value; i++)
				{
					spawnchance[i] = MT_GREENTV;
					numchoices++;
				}
			}
			if (cv_ringshield.value)
			{
				oldi = i;

				for (; i < oldi + cv_ringshield.value; i++)
				{
					spawnchance[i] = MT_YELLOWTV;
					numchoices++;
				}
			}
			if (cv_forceshield.value)
			{
				oldi = i;

				for (; i < oldi + cv_forceshield.value; i++)
				{
					spawnchance[i] = MT_BLUETV;
					numchoices++;
				}
			}
			if (cv_bombshield.value)
			{
				oldi = i;

				for (; i < oldi + cv_bombshield.value; i++)
				{
					spawnchance[i] = MT_BLACKTV;
					numchoices++;
				}
			}
			if (cv_1up.value)
			{
				oldi = i;

				for (; i < oldi + cv_1up.value; i++)
				{
					spawnchance[i] = MT_PRUP;
					numchoices++;
				}
			}
			if (cv_eggmanbox.value)
			{
				oldi = i;

				for (; i < oldi + cv_eggmanbox.value; i++)
				{
					spawnchance[i] = MT_EGGMANBOX;
					numchoices++;
				}
			}
			if (cv_teleporters.value)
			{
				oldi = i;

				for (; i < oldi + cv_teleporters.value; i++)
				{
					spawnchance[i] = MT_MIXUPBOX;
					numchoices++;
				}
			}

			if (cv_recycler.value)
			{
				oldi = i;

				for (; i < oldi + cv_recycler.value; i++)
				{
					spawnchance[i] = MT_RECYCLETV;
					numchoices++;
				}
			}

			if (numchoices == 0)
			{
				CONS_Printf("Note: All monitors turned off.\n");
				return;
			}

			newbox = spawnchance[prandom%numchoices];
			item = mobjinfo[newbox].damage;

			remains->flags &= ~MF_AMBUSH;
			break;
		}
		default:
			item = actor->info->damage;
			break;
	}

	if (item != 0)
	{
		mobj_t *newmobj;
		newmobj = P_SpawnMobj(actor->x, actor->y, actor->z + 13*FRACUNIT, item);

		P_SetTarget(&newmobj->target, actor->target); // Transfer target
		if (actor->eflags & MFE_VERTICALFLIP)
			newmobj->eflags |= MFE_VERTICALFLIP;

		if (item == MT_1UPICO && newmobj->target->player)
		{
			newmobj->skin = &skins[newmobj->target->player->skin];
			P_SetMobjState(newmobj, newmobj->info->spawnstate);
		}
	}
	else
		CONS_Printf("Powerup item not defined in 'damage' field for A_MonitorPop\n");

	P_RemoveMobj(actor);
}

// Function: A_Explode
//
// Description: Explodes an object, doing damage to any objects nearby. The target is used as the cause of the explosion. Damage value is used as amount of damage to be dealt.
//
// var1 = unused
// var2 = unused
//
void A_Explode(mobj_t *actor)
{
	P_RadiusAttack(actor, actor->target, actor->info->damage);
}

// Function: A_BossDeath
//
// Description: Possibly trigger special effects when boss dies.
//
// var1 = unused
// var2 = unused
//
void A_BossDeath(mobj_t *mo)
{
	thinker_t *th;
	mobj_t *mo2;
	line_t junk;
	INT32 i;

	if (mo->type == MT_EGGMOBILE || mo->type == MT_EGGMOBILE2)
	{
		if (mo->flags2 & MF2_CHAOSBOSS)
		{
			mo->health = 0;
			P_SetMobjState(mo, S_DISS);
			return;
		}
	}

	mo->health = 0;

	// make sure there is a player alive for victory
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && (players[i].health > 0
			|| ((netgame || multiplayer) && (players[i].lives > 0 || players[i].continues > 0))))
			break;

	if (i == MAXPLAYERS)
		return; // no one left alive, so do not end game

	// scan the remaining thinkers to see
	// if all bosses are dead
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;
		if (mo2 != mo && (mo2->flags & MF_BOSS) && mo2->health > 0)
			return; // other boss not dead
	}

	// victory!
	if (!mariomode)
	{
		if (mo->flags2 & MF2_BOSSNOTRAP)
		{
			for (i = 0; i < MAXPLAYERS; i++)
				P_DoPlayerExit(&players[i]);
		}
		else
		{
			// Bring the egg trap up to the surface
			junk.tag = 680;
			EV_DoElevator(&junk, elevateHighest, false);
			junk.tag = 681;
			EV_DoElevator(&junk, elevateUp, false);
			junk.tag = 682;
			EV_DoElevator(&junk, elevateHighest, false);
		}

		// Stop exploding and prepare to run.
		P_SetMobjState(mo, mo->info->xdeathstate);

		P_SetTarget(&mo->target, NULL);

		// Flee! Flee! Find a point to escape to! If none, just shoot upward!
		// scan the thinkers to find the runaway point
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;

			if (mo2->type == MT_BOSSFLYPOINT)
			{
				// If this one's closer then the last one, go for it.
				if (!mo->target ||
					P_AproxDistance(P_AproxDistance(mo->x - mo2->x, mo->y - mo2->y), mo->z - mo2->z) <
					P_AproxDistance(P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y), mo->z - mo->target->z))
						P_SetTarget(&mo->target, mo2);
				// Otherwise... Don't!
			}
		}

		mo->flags |= MF_NOGRAVITY|MF_NOCLIP;
		mo->flags |= MF_NOCLIPHEIGHT;

		if (mo->target)
		{
			mo->angle = R_PointToAngle2(mo->x, mo->y, mo->target->x, mo->target->y);
			mo->flags2 |= MF2_BOSSFLEE;
			mo->momz = FixedMul(FixedDiv(mo->target->z - mo->z, P_AproxDistance(mo->x-mo->target->x,mo->y-mo->target->y)), 2*FRACUNIT);
		}
		else
			mo->momz = 2*FRACUNIT;

		if (mo->type == MT_EGGMOBILE2)
		{
			mo2 = P_SpawnMobj(mo->x + P_ReturnThrustX(mo, mo->angle - ANGLE_90, 32*FRACUNIT),
				mo->y + P_ReturnThrustY(mo, mo->angle-ANGLE_90, 24*FRACUNIT),
				mo->z + mo->height/2 - 8*FRACUNIT, MT_BOSSTANK1); // Right tank
			mo2->angle = mo->angle;
			P_InstaThrust(mo2, mo2->angle - ANGLE_90, 4*FRACUNIT);
			mo2->momz = 4*FRACUNIT;

			mo2 = P_SpawnMobj(mo->x + P_ReturnThrustX(mo, mo->angle + ANGLE_90, 32*FRACUNIT),
				mo->y + P_ReturnThrustY(mo, mo->angle-ANGLE_90, 24*FRACUNIT),
				mo->z + mo->height/2 - 8*FRACUNIT, MT_BOSSTANK2); // Left tank
			mo2->angle = mo->angle;
			P_InstaThrust(mo2, mo2->angle + ANGLE_90, 4*FRACUNIT);
			mo2->momz = 4*FRACUNIT;

			P_SpawnMobj(mo->x, mo->y, mo->z + mo->height + 32*FRACUNIT, MT_BOSSSPIGOT)->momz = 4*FRACUNIT;
			return;
		}
	}
	else if (mariomode && mo->type == MT_KOOPA)
	{
		junk.tag = 650;
		EV_DoCeiling(&junk, raiseToHighest);
		return;
	}
}

// Function: A_CustomPower
//
// Description: Provides a custom powerup. Target (must be a player) is awarded the powerup. Reactiontime of the object is used as an index to the powers array.
//
// var1 = Power index #
// var2 = Power duration in tics
//
void A_CustomPower(mobj_t *actor)
{
	player_t *player;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	boolean spawnshield = false;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s", text[POWERUPNOTARGET]);
		return;
	}

	if (locvar1 >= NUMPOWERS)
	{
		CONS_Printf("Power #%d out of range!\n", locvar1);
		return;
	}

	player = actor->target->player;

	if ((locvar1 == pw_jumpshield || locvar1 == pw_forceshield //outliers!
		|| (locvar1 >= pw_ringshield && locvar1 <= pw_flameshield))
		&& !player->powers[locvar1])
		spawnshield = true;

	player->powers[locvar1] = locvar2;
	if (actor->info->seesound)
		S_StartSound(player->mo, actor->info->seesound);

	if (spawnshield) //workaround for a bug
		P_SpawnShieldOrb(player);
}

// Function: A_GiveWeapon
//
// Description: Gives the player the specified weapon panels.
//
// var1 = Weapon index #
// var2 = unused
//
void A_GiveWeapon(mobj_t *actor)
{
	player_t *player;
	INT32 locvar1 = var1;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s", text[POWERUPNOTARGET]);
		return;
	}

	if (locvar1 >= 64)
	{
		CONS_Printf("Weapon #%d out of range!\n", locvar1);
		return;
	}

	player = actor->target->player;

	player->ringweapons |= locvar1;
	if (actor->info->seesound)
		S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_JumpShield
//
// Description: Awards the player a jump shield.
//
// var1 = unused
// var2 = unused
//
void A_JumpShield(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s", text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	player->powers[pw_forceshield] = player->powers[pw_bombshield] = false;
	player->powers[pw_watershield] = player->powers[pw_ringshield] = false;

	if (!(player->powers[pw_jumpshield]))
	{
		player->powers[pw_jumpshield] = true;
		P_SpawnShieldOrb(player);
	}

	S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_RingShield
//
// Description: Awards the player a ring shield.
//
// var1 = unused
// var2 = unused
//
void A_RingShield(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s", text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	player->powers[pw_bombshield] = player->powers[pw_watershield] = false;
	player->powers[pw_forceshield] = player->powers[pw_jumpshield] = false;

	if (!(player->powers[pw_ringshield]))
	{
		player->powers[pw_ringshield] = true;
		P_SpawnShieldOrb(player);
	}

	S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_RingBox
//
// Description: Awards the player 10 rings.
//
// var1 = unused
// var2 = unused
//
void A_RingBox(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s", text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	P_GivePlayerRings(player, actor->info->reactiontime, false);
	if (actor->info->seesound)
		S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_Invincibility
//
// Description: Awards the player invincibility.
//
// var1 = unused
// var2 = unused
//
void A_Invincibility(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;
	player->powers[pw_invulnerability] = invulntics + 1;

	if (P_IsLocalPlayer(player) && !player->powers[pw_super])
	{
		S_StopMusic();
		if (mariomode)
			S_ChangeMusic(mus_minvnc, false);
		else
			S_ChangeMusic(mus_invinc, false);
	}
}

// Function: A_SuperSneakers
//
// Description: Awards the player super sneakers.
//
// var1 = unused
// var2 = unused
//
void A_SuperSneakers(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	actor->target->player->powers[pw_sneakers] = sneakertics + 1;

	if (P_IsLocalPlayer(player) && (!player->powers[pw_super]))
	{
		if (S_SpeedMusic(0.0f) && mapheaderinfo[gamemap-1].speedmusic)
			S_SpeedMusic(1.4f);
		else
		{
			S_StopMusic();
			S_ChangeMusic(mus_shoes, false);
		}
	}
}

// Function: A_ExtraLife
//
// Description: Awards the player an extra life.
//
// var1 = unused
// var2 = unused
//
void A_ExtraLife(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	// In shooter gametypes, give the player 100 rings instead of an extra life.
	if (gametype == GT_MATCH || gametype == GT_CTF || gametype == GT_TAG)
		P_GivePlayerRings(player, 100, false);
	else
		P_GivePlayerLives(player, 1);

	if (mariomode)
		S_StartSound(player->mo, sfx_marioa);
	else
	{
		player->powers[pw_extralife] = extralifetics + 1;

		if (P_IsLocalPlayer(player))
		{
			S_StopMusic();
			S_ChangeMusic(mus_xtlife, false);
		}
	}
}

// Function: A_BombShield
//
// Description: Awards the player a bomb shield.
//
// var1 = unused
// var2 = unused
//
void A_BombShield(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	player->powers[pw_watershield] = player->powers[pw_forceshield] = false;
	player->powers[pw_ringshield] = player->powers[pw_jumpshield] = false;

	if (!(player->powers[pw_bombshield]))
	{
		player->powers[pw_bombshield] = true;
		P_SpawnShieldOrb(player);
	}

	S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_WaterShield
//
// Description: Awards the player a water shield.
//
// var1 = unused
// var2 = unused
//
void A_WaterShield(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	player->powers[pw_bombshield] = player->powers[pw_forceshield] = false;
	player->powers[pw_ringshield] = player->powers[pw_jumpshield] = false;

	if (!(player->powers[pw_watershield]))
	{
		player->powers[pw_watershield] = true;
		P_SpawnShieldOrb(player);
	}

	if (player->powers[pw_underwater] && player->powers[pw_underwater] <= 12*TICRATE + 1)
		P_RestoreMusic(player);

	player->powers[pw_underwater] = 0;

	if (player->powers[pw_spacetime] > 1)
	{
		player->powers[pw_spacetime] = 0;
		P_RestoreMusic(player);
	}
	S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_ForceShield
//
// Description: Awards the player a force shield.
//
// var1 = unused
// var2 = unused
//
void A_ForceShield(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;

	player->powers[pw_bombshield] = player->powers[pw_watershield] = false;
	player->powers[pw_ringshield] = player->powers[pw_jumpshield] = false;

	if (!(player->powers[pw_forceshield]))
	{
		player->powers[pw_forceshield] = 2;
		P_SpawnShieldOrb(player);
	}
	else
		player->powers[pw_forceshield] = 2;

	S_StartSound(player->mo, actor->info->seesound);
}

// Function: A_GravityBox
//
// Description: Awards the player gravity boots.
//
// var1 = unused
// var2 = unused
//
void A_GravityBox(mobj_t *actor)
{
	player_t *player;

	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	player = actor->target->player;
	player->powers[pw_gravityboots] = gravbootstics + 1;

	S_StartSound(player, actor->info->activesound);
}

// Function: A_ScoreRise
//
// Description: Makes the little score logos rise. Speed value sets speed.
//
// var1 = unused
// var2 = unused
//
void A_ScoreRise(mobj_t *actor)
{
	actor->momz = actor->info->speed; // make logo rise!
}

// Function: A_ParticleSpawn
//
// Description: Spawns a particle at a specified interval
//
// var1 = unused
// var2 = unused
//
void A_ParticleSpawn(mobj_t *actor)
{
	fixed_t speed;
	mobjtype_t type;
	mobj_t *spawn;

	if (!actor->spawnpoint)
	{
		P_SetMobjState(actor, S_DISS);
		return;
	}

	type = MT_PARTICLE + (actor->spawnpoint->angle & 15);
	speed = (actor->spawnpoint->angle >> 12) << FRACBITS;

	spawn = P_SpawnMobj(actor->x, actor->y, actor->z, type);
	spawn->momz = speed;
	spawn->destscale = 1;
	spawn->scalespeed = (UINT8)((actor->spawnpoint->angle >> 8) & 63);
	actor->tics = actor->spawnpoint->extrainfo + 1;
}

// Function: A_BunnyHop
//
// Description: Makes object hop like a bunny.
//
// var1 = jump strength
// var2 = horizontal movement
//
void A_BunnyHop(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (actor->z <= actor->floorz)
	{
		actor->momz = locvar1*FRACUNIT; // make it hop!
		actor->angle += P_Random()*FINEANGLES;
		P_InstaThrust(actor, actor->angle, locvar2*FRACUNIT); // Launch the hopping action! PHOOM!!
	}
}

// Function: A_BubbleSpawn
//
// Description: Spawns a randomly sized bubble from the object's location. Only works underwater.
//
// var1 = unused
// var2 = unused
//
void A_BubbleSpawn(mobj_t *actor)
{
	UINT8 prandom;
	mobj_t *bubble = NULL;
	if (!(actor->eflags & MFE_UNDERWATER))
	{
		// Don't draw or spawn bubbles above water
		actor->flags2 |= MF2_DONTDRAW;
		return;
	}

	actor->flags2 &= ~MF2_DONTDRAW;
	prandom = P_Random();

	if (leveltime % (3*TICRATE) < 8)
		bubble = P_SpawnMobj(actor->x, actor->y, actor->z + (actor->height / 2), MT_EXTRALARGEBUBBLE);
	else if (prandom > 128)
		bubble = P_SpawnMobj(actor->x, actor->y, actor->z + (actor->height / 2), MT_SMALLBUBBLE);
	else if (prandom < 128 && prandom > 96)
		bubble = P_SpawnMobj(actor->x, actor->y, actor->z + (actor->height / 2), MT_MEDIUMBUBBLE);

	if (bubble)
	{
		bubble->destscale = actor->scale;
		P_SetScale(bubble,actor->scale);
	}
}

// Function: A_BubbleRise
//
// Description: Raises a bubble
//
// var1:
//		0 = Bend around the water abit, looking more realistic
//		1 = Rise straight up
// var2 = rising speed
//
void A_BubbleRise(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (actor->type == MT_EXTRALARGEBUBBLE)
	{
		actor->momz = FixedDiv(6*FRACUNIT,5*FRACUNIT); // make bubbles rise!
	}
	else
	{
		actor->momz += locvar2; // make bubbles rise!

		// Move around slightly to make it look like it's bending around the water

		if (!locvar1)
		{
			if (P_Random() < 32)
			{
				P_InstaThrust(actor, P_Random() & 1 ? actor->angle + ANGLE_90 : actor->angle,
					P_Random() & 1? FRACUNIT/2 : -FRACUNIT/2);
			}
			else if (P_Random() < 32)
			{
				P_InstaThrust(actor, P_Random() & 1 ? actor->angle - ANGLE_90 : actor->angle - ANGLE_180,
					P_Random() & 1? FRACUNIT/2 : -FRACUNIT/2);
			}
		}
	}
}

// Function: A_BubbleCheck
//
// Description: Checks if a bubble should be drawn or not. Bubbles are not drawn above water.
//
// var1 = unused
// var2 = unused
//
void A_BubbleCheck(mobj_t *actor)
{
	if (actor->eflags & MFE_UNDERWATER)
		actor->flags2 &= ~MF2_DONTDRAW; // underwater so draw
	else
		actor->flags2 |= MF2_DONTDRAW; // above water so don't draw
}

// Function: A_AttractChase
//
// Description: Makes a ring chase after a player with a ring shield and also causes spilled rings to flicker.
//
// var1 = unused
// var2 = unused
//
void A_AttractChase(mobj_t *actor)
{
	if (actor->flags2 & MF2_NIGHTSPULL)
		return;

	// spilled rings flicker before disappearing
	if (leveltime & 1 && actor->type == (mobjtype_t)actor->info->reactiontime && actor->fuse && actor->fuse < 2*TICRATE)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

	// Turn flingrings back into regular rings if attracted.
	if (actor->tracer && actor->tracer->player
		&& !actor->tracer->player->powers[pw_ringshield] && actor->info->reactiontime && actor->type != (mobjtype_t)actor->info->reactiontime)
	{
		mobj_t *newring;
		newring = P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->reactiontime);
		newring->momx = actor->momx;
		newring->momy = actor->momy;
		newring->momz = actor->momz;
		P_SetMobjState(actor, S_DISS);
	}

	P_LookForShield(actor); // Go find 'em, boy!

	if (!actor->tracer
		|| !actor->tracer->player
		|| !actor->tracer->health
		|| !P_CheckSight(actor, actor->tracer)) // You have to be able to SEE it...sorta
	{
		P_SetTarget(&actor->tracer, NULL);
		return;
	}

	// If a FlingRing gets attracted by a shield, change it into a normal
	// ring, but don't count towards the total.
	if (actor->type == (mobjtype_t)actor->info->reactiontime)
	{
		P_SetMobjState(actor, S_DISS);
		P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->painchance);
	}

	// Keep stuff from going down inside floors and junk
	actor->flags &= ~MF_NOCLIPHEIGHT;

	// Let attracted rings move through walls and such.
	actor->flags |= MF_NOCLIP;

	P_Attract(actor, actor->tracer, false);
}

// Function: A_DropMine
//
// Description: Drops a mine. Raisestate specifies the object # to use for the mine.
//
// var1 = height offset
// var2:
//		lower 16 bits = proximity check distance (0 disables)
//		upper 16 bits = 0 to check proximity with target, 1 for tracer
//
void A_DropMine(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (locvar2 & 65535)
	{
		fixed_t dist;
		mobj_t *target;

		if (locvar2 >> 16)
			target = actor->tracer;
		else
			target = actor->target;

		if (!target)
			return;

		dist = P_AproxDistance(actor->x-target->x, actor->y-target->y)>>FRACBITS;

		if (dist > (locvar2 & 65535))
			return;
	}

	// Use raisestate instead of MT_MINE
	P_SpawnMobj(actor->x, actor->y, actor->z - 12*FRACUNIT + (locvar1*FRACUNIT), actor->info->raisestate)
		->momz = actor->momz + actor->pmomz;

	S_StartSound(actor, actor->info->attacksound);
}

// Function: A_FishJump
//
// Description: Makes the stupid harmless fish in Greenflower Zone jump.
//
// var1 = Jump strength (in FRACBITS), if specified. Otherwise, uses the angle value.
// var2 = unused
//
void A_FishJump(mobj_t *actor)
{
	INT32 locvar1 = var1;

	if ((actor->z <= actor->floorz) || (actor->z <= actor->watertop - (64 << FRACBITS)))
	{
		fixed_t jumpval;

		if (locvar1)
			jumpval = var1;
		else
			jumpval = AngleFixed(actor->angle)/4;

		if (!jumpval) jumpval = 44*(FRACUNIT/4);
		actor->momz = jumpval;
		P_SetMobjStateNF(actor, actor->info->seestate);
	}

	if (actor->momz < 0
		&& (actor->state < &states[actor->info->meleestate] || actor->state > &states[actor->info->xdeathstate]))
		P_SetMobjStateNF(actor, actor->info->meleestate);
}

// Function:A_ThrownRing
//
// Description: Thinker for thrown rings/sparkle trail
//
// var1 = unused
// var2 = unused
//
void A_ThrownRing(mobj_t *actor)
{
	INT32 c = 0;
	INT32 stop;
	player_t *player;

	if (leveltime % (TICRATE/7) == 0)
	{
		mobj_t *ring = NULL;

		if (actor->flags2 & MF2_EXPLOSION)
			ring = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SMOK);
		else if (actor->flags2 & MF2_AUTOMATIC)
			ring = P_SpawnGhostMobj(actor);
		else if (!(actor->flags2 & MF2_RAILRING))
			ring = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPARK);
		else if (!(actor->flags2 & MF2_GRENADE))
			ring = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SUPERSPARK);

/*		if (ring)
		{
			P_SetTarget(&ring->target, actor);
			ring->color = actor->color; //copy color
		}*/
	}

	// decrement bounce ring time
	if (actor->flags2 & MF2_BOUNCERING)
	{
		if (actor->fuse)
			actor->fuse--;
		else
			P_SetMobjState(actor, S_DISS);
	}

	// spilled rings (and thrown bounce) flicker before disappearing
	if (leveltime & 1 && actor->fuse > 0 && actor->fuse < 2*TICRATE)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

	if (actor->tracer && actor->tracer->health <= 0)
		P_SetTarget(&actor->tracer, NULL);

	// Updated homing ring special capability
	// If you have a ring shield, all rings thrown
	// at you become homing (except rail)!
	if (actor->tracer)
	{
		// A non-homing ring getting attracted by a
		// magnetic player. If he gets too far away, make
		// sure to stop the attraction!
		if ((!actor->tracer->health) || (actor->tracer->player && actor->tracer->player->powers[pw_ringshield]
		    && P_AproxDistance(P_AproxDistance(actor->tracer->x-actor->x,
		    actor->tracer->y-actor->y), actor->tracer->z-actor->z) > RING_DIST/4))
		{
			P_SetTarget(&actor->tracer, NULL);
		}

		if (actor->tracer && (actor->tracer->health)
			&& (actor->tracer->player->powers[pw_ringshield]))// Already found someone to follow.
		{
			const INT32 temp = actor->threshold;
			actor->threshold = 32000;
			P_HomingAttack(actor, actor->tracer);
			actor->threshold = temp;
			return;
		}
	}

	// first time init, this allow minimum lastlook changes
	if (actor->lastlook < 0)
		actor->lastlook = P_Random();

	actor->lastlook %= MAXPLAYERS;

	stop = (actor->lastlook - 1) & PLAYERSMASK;

	for (; ; actor->lastlook = (actor->lastlook + 1) & PLAYERSMASK)
	{
		// done looking
		if (actor->lastlook == stop)
			return;

		if (!playeringame[actor->lastlook])
			continue;

		if (c++ == 2)
			return;

		player = &players[actor->lastlook];

		if (!player->mo)
			continue;

		if (player->mo->health <= 0)
			continue; // dead

		if ((netgame || multiplayer) && player->spectator)
			continue; // spectator

		if (actor->target && actor->target->player)
		{
			if (player->mo == actor->target)
				continue;

			// Don't home in on teammates.
			if (gametype == GT_CTF
				&& actor->target->player->ctfteam == player->ctfteam)
				continue;
		}

		// check distance
		if (actor->flags2 & MF2_RAILRING)
		{
			if (P_AproxDistance(P_AproxDistance(player->mo->x-actor->x,
				player->mo->y-actor->y), player->mo->z-actor->z) > RING_DIST/2)
			{
				continue;
			}
		}
		else if (P_AproxDistance(P_AproxDistance(player->mo->x-actor->x,
			player->mo->y-actor->y), player->mo->z-actor->z) > RING_DIST)
		{
			continue;
		}

		// do this after distance check because it's more computationally expensive
		if (!P_CheckSight(actor, player->mo))
			continue; // out of sight

		if ((player->powers[pw_ringshield] == true
		    && P_AproxDistance(P_AproxDistance(player->mo->x-actor->x,
		    player->mo->y-actor->y), player->mo->z-actor->z) < RING_DIST/4))
			P_SetTarget(&actor->tracer, player->mo);
		return;
	}

	return;
}

static mobj_t *grenade;

static inline boolean PIT_GrenadeRing(mobj_t *thing)
{
	if (!grenade)
		return true;

	if (thing->type != MT_PLAYER) // Don't explode for anything but an actual player.
		return true;

	if (thing == grenade->target) // Don't blow up at your owner.
		return true;

	if ((gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
		&& !cv_friendlyfire.value && grenade->target->player && thing->player
		&& grenade->target->player->ctfteam == thing->player->ctfteam) // Don't blow up at your teammates, unless friendlyfire is on
		return true;

	// see if it went over / under
	if (grenade->z > thing->z + thing->height)
		return true; // overhead
	if (grenade->z + grenade->height < thing->z)
		return true; // underneath

	if (netgame && thing->player && thing->player->spectator)
		return true;

	if (!(thing->flags & MF_SHOOTABLE))
	{
		// didn't do any damage
		return true;
	}

	if (P_AproxDistance(P_AproxDistance(thing->x - grenade->x, thing->y - grenade->y),
		thing->z - grenade->z) > grenade->info->painchance)
		return true; // Too far away

	// Explode!
	P_SetMobjState(grenade, grenade->info->deathstate);
	return false;
}

// Function:A_GrenadeRing
//
// Description: Thinker for thrown grenades
//
// var1 = unused
// var2 = unused
//
void A_GrenadeRing(mobj_t *actor)
{
	INT32 bx, by, xl, xh, yl, yh;
	const fixed_t explodedist = actor->info->painchance;

	if (leveltime % 35 == 0)
		S_StartSound(actor, actor->info->activesound);

	// Use blockmap to check for nearby shootables
	yh = (unsigned)(actor->y + explodedist - bmaporgy)>>MAPBLOCKSHIFT;
	yl = (unsigned)(actor->y - explodedist - bmaporgy)>>MAPBLOCKSHIFT;
	xh = (unsigned)(actor->x + explodedist - bmaporgx)>>MAPBLOCKSHIFT;
	xl = (unsigned)(actor->x - explodedist - bmaporgx)>>MAPBLOCKSHIFT;

	grenade = actor;

	for (by = yl; by <= yh; by++)
		for (bx = xl; bx <= xh; bx++)
			P_BlockThingsIterator(bx, by, PIT_GrenadeRing);
}

// Function: A_SetSolidSteam
//
// Description: Makes steam solid so it collides with the player to boost them.
//
// var1 = unused
// var2 = unused
//
void A_SetSolidSteam(mobj_t *actor)
{
	actor->flags &= ~MF_NOCLIP;
	actor->flags |= MF_SOLID;
	if (!(P_Random() % 8))
	{
		if (actor->info->deathsound)
			S_StartSound(actor, actor->info->deathsound); // Hiss!
	}
	else
	{
		if (actor->info->painsound)
			S_StartSound(actor, actor->info->painsound);
	}

	P_SetObjectMomZ (actor, 1, true);
}

// Function: A_UnsetSolidSteam
//
// Description: Makes an object non-solid and also noclip. Used by the steam.
//
// var1 = unused
// var2 = unused
//
void A_UnsetSolidSteam(mobj_t *actor)
{
	actor->flags &= ~MF_SOLID;
	actor->flags |= MF_NOCLIP;
}

// Function: A_SignPlayer
//
// Description: Changes the state of a level end sign to reflect the player that hit it.
//
// var1 = unused
// var2 = unused
//
void A_SignPlayer(mobj_t *actor)
{
	if (!actor->target)
		return;

	if (!actor->target->player)
		return;

	actor->skin = &skins[actor->target->player->skin];
	P_SetMobjState(actor, S_PLAY_SIGN);
}

// Function: A_JetChase
//
// Description: A_Chase for Jettysyns
//
// var1 = unused
// var2 = unused
//
void A_JetChase(mobj_t *actor)
{
	fixed_t thefloor;

	if (actor->flags & MF_AMBUSH)
		return;

	if (actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if (actor->reactiontime)
		actor->reactiontime--;

	if (P_Random() % 32 == 1)
	{
		actor->momx = actor->momx / 2;
		actor->momy = actor->momy / 2;
		actor->momz = actor->momz / 2;
	}

	// Bounce if too close to floor or ceiling -
	// ideal for Jetty-Syns above you on 3d floors
	if (actor->momz && ((actor->z - (32<<FRACBITS)) < thefloor) && !((thefloor + 32*FRACUNIT + actor->height) > actor->ceilingz))
		actor->momz = -actor->momz/2;

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		actor->momx = actor->momy = actor->momz = 0;
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	// modify target threshold
	if (actor->threshold)
	{
		if (!actor->target || actor->target->health <= 0)
			actor->threshold = 0;
		else
			actor->threshold--;
	}

	// turn towards movement direction if not there yet
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if ((multiplayer || netgame) && !actor->threshold && (actor->target->health <= 0 || !P_CheckSight(actor, actor->target)))
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

	// If the player is over 3072 fracunits away, then look for another player
	if (P_AproxDistance(P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y),
		actor->target->z - actor->z) > 3072*FRACUNIT && P_LookForPlayers(actor, true, false, 3072*FRACUNIT))
	{
		return; // got a new target
	}

	// chase towards player
	if (ultimatemode)
		P_Thrust(actor, actor->angle, actor->info->speed/2);
	else
		P_Thrust(actor, actor->angle, actor->info->speed/4);

	// must adjust height
	if (ultimatemode)
	{
		if (actor->z < (actor->target->z + actor->target->height + (64<<FRACBITS)))
			actor->momz += FRACUNIT/2;
		else
			actor->momz -= FRACUNIT/2;
	}
	else
	{
		if (actor->z < (actor->target->z + actor->target->height + (32<<FRACBITS)))
			actor->momz += FRACUNIT/2;
		else
			actor->momz -= FRACUNIT/2;
	}
}

// Function: A_JetbThink
//
// Description: Thinker for Jetty-Syn bombers
//
// var1 = unused
// var2 = unused
//
void A_JetbThink(mobj_t *actor)
{
	sector_t *nextsector;

	fixed_t thefloor;

	if (actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if (actor->target)
	{
		A_JetChase (actor);
		// check for melee attack
		if ((actor->z > (actor->floorz + (32<<FRACBITS)))
			&& P_CheckMeleeRange (actor) && !actor->reactiontime
			&& (actor->target->z >= actor->floorz))
		{
			if (actor->info->attacksound)
				S_StartAttackSound(actor, actor->info->attacksound);

			// use raisestate instead of MT_MINE
			P_SetTarget(&P_SpawnMobj(actor->x, actor->y, actor->z - (32<<FRACBITS), actor->info->raisestate)->target, actor);
			actor->reactiontime = TICRATE; // one second
			S_StartSound(actor, actor->info->attacksound);
		}
	}
	else if (((actor->z - (32<<FRACBITS)) < thefloor) && !((thefloor + (32<<FRACBITS) + actor->height) > actor->ceilingz))
			actor->z = thefloor+(32<<FRACBITS);

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;

	// Move downwards or upwards to go through a passageway.
	if (nextsector->ceilingheight < actor->height)
		actor->momz -= 5*FRACUNIT;
	else if (nextsector->floorheight > actor->z)
		actor->momz += 5*FRACUNIT;
}

// Function: A_JetgShoot
//
// Description: Firing function for Jetty-Syn gunners.
//
// var1 = unused
// var2 = unused
//
void A_JetgShoot(mobj_t *actor)
{
	fixed_t dist;

	if (!actor->target)
		return;

	if (actor->reactiontime)
		return;

	dist = P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y);

	if (dist > actor->info->painchance*FRACUNIT)
		return;

	if (dist < 64*FRACUNIT)
		return;

	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, actor->info->raisestate);

	if (ultimatemode)
		actor->reactiontime = actor->info->reactiontime*TICRATE;
	else
		actor->reactiontime = actor->info->reactiontime*TICRATE*2;

	if (actor->info->attacksound)
		S_StartSound(actor, actor->info->attacksound);
}

// Function: A_ShootBullet
//
// Description: Shoots a bullet. Raisestate defines object # to use as projectile.
//
// var1 = unused
// var2 = unused
//
void A_ShootBullet(mobj_t *actor)
{
	fixed_t dist;

	if (!actor->target)
		return;

	dist = P_AproxDistance(P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y), actor->target->z - actor->z);

	if (dist > actor->info->painchance*FRACUNIT)
		return;

	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, actor->info->raisestate);

	if (actor->info->attacksound)
		S_StartSound(actor, actor->info->attacksound);
}

// Function: A_MinusDigging
//
// Description: Minus digging in the ground.
//
// var1 = unused
// var2 = unused
//
void A_MinusDigging(mobj_t *actor)
{
	actor->flags &= ~MF_SPECIAL;
	actor->flags &= ~MF_SHOOTABLE;
	actor->flags &= ~MF_ENEMY;

	if (!actor->target)
	{
		A_Look(actor);
		return;
	}

	if (actor->reactiontime)
	{
		actor->reactiontime--;
		return;
	}

	// Dirt trail
	P_SpawnGhostMobj(actor);

	actor->flags |= MF_NOCLIPTHING;
	var1 = 3;
	A_Chase(actor);
	actor->flags &= ~MF_NOCLIPTHING;

	// Play digging sound
	if (!(leveltime & 15))
		S_StartSound(actor, actor->info->activesound);

	// If we're close enough to our target, pop out of the ground
	if (P_AproxDistance(actor->target->x-actor->x, actor->target->y-actor->y) < actor->radius
		&& abs(actor->target->z - actor->z) < actor->height)
		P_SetMobjState(actor, actor->info->missilestate);

	actor->z = actor->floorz; // Snap to ground
}

// Function: A_MinusPopup
//
// Description: Minus popping out of the ground.
//
// var1 = unused
// var2 = unused
//
void A_MinusPopup(mobj_t *actor)
{
	actor->momz = 10*FRACUNIT;

	actor->flags |= MF_SPECIAL;
	actor->flags |= MF_SHOOTABLE;
	actor->flags |= MF_ENEMY;

	// Sound for busting out of the ground.
	S_StartSound(actor, actor->info->attacksound);
}

// Function: A_MinusCheck
//
// Description: If the minus hits the floor, dig back into the ground.
//
// var1 = unused
// var2 = unused
//
void A_MinusCheck(mobj_t *actor)
{
	if (actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, actor->info->seestate);
		actor->flags &= ~MF_SPECIAL;
		actor->flags &= ~MF_SHOOTABLE;
		actor->flags &= ~MF_ENEMY;
		actor->reactiontime = TICRATE;
		return;
	}

	// 'Falling' animation
	if (actor->momz < 0 && actor->state < &states[actor->info->meleestate])
		P_SetMobjState(actor, actor->info->meleestate);
}

// Function: A_ChickenCheck
//
// Description: Resets the chicken once it hits the floor again.
//
// var1 = unused
// var2 = unused
//
void A_ChickenCheck(mobj_t *actor)
{
	if (actor->z <= actor->floorz)
	{
		if (!(actor->momx || actor->momy || actor->momz)
			&& actor->state > &states[actor->info->seestate])
		{
			A_Chase(actor);
			P_SetMobjState(actor, actor->info->seestate);
		}

		actor->momx >>= 2;
		actor->momy >>= 2;
	}

}

// Function: A_JetgThink
//
// Description: Thinker for Jetty-Syn Gunners
//
// var1 = unused
// var2 = unused
//
void A_JetgThink(mobj_t *actor)
{
	sector_t *nextsector;

	fixed_t thefloor;

	if (actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if (actor->target)
	{
		if (P_Random() <= 32 && !actor->reactiontime)
			P_SetMobjState(actor, actor->info->missilestate);
		else
			A_JetChase (actor);
	}
	else if (actor->z - (32<<FRACBITS) < thefloor && !(thefloor + (32<<FRACBITS)
		+ actor->height > actor->ceilingz))
	{
		actor->z = thefloor + (32<<FRACBITS);
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;

	// Move downwards or upwards to go through a passageway.
	if (nextsector->ceilingheight < actor->height)
		actor->momz -= 5*FRACUNIT;
	else if (nextsector->floorheight > actor->z)
		actor->momz += 5*FRACUNIT;
}

// Function: A_MouseThink
//
// Description: Thinker for scurrying mice.
//
// var1 = unused
// var2 = unused
//
void A_MouseThink(mobj_t *actor)
{
	if (actor->reactiontime)
		actor->reactiontime--;

	if (actor->z == actor->floorz && !actor->reactiontime)
	{
		if (P_Random() & 1)
			actor->angle += ANGLE_90;
		else
			actor->angle -= ANGLE_90;

		P_InstaThrust(actor, actor->angle, actor->info->speed);
		actor->reactiontime = TICRATE/5;
	}
}

// Function: A_DetonChase
//
// Description: Chases a Deton after a player.
//
// var1 = unused
// var2 = unused
//
void A_DetonChase(mobj_t *actor)
{
	angle_t exact;
	fixed_t xydist, dist;
	mobj_t *oldtracer;

	oldtracer = actor->tracer;

	// modify tracer threshold
	if (!actor->tracer || actor->tracer->health <= 0)
		actor->threshold = 0;
	else
		actor->threshold = 1;

	if (!actor->tracer || !(actor->tracer->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, true, 0))
			return; // got a new target

		actor->momx = actor->momy = actor->momz = 0;
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	if (multiplayer && !actor->threshold && P_LookForPlayers(actor, true, true, 0))
		return; // got a new target

	// Face movement direction if not doing so
	exact = R_PointToAngle2(actor->x, actor->y, actor->tracer->x, actor->tracer->y);
	actor->angle = exact;
	if (exact != actor->angle)
	{
		if (exact - actor->angle > ANGLE_180)
		{
			actor->angle -= actor->info->raisestate;
			if (exact - actor->angle < ANGLE_180)
				actor->angle = exact;
		}
		else
		{
			actor->angle += actor->info->raisestate;
			if (exact - actor->angle > ANGLE_180)
				actor->angle = exact;
		}
	}
	// movedir is up/down angle: how much it has to go up as it goes over to the player
	xydist = P_AproxDistance(actor->tracer->x - actor->x, actor->tracer->y - actor->y);
	exact = R_PointToAngle2(actor->x, actor->z, actor->x + xydist, actor->tracer->z);
	actor->movedir = exact;
	if (exact != actor->movedir)
	{
		if (exact - actor->movedir > ANGLE_180)
		{
			actor->movedir -= actor->info->raisestate;
			if (exact - actor->movedir < ANGLE_180)
				actor->movedir = exact;
		}
		else
		{
			actor->movedir += actor->info->raisestate;
			if (exact - actor->movedir > ANGLE_180)
				actor->movedir = exact;
		}
	}

	// check for melee attack
	if (actor->tracer)
	{
		if (P_AproxDistance(actor->tracer->x-actor->x, actor->tracer->y-actor->y) < actor->radius+actor->tracer->radius)
		{
			if (!((actor->tracer->z > actor->z + actor->height) || (actor->z > actor->tracer->z + actor->tracer->height)))
			{
				P_ExplodeMissile(actor);
				P_RadiusAttack(actor, actor, 96);
				return;
			}
		}
	}

	// chase towards player
	if ((dist = P_AproxDistance(xydist, actor->tracer->z-actor->z))
		> (actor->info->painchance << FRACBITS))
	{
		P_SetTarget(&actor->tracer, NULL); // Too far away
		return;
	}

	if (actor->reactiontime == 0)
	{
		actor->reactiontime = actor->info->reactiontime;
		return;
	}

	if (actor->reactiontime > 1)
	{
		actor->reactiontime--;
		return;
	}

	if (actor->reactiontime > 0)
	{
		actor->reactiontime = -42;

		if (actor->info->seesound)
			S_StartScreamSound(actor, actor->info->seesound);
	}

	if (actor->reactiontime == -42)
	{
		fixed_t xyspeed;

		actor->reactiontime = -42;

		exact = actor->movedir>>ANGLETOFINESHIFT;
		xyspeed = FixedMul(actor->tracer->player->normalspeed*3*(FRACUNIT/4), FINECOSINE(exact));
		actor->momz = FixedMul(actor->tracer->player->normalspeed*3*(FRACUNIT/4), FINESINE(exact));

		exact = actor->angle>>ANGLETOFINESHIFT;
		actor->momx = FixedMul(xyspeed, FINECOSINE(exact));
		actor->momy = FixedMul(xyspeed, FINESINE(exact));

		// Variable re-use
		xyspeed = (P_AproxDistance(actor->tracer->x - actor->x, P_AproxDistance(actor->tracer->y - actor->y, actor->tracer->z - actor->z))>>(FRACBITS+6));

		if (xyspeed < 1)
			xyspeed = 1;

		if (leveltime % xyspeed == 0)
			S_StartSound(actor, sfx_deton);
	}
}

// Function: A_CapeChase
//
// Description: Set an object's location to its target or tracer.
//
// var1:
//		0 = Use target
//		1 = Use tracer
//		upper 16 bits = Z offset
// var2:
//		upper 16 bits = forward/backward offset
//		lower 16 bits = sideways offset
//
void A_CapeChase(mobj_t *actor)
{
	mobj_t *chaser;
	fixed_t foffsetx, foffsety, boffsetx, boffsety;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (cv_debug)
		CONS_Printf("A_CapeChase called from object type %d, var1: %d, var2: %d\n", actor->type, locvar1, locvar2);

	if (locvar1 & 65535)
		chaser = actor->tracer;
	else
		chaser = actor->target;

	if (actor->state == &states[S_DISS])
		return;

	if (!chaser || (chaser->health <= 0))
	{
		if (chaser && cv_debug)
			CONS_Printf("Hmm, the guy I'm chasing (object type %d) has no health.. so I'll die too!\n", chaser->type);

		P_SetMobjState(actor, S_DISS);
		return;
	}

	foffsetx = P_ReturnThrustX(chaser, chaser->angle, (locvar2 >> 16)*FRACUNIT);
	foffsety = P_ReturnThrustY(chaser, chaser->angle, (locvar2 >> 16)*FRACUNIT);

	boffsetx = P_ReturnThrustX(chaser, chaser->angle-ANGLE_90, (locvar2 & 65535)*FRACUNIT);
	boffsety = P_ReturnThrustY(chaser, chaser->angle-ANGLE_90, (locvar2 & 65535)*FRACUNIT);

	P_UnsetThingPosition(actor);
	actor->x = chaser->x + foffsetx + boffsetx;
	actor->y = chaser->y + foffsety + boffsety;
	actor->z = chaser->z + ((locvar1 >> 16)*FRACUNIT);
	actor->angle = chaser->angle;
	P_SetThingPosition(actor);
}

// Function: A_RotateSpikeBall
//
// Description: Rotates a spike ball around its target.
//
// var1 = unused
// var2 = unused
//
void A_RotateSpikeBall(mobj_t *actor)
{
	const fixed_t radius = 12*actor->info->speed;

	if (actor->type == MT_SPECIALSPIKEBALL)
		return;

	if (!actor->target) // This should NEVER happen.
	{
		if (cv_debug)
			CONS_Printf("Error: Spikeball has no target\n");
		P_SetMobjState(actor, S_DISS);
		return;
	}

	if (!actor->info->speed)
	{
		CONS_Printf("Error: A_RotateSpikeBall: Object has no speed.\n");
		return;
	}

	actor->angle += FixedAngle(actor->info->speed);
	P_UnsetThingPosition(actor);
	{
		const angle_t fa = actor->angle>>ANGLETOFINESHIFT;
		actor->x = actor->target->x + FixedMul(FINECOSINE(fa),radius);
		actor->y = actor->target->y + FixedMul(FINESINE(fa),radius);
		actor->z = actor->target->z + actor->target->height/2;
		P_SetThingPosition(actor);
	}
}

// Function: A_RockSpawn
//
// Spawns rocks at a specified interval
//
// var1 = unused
// var2 = unused
void A_RockSpawn(mobj_t *actor)
{
	mobj_t *mo;
	mobjtype_t type;
	INT32 i = P_FindSpecialLineFromTag(12, (INT16)actor->threshold, -1);
	line_t *line;
	fixed_t dist;
	fixed_t randomoomph;

	if (i == -1)
	{
		CONS_Printf("A_RockSpawn: Unable to find parameter line 12 (tag %d)!\n", actor->threshold);
		return;
	}

	line = &lines[i];

	if (!(sides[line->sidenum[0]].textureoffset >> FRACBITS))
	{
		CONS_Printf("A_RockSpawn: No X-offset detected! (tag %d)!\n", actor->threshold);
		return;
	}

	dist = P_AproxDistance(line->dx, line->dy)/16;

	if (dist < 1)
		dist = 1;

	type = MT_ROCKCRUMBLE1 + (sides[line->sidenum[0]].rowoffset >> FRACBITS);

	if (line->flags & ML_NOCLIMB)
		randomoomph = P_Random() * (FRACUNIT/32);
	else
		randomoomph = 0;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_FALLINGROCK);
	P_SetMobjState(mo, mobjinfo[type].spawnstate);
	mo->angle = R_PointToAngle2(line->v2->x, line->v2->y, line->v1->x, line->v1->y);

	P_InstaThrust(mo, mo->angle, dist + randomoomph);
	mo->momz = dist + randomoomph;

	var1 = sides[line->sidenum[0]].textureoffset >> FRACBITS;
	A_SetTics(actor);
}

// Function: A_MaceRotate
//
// Whether a mace ball or a chain link of it, rotate around your axis target.
//
// var1 = unused
// var2 = unused
//
void A_MaceRotate(mobj_t *actor)
{
	TVector v;
	TVector *res;
	fixed_t radius = actor->info->speed*actor->reactiontime;

	if (!actor->target) // This should NEVER happen.
	{
		if (cv_debug)
			CONS_Printf("Mace object (type %d) has no target!\n", actor->type);
		P_SetMobjState(actor, S_DISS);
		return;
	}
/*
So NOBODY forgets:

actor->target->
threshold - X tilt
movecount - Z tilt
reactiontime - link # in the chain (1 is closest)
lastlook - speed
friction - top speed

movedir - current angle holder
*/

	P_UnsetThingPosition(actor);
	actor->x = actor->target->x;
	actor->y = actor->target->y;
	if (actor->type == MT_SMALLMACECHAIN || actor->type == MT_BIGMACECHAIN)
		actor->z = actor->target->z - actor->height/4;
	else
		actor->z = actor->target->z - actor->height/2;

	if (actor->target->lastlook > actor->target->friction)
		actor->target->lastlook = actor->target->friction;

	if (actor->target->type == MT_HANGMACEPOINT || actor->target->type == MT_SWINGMACEPOINT)
	{
		actor->movecount += actor->target->lastlook;
		actor->movecount &= FINEMASK;

		actor->threshold = FixedMul(FINECOSINE(actor->movecount), actor->target->lastlook);

		v[0] = FRACUNIT;
		v[1] = 0;
		v[2] = -radius;
		v[3] = FRACUNIT;

		res = VectorMatrixMultiply(v, *RotateXMatrix(FixedAngle(actor->threshold<<FRACBITS)));
		M_Memcpy(&v, res, sizeof (v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(actor->target->health<<ANGLETOFINESHIFT));
		M_Memcpy(&v, res, sizeof (v));
	}
	else
	{
		angle_t fa;
		actor->threshold += actor->target->lastlook;
		actor->threshold &= FINEMASK;

		fa = actor->threshold;
		v[0] = FixedMul(FINECOSINE(fa),radius);
		v[1] = 0;
		v[2] = FixedMul(FINESINE(fa),radius);
		v[3] = FRACUNIT;

		actor->target->health &= FINEMASK;

		res = VectorMatrixMultiply(v, *RotateXMatrix(actor->target->threshold<<ANGLETOFINESHIFT));
		M_Memcpy(&v, res, sizeof (v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(actor->target->health<<ANGLETOFINESHIFT));
		M_Memcpy(&v, res, sizeof (v));
	}

	actor->x += v[0];
	actor->y += v[1];
	actor->z += v[2];

	P_SetThingPosition(actor);

	if ((leveltime & 63) && (actor->type == MT_BIGMACE || actor->type == MT_SMALLMACE) && actor->target->type == MT_MACEPOINT)
		S_StartSound(actor, actor->info->activesound);
}

// Function: A_SnowBall
//
// Description: Moves an object like A_MoveAbsolute in its current direction, using its Speed variable for the speed. Also sets a timer for the object to disappear.
//
// var1 = duration before disappearing (in seconds).
// var2 = unused
//
void A_SnowBall(mobj_t *actor)
{
	INT32 locvar1 = var1;
	//INT32 locvar2 = var2;

	P_InstaThrust(actor, actor->angle, actor->info->speed);
	if (!actor->fuse)
		actor->fuse = locvar1*TICRATE;
}

// Function: A_CrawlaCommanderThink
//
// Description: Thinker for Crawla Commander.
//
// var1 = shoot bullets?
// var2 = "pogo mode" speed
//
void A_CrawlaCommanderThink(mobj_t *actor)
{
	fixed_t dist;
	sector_t *nextsector;
	fixed_t thefloor;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if (actor->fuse & 1)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

	if (actor->reactiontime > 0)
		actor->reactiontime--;

	if (actor->fuse < 2)
	{
		actor->fuse = 0;
		actor->flags2 &= ~MF2_FRET;
	}

	// Hover mode
	if (actor->health > 1 || actor->fuse)
	{
		if (actor->z < thefloor + (16*FRACUNIT))
			actor->momz += FRACUNIT;
		else if (actor->z < thefloor + (32*FRACUNIT))
			actor->momz += FRACUNIT/2;
		else
			actor->momz += 16;
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		if (actor->state != &states[actor->info->spawnstate])
			P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	dist = P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y);

	if (actor->target->player && actor->health > 1)
	{
		if (dist < 128*FRACUNIT
			&& ((actor->target->player->pflags & PF_JUMPED) || (actor->target->player->pflags & PF_SPINNING)))
		{
			// Auugh! He's trying to kill you! Strafe! STRAAAAFFEEE!!
			if (actor->target->momx || actor->target->momy)
			{
				P_InstaThrust(actor, actor->angle - ANGLE_180, 20*FRACUNIT);
			}
			return;
		}
	}

	if (locvar1)
	{
		if (actor->health < 2 && P_Random() < 2)
		{
			P_SpawnMissile (actor, actor->target, locvar1);
		}
	}

	// Face the player
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if (actor->threshold && dist > 256*FRACUNIT)
		actor->momx = actor->momy = 0;

	if (actor->reactiontime && actor->reactiontime <= 2*TICRATE && dist > actor->target->radius - FRACUNIT)
	{
		actor->threshold = 0;

		// Roam around, somewhat in the player's direction.
		actor->angle += (P_Random()<<10);
		actor->angle -= (P_Random()<<10);

		if (actor->health > 1)
			P_InstaThrust(actor, actor->angle, 10*FRACUNIT);
	}
	else if (!actor->reactiontime)
	{
		if (actor->health > 1) // Hover Mode
		{
			if (dist < 512*FRACUNIT)
			{
				actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
				P_InstaThrust(actor, actor->angle, 60*FRACUNIT);
				actor->threshold = 1;
			}
		}
		actor->reactiontime = 2*TICRATE + P_Random()/2;
	}

	if (actor->health == 1)
		P_Thrust(actor, actor->angle, 1);

	// Pogo Mode
	if (!actor->fuse && actor->health == 1 && actor->z <= actor->floorz)
	{
		if (dist < 256*FRACUNIT)
		{
			actor->momz = locvar2;
			actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
			P_InstaThrust(actor, actor->angle, locvar2/8);
			// pogo on player
		}
		else
		{
			UINT8 prandom = P_Random();
			actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y) + (P_Random() & 1 ? -prandom : +prandom);
			P_InstaThrust(actor, actor->angle, FixedDiv(locvar2, 3*FRACUNIT/2));
			actor->momz = locvar2; // Bounce up in air
		}
	}

	nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;

	// Move downwards or upwards to go through a passageway.
	if (nextsector->floorheight > actor->z && nextsector->floorheight - actor->z < 128*FRACUNIT)
		actor->momz += (nextsector->floorheight - actor->z) / 4;
}

// Function: A_RingExplode
//
// Description: An explosion ring exploding
//
// var1 = unused
// var2 = unused
//
void A_RingExplode(mobj_t *actor)
{
	mobj_t *mo2;
	thinker_t *th;
	INT32 d;

	for (d = 0; d < 16; d++)
		P_SpawnParaloop(actor->x, actor->y, actor->z + actor->height, actor->info->painchance, 16, MT_NIGHTSPARKLE, d*(ANGLE_22h), true, false);

	S_StartSound(actor, sfx_prloop);

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if (mo2 == actor) // Don't explode yourself! Endless loop!
			continue;

		if (P_AproxDistance(P_AproxDistance(mo2->x - actor->x, mo2->y - actor->y), mo2->z - actor->z) > actor->info->painchance)
			continue;

		if (mo2->flags & MF_SHOOTABLE)
		{
			actor->flags2 |= MF2_DEBRIS;
#if 0
			// Fun experimental explosion hop code.
			// TF2 in my SRB2? It's more likely than you think.
			if (mo2->player && (mo2->player == actor->target->player) && actor->type == MT_THROWNEXPLOSION)
			{
				player_t *jumpingplayer;
				angle_t jumpangle;
				fixed_t horizdist, vertdist;
				fixed_t horizmom, vertmom;

				jumpingplayer = actor->target->player;
				jumpangle = R_PointToAngle2(actor->x, actor->y, jumpingplayer->mo->x, jumpingplayer->mo->y);

				// Scale force based on distance from explosive.
				horizdist = P_AproxDistance(actor->x - jumpingplayer->mo->x, actor->y - jumpingplayer->mo->y);
				vertdist = abs(actor->z - jumpingplayer->mo->z);
				horizmom = FixedMul(20*FRACUNIT, FixedDiv(actor->info->painchance - horizdist, actor->info->painchance));
				vertmom = FixedMul(20*FRACUNIT, FixedDiv(actor->info->painchance - vertdist, actor->info->painchance));

				//Minimum force
				if (horizmom < 8*FRACUNIT)
					horizmom = 8*FRACUNIT;
				if (vertmom < 8*FRACUNIT)
					vertmom = 8*FRACUNIT;

				// Horizontal momentum.
				P_InstaThrust(jumpingplayer->mo, jumpangle, horizmom);

				// If off the ground, apply vertical momentum.
				if (!P_IsObjectOnGround(jumpingplayer->mo))
				{
					if (actor->z > jumpingplayer->mo->z)
						jumpingplayer->mo->momz -= vertmom;
					else
						jumpingplayer->mo->momz += vertmom;
				}
			}
			else
#endif
				P_DamageMobj(mo2, actor, actor->target, 1);

			continue;
		}
	}
	return;
}

// Function: A_OldRingExplode
//
// Description: An explosion ring exploding, 1.09.4 style
//
// var1 = object # to explode as debris
// var2 = unused
//
void A_OldRingExplode(mobj_t *actor) {
	UINT8 i;
	mobj_t *mo;
	const fixed_t ns = 20 * FRACUNIT;
	INT32 locvar1 = var1;
	//INT32 locvar2 = var2;
	boolean changecolor = (actor->target && actor->target->player);

	for (i = 0; i < 32; i++)
	{
		const angle_t fa = (i*FINEANGLES/16) & FINEMASK;

		mo = P_SpawnMobj(actor->x, actor->y, actor->z, locvar1);
		P_SetTarget(&mo->target, actor->target); // Transfer target so player gets the points

		mo->momx = FixedMul(FINESINE(fa),ns);
		mo->momy = FixedMul(FINECOSINE(fa),ns);

		if (i > 15)
		{
			if (i & 1)
				mo->momz = ns;
			else
				mo->momz = -ns;
		}

		mo->flags2 |= MF2_DEBRIS;
		mo->fuse = TICRATE/(OLDTICRATE/5);

		if (changecolor)
		{
			mo->flags |= MF_TRANSLATION;
			if (gametype != GT_CTF)
				mo->color = actor->target->color; //copy color
			else if (actor->target->player->ctfteam == 2)
				mo->color = 8;
		}
	}

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, locvar1);

	P_SetTarget(&mo->target, actor->target);
	mo->momz = ns;
	mo->flags2 |= MF2_DEBRIS;
	mo->fuse = TICRATE/(OLDTICRATE/5);

	if (changecolor)
	{
		mo->flags |= MF_TRANSLATION;
		if (gametype != GT_CTF)
			mo->color = actor->target->color; //copy color
		else if (actor->target->player->ctfteam == 2)
			mo->color = 8;
	}

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, locvar1);

	P_SetTarget(&mo->target, actor->target);
	mo->momz = -ns;
	mo->flags2 |= MF2_DEBRIS;
	mo->fuse = TICRATE/(OLDTICRATE/5);

	if (changecolor)
	{
		mo->flags |= MF_TRANSLATION;
		if (gametype != GT_CTF)
			mo->color = actor->target->color; //copy color
		else if (actor->target->player->ctfteam == 2)
			mo->color = 8;
	}
}

// Function: A_MixUp
//
// Description: Mix up all of the player positions.
//
// var1 = unused
// var2 = unused
//
void A_MixUp(mobj_t *actor)
{
	boolean teleported[MAXPLAYERS];
	INT32 i, numplayers = 0, prandom = 0;

	actor = NULL;
	if (!multiplayer)
		return;

	// No mix-up monitors in hide and seek! With so little time to hide,
	// random factors causing you to lose your hiding spot are unfun.
	if (gametype == GT_TAG && cv_tagtype.value)
		return;

	numplayers = 0;
	memset(teleported, 0, sizeof (teleported));

	// Count the number of players in the game
	// and grab their xyz coords
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
			&& !players[i].exiting && !players[i].powers[pw_super])
		{
			if ((netgame || multiplayer) && players[i].spectator) // Ignore spectators
				continue;

			numplayers++;
		}

	if (numplayers <= 1) // Not enough players to mix up.
		return;
	else if (numplayers == 2) // Special case -- simple swap
	{
		fixed_t x, y, z;
		angle_t angle;
		INT32 one = -1, two = 0; // default value 0 to make the compiler shut up

		// Zoom tube stuff
		mobj_t *tempthing = NULL; //tracer
		pflags_t flags1,flags2;   //player pflags
		INT32 transspeed;          //player speed

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
				&& !players[i].exiting && !players[i].powers[pw_super])
			{
				if ((netgame || multiplayer) && players[i].spectator) // Ignore spectators
					continue;

				if (one == -1)
					one = i;
				else
				{
					two = i;
					break;
				}
			}

		//get this done first!
		tempthing = players[one].mo->tracer;
		P_SetTarget(&players[one].mo->tracer, players[two].mo->tracer);
		P_SetTarget(&players[two].mo->tracer, tempthing);

		//zoom tubes use player->speed to determine direction and speed
		transspeed = players[one].speed;
		players[one].speed = players[two].speed;
		players[two].speed = transspeed;

		//set flags variables now but DON'T set them.
		flags1 = (players[one].pflags & (PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART));
		flags2 = (players[two].pflags & (PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART));

		x = players[one].mo->x;
		y = players[one].mo->y;
		z = players[one].mo->z;
		angle = players[one].mo->angle;

		P_MixUp(players[one].mo, players[two].mo->x, players[two].mo->y,
			players[two].mo->z, players[two].mo->angle);

		P_MixUp(players[two].mo, x, y, z, angle);

		//flags set after mixup.  Stupid P_ResetPlayer() takes away some of the flags we look for...
		//but not all of them!  So we need to make sure they aren't set wrong or anything.
		players[one].pflags &= ~(PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART);
		players[one].pflags |= flags2;
		players[two].pflags &= ~(PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART);
		players[two].pflags |= flags1;

		teleported[one] = true;
		teleported[two] = true;
	}
	else
	{
		fixed_t position[MAXPLAYERS][3];
		angle_t anglepos[MAXPLAYERS];
		INT32 pindex[MAXPLAYERS], counter = 0, teleportfrom = 0;

		// Zoom tube stuff
		mobj_t *transtracer[MAXPLAYERS]; //tracer
		pflags_t transflag[MAXPLAYERS];  //player pflags
		INT32 transspeed[MAXPLAYERS];     //player speed

		for (i = 0; i < MAXPLAYERS; i++)
		{
			position[i][0] = position[i][1] = position[i][2] = anglepos[i] = pindex[i] = -1;
			teleported[i] = false;
		}

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].playerstate == PST_LIVE
				&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super])
			{
				if ((netgame || multiplayer) && players[i].spectator)// Ignore spectators
					continue;

				position[counter][0] = players[i].mo->x;
				position[counter][1] = players[i].mo->y;
				position[counter][2] = players[i].mo->z;
				pindex[counter] = i;
				anglepos[counter] = players[i].mo->angle;
				players[i].mo->momx = players[i].mo->momy = players[i].mo->momz =
					players[i].rmomx = players[i].rmomy = 1;
				players[i].cmomx = players[i].cmomy = 0;

				transflag[counter] = (players[i].pflags & (PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART));
				transspeed[counter] = players[i].speed;
				transtracer[counter] = players[i].mo->tracer;

				counter++;
			}
		}

		counter = 0;

		// Mix them up!
		for (;;)
		{
			if (counter > 255) // fail-safe to avoid endless loop
				break;
			prandom = P_Random();
			prandom %= numplayers; // I love modular arithmetic, don't you?
			if (prandom) // Make sure it's not a useless mix
				break;
			counter++;
		}

		counter = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].playerstate == PST_LIVE
				&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super])
			{
				if ((netgame || multiplayer) && players[i].spectator)// Ignore spectators
					continue;

				teleportfrom = (counter + prandom) % numplayers;

				//speed and tracer come before...
				players[i].speed = transspeed[teleportfrom];
				P_SetTarget(&players[i].mo->tracer, transtracer[teleportfrom]);

				P_MixUp(players[i].mo,
					position[teleportfrom][0],
					position[teleportfrom][1],
					position[teleportfrom][2],
					anglepos[teleportfrom]);

				//...flags after.  same reasoning.
				players[i].pflags &= ~(PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_MINECART);
				players[i].pflags |= transflag[teleportfrom];

				teleported[i] = true;
				counter++;
			}
		}
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (teleported[i])
		{
			if (playeringame[i] && players[i].playerstate == PST_LIVE
				&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super])
			{
				if ((netgame || multiplayer) && players[i].spectator)// Ignore spectators
					continue;

				P_SetThingPosition(players[i].mo);

				players[i].mo->floorz = players[i].mo->subsector->sector->floorheight;
				players[i].mo->ceilingz = players[i].mo->subsector->sector->ceilingheight;

				P_CheckPosition(players[i].mo, players[i].mo->x, players[i].mo->y);
			}
		}
	}

	// Play the 'bowrwoosh!' sound
	S_StartSound(NULL, sfx_mixup);
}

// Function: A_RecyclePowers
//
// Description: Take all player's powers, and swap 'em.
//
// var1 = unused
// var2 = unused
//
void A_RecyclePowers(mobj_t *actor)
{
	INT32 i, numplayers = 0;

	actor = NULL;
	if (!multiplayer)
		return;

	numplayers = 0;

	// Count the number of players in the game
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
			&& !players[i].exiting && !players[i].powers[pw_super] && !((netgame || multiplayer) && players[i].spectator))
			numplayers++;

	if (numplayers <= 1)
		return; //nobody to touch!

	else if (numplayers == 2) //simple swap is all that's needed
	{
		INT32 temp[NUMPOWERS];
		INT32 weapons;
		INT32 weaponheld;

		INT32 one = -1, two = 0; // default value 0 to make the compiler shut up

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
				&& !players[i].exiting && !players[i].powers[pw_super] && !((netgame || multiplayer) && players[i].spectator))
			{
				if (one == -1)
					one = i;
				else
					two = i;
			}
		for (i = 0; i < NUMPOWERS; i++)
		{
			if (i == pw_flashing || i == pw_underwater || i == pw_spacetime
			    || i == pw_tailsfly || i == pw_extralife || i == pw_super || i == pw_nocontrol)
				continue;
			temp[i] = players[one].powers[i];
			players[one].powers[i] = players[two].powers[i];
			players[two].powers[i] = temp[i];
		}
		//1.1: weapons need to be swapped too
		weapons = players[one].ringweapons;
		players[one].ringweapons = players[two].ringweapons;
		players[two].ringweapons = weapons;

		weaponheld = players[one].currentweapon;
		players[one].currentweapon = players[two].currentweapon;
		players[two].currentweapon = weaponheld;

		P_SpawnShieldOrb(players[one].mo->player);
		P_SpawnShieldOrb(players[two].mo->player);
		players[one].bonuscount = players[two].bonuscount = 10;
		//piece o' cake, eh?
	}
	else
	{
		//well, the cake is a LIE!
		INT32 temp[MAXPLAYERS][NUMPOWERS];
		INT32 weapons[MAXPLAYERS];
		INT32 weaponheld[MAXPLAYERS];
		INT32 pindex[MAXPLAYERS], counter = 0, j = 0, prandom = 0, recyclefrom = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].playerstate == PST_LIVE
				&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super]
				&& !((netgame || multiplayer) && players[i].spectator))
			{
				pindex[counter] = i;
				for (j = 0; j < NUMPOWERS; j++)
					temp[counter][j] = players[i].powers[j];
				//1.1: ring weapons too
				weapons[counter] = players[i].ringweapons;
				weaponheld[counter] = players[i].currentweapon;
				counter++;
			}
		}
		counter = 0;

		// Mix them up!
		for (;;)
		{
			if (counter > 255) // fail-safe to avoid endless loop
				break;
			prandom = P_Random();
			prandom %= numplayers; // I love modular arithmetic, don't you?
			if (prandom) // Make sure it's not a useless mix
				break;
			counter++;
		}

		counter = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].playerstate == PST_LIVE
				&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super]
				&& !((netgame || multiplayer) && players[i].spectator))
			{
				recyclefrom = (counter + prandom) % numplayers;
				for (j = 0; j < NUMPOWERS; j++)
				{
					if (j == pw_flashing || j == pw_underwater || j == pw_spacetime
					    || j == pw_tailsfly || j == pw_extralife || j == pw_super || j == pw_nocontrol)
						continue;
					players[i].powers[j] = temp[recyclefrom][j];
				}
				//1.1: weapon rings too
				players[i].ringweapons = weapons[recyclefrom];
				players[i].currentweapon = weaponheld[recyclefrom];

				P_SpawnShieldOrb(players[i].mo->player);
				players[i].bonuscount = 10;
				counter++;
			}
		}
	}
	for (i = 0; i < MAXPLAYERS; i++) //just for sneakers/invinc.
		if (playeringame[i] && players[i].playerstate == PST_LIVE
			&& players[i].mo && players[i].mo->health > 0 && !players[i].exiting && !players[i].powers[pw_super]
			&& !((netgame || multiplayer) && players[i].spectator))
			if (P_IsLocalPlayer(players[i].mo->player))
				P_RestoreMusic(players[i].mo->player);

	S_StartSound(NULL, sfx_gravch); //heh, the sound effect I used is already in
}

// Function: A_Boss1Chase
//
// Description: Like A_Chase, but for Boss 1.
//
// var1 = unused
// var2 = unused
//
void A_Boss1Chase(mobj_t *actor)
{
	INT32 delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	if (actor->z < actor->floorz+33*FRACUNIT)
		actor->z = actor->floorz+33*FRACUNIT;

	// turn towards movement direction if not there yet
	if (actor->movedir < NUMDIRS)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANGLE_45;
		else if (delta < 0)
			actor->angle += ANGLE_45;
	}

	// do not attack twice in a row
	if (actor->flags2 & MF2_JUSTATTACKED)
	{
		actor->flags2 &= ~MF2_JUSTATTACKED;
		P_NewChaseDir(actor);
		return;
	}

	if (actor->movecount)
		goto nomissile;

	if (!P_CheckMissileRange(actor))
		goto nomissile;

	if (actor->reactiontime <= 0)
	{
		if (actor->health > actor->info->damage)
		{
			if (P_Random() & 1)
				P_SetMobjState(actor, actor->info->missilestate);
			else
				P_SetMobjState(actor, actor->info->meleestate);
		}
		else
			P_SetMobjState(actor, actor->info->raisestate);

		actor->flags2 |= MF2_JUSTATTACKED;
		actor->reactiontime = 2*TICRATE;
		return;
	}

	// ?
nomissile:
	// possibly choose another target
	if (multiplayer && P_Random() < 2)
	{
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target
	}

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor, actor->info->speed))
		P_NewChaseDir(actor);
}

// Function: A_Boss2Chase
//
// Description: Really doesn't 'chase', but rather goes in a circle.
//
// var1 = unused
// var2 = unused
//
void A_Boss2Chase(mobj_t *actor)
{
	fixed_t radius;
	boolean reverse = false;
	INT32 speedvar;

	if (actor->health <= 0)
		return;

	// When reactiontime hits zero, he will go the other way
	if (actor->reactiontime)
		actor->reactiontime--;

	if (actor->reactiontime <= 0)
	{
		reverse = true;
		actor->reactiontime = 2*TICRATE + P_Random();
	}

	P_SetTarget(&actor->target, P_GetClosestAxis(actor));

	if (!actor->target) // This should NEVER happen.
	{
		CONS_Printf("Error: Boss2 has no target!\n");
		A_BossDeath(actor);
		return;
	}

	radius = actor->target->radius;

	if (reverse)
		actor->watertop = -actor->watertop;

	// Only speed up if you have the 'Deaf' flag.
	if (actor->flags & MF_AMBUSH)
		speedvar = actor->health;
	else
		speedvar = actor->info->spawnhealth;

	actor->target->angle += FixedAngle(FixedDiv(FixedMul(actor->watertop, (actor->info->spawnhealth*(FRACUNIT/4)*3)), speedvar*FRACUNIT)); // Don't use FixedAngleC!

	P_UnsetThingPosition(actor);
	{
		const angle_t fa = actor->target->angle>>ANGLETOFINESHIFT;
		const fixed_t fc = FixedMul(FINECOSINE(fa),radius);
		const fixed_t fs = FixedMul(FINESINE(fa),radius);
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x + fc, actor->target->y + fs);
		actor->x = actor->target->x + fc;
		actor->y = actor->target->y + fs;
	}
	P_SetThingPosition(actor);

	// Spray goo once every second
	if (leveltime % (speedvar*15/10)-1 == 0)
	{
		const fixed_t ns = 3 * FRACUNIT;
		mobj_t *goop;
		fixed_t fz = actor->z+actor->height+56*FRACUNIT;
		angle_t fa;
		// actor->movedir is used to determine the last
		// direction goo was sprayed in. There are 8 possible
		// directions to spray. (45-degree increments)

		actor->movedir++;
		actor->movedir %= NUMDIRS;
		fa = (actor->movedir*FINEANGLES/8) & FINEMASK;

		goop = P_SpawnMobj(actor->x, actor->y, fz, actor->info->painchance);
		goop->momx = FixedMul(FINESINE(fa),ns);
		goop->momy = FixedMul(FINECOSINE(fa),ns);
		goop->momz = 4*FRACUNIT;
		goop->fuse = 30*TICRATE+P_Random();

		if (actor->info->attacksound)
			S_StartAttackSound(actor, actor->info->attacksound);

		if (P_Random() & 1)
		{
			goop->momx *= 2;
			goop->momy *= 2;
		}
		else if (P_Random() > 128)
		{
			goop->momx *= 3;
			goop->momy *= 3;
		}

		actor->flags2 |= MF2_JUSTATTACKED;
	}
}

// Function: A_Boss2Pogo
//
// Description: Pogo part of Boss 2 AI.
//
// var1 = unused
// var2 = unused
//
void A_Boss2Pogo(mobj_t *actor)
{
	if (actor->z <= actor->floorz + 8*FRACUNIT && actor->momz <= 0)
	{
		P_SetMobjState(actor, actor->info->raisestate);
		// Pogo Mode
	}
	else if (actor->momz < 0 && actor->reactiontime)
	{
		const fixed_t ns = 3 * FRACUNIT;
		mobj_t *goop;
		fixed_t fz = actor->z+actor->height+56*FRACUNIT;
		angle_t fa;
		INT32 i;
		// spray in all 8 directions!
		for (i = 0; i < 8; i++)
		{
			actor->movedir++;
			actor->movedir %= NUMDIRS;
			fa = (actor->movedir*FINEANGLES/8) & FINEMASK;

			goop = P_SpawnMobj(actor->x, actor->y, fz, actor->info->painchance);
			goop->momx = FixedMul(FINESINE(fa),ns);
			goop->momy = FixedMul(FINECOSINE(fa),ns);
			goop->momz = 4*FRACUNIT;


#ifdef CHAOSISNOTDEADYET
			if (gametype == GT_CHAOS)
				goop->fuse = 15*TICRATE;
			else
#endif
				goop->fuse = 30*TICRATE+P_Random();
		}
		actor->reactiontime = 0;
		if (actor->info->attacksound)
			S_StartAttackSound(actor, actor->info->attacksound);
		actor->flags2 |= MF2_JUSTATTACKED;
	}
}

// Function: A_Invinciblerize
//
// Description: Special function for Boss 2 so you can't just sit and destroy him.
//
// var1 = unused
// var2 = unused
//
void A_Invinciblerize(mobj_t *actor)
{
	A_Pain(actor);
	actor->reactiontime = 1;
	actor->movecount = TICRATE;
}

// Function: A_DeInvinciblerize
//
// Description: Does the opposite of A_Invinciblerize.
//
// var1 = unused
// var2 = unused
//
void A_DeInvinciblerize(mobj_t *actor)
{
	actor->movecount = actor->state->tics+TICRATE;
}

// Function: A_GoopSplat
//
// Description: Black Eggman goop hits a target and sticks around for awhile.
//
// var1 = unused
// var2 = unused
//
void A_GoopSplat(mobj_t *actor)
{
	P_UnsetThingPosition(actor);
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}
	actor->flags = MF_SPECIAL; // Not a typo
	P_SetThingPosition(actor);
}

// Function: A_Boss2PogoSFX
//
// Description: Pogoing for Boss 2
//
// var1 = pogo jump strength
// var2 = horizontal pogoing speed multiple
//
void A_Boss2PogoSFX(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		return;
	}

	// Boing!
	if (P_AproxDistance(actor->x-actor->target->x, actor->y-actor->target->y) < 256*FRACUNIT)
	{
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
		P_InstaThrust(actor, actor->angle, actor->info->speed);
		// pogo on player
	}
	else
	{
		UINT8 prandom = P_Random();
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y) + (P_Random() & 1 ? -prandom : +prandom);
		P_InstaThrust(actor, actor->angle, FixedMul(actor->info->speed,(locvar2)));
	}
	if (actor->info->activesound) S_StartSound(actor, actor->info->activesound);
	actor->momz = locvar1; // Bounce up in air
	actor->reactiontime = 1;
}

// Function: A_EggmanBox
//
// Description: Harms the player
//
// var1 = unused
// var2 = unused
//
void A_EggmanBox(mobj_t *actor)
{
	if (!actor->target || !actor->target->player)
	{
		if (cv_debug)
			CONS_Printf("%s",text[POWERUPNOTARGET]);
		return;
	}

	P_DamageMobj(actor->target, actor, actor, 1); // Ow!
}

// Function: A_TurretFire
//
// Description: Initiates turret fire.
//
// var1 = object # to repeatedly fire
// var2 = distance threshold
//
void A_TurretFire(mobj_t *actor)
{
	INT32 count = 0;
	fixed_t dist;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (locvar2)
		dist = locvar2*FRACUNIT;
	else
		dist = 2048*FRACUNIT;

	if (!locvar1)
		locvar1 = MT_TURRETLASER;

	while (P_SupermanLook4Players(actor) && count < MAXPLAYERS)
	{
		if (P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y) < dist)
		{
			actor->flags2 |= MF2_FIRING;
			actor->eflags &= 65535;
			actor->eflags += (locvar1 << 16); // Upper 16 bits contains mobj #
			break;
		}

		count++;
	}
}

// Function: A_SuperTurretFire
//
// Description: Initiates turret fire that even stops Super Sonic.
//
// var1 = object # to repeatedly fire
// var2 = distance threshold
//
void A_SuperTurretFire(mobj_t *actor)
{
	INT32 count = 0;
	fixed_t dist;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (locvar2)
		dist = locvar2*FRACUNIT;
	else
		dist = 2048*FRACUNIT;

	if (!locvar1)
		locvar1 = MT_TURRETLASER;

	while (P_SupermanLook4Players(actor) && count < MAXPLAYERS)
	{
		if (P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y) < dist)
		{
			actor->flags2 |= MF2_FIRING;
			actor->flags2 |= MF2_SUPERFIRE;
			actor->eflags &= 65535;
			actor->eflags += (locvar1 << 16); // Upper 16 bits contains mobj #
			break;
		}

		count++;
	}
}

// Function: A_TurretStop
//
// Description: Stops the turret fire.
//
// var1 = Don't play activesound?
// var2 = unused
//
void A_TurretStop(mobj_t *actor)
{
	INT32 locvar1 = var1;

	actor->flags2 &= ~MF2_FIRING;
	actor->flags2 &= ~MF2_SUPERFIRE;

	if (actor->target && actor->info->activesound && !locvar1)
		S_StartSound(actor, actor->info->activesound);
}

// Function: A_SparkFollow
//
// Description: Used by the hyper sparks to rotate around their target.
//
// var1 = unused
// var2 = unused
//
void A_SparkFollow(mobj_t *actor)
{
	if (actor->state == &states[S_DISS])
		return;

	if ((!actor->target || (actor->target->health <= 0))
		|| (actor->target->player && !actor->target->player->powers[pw_super]))
	{
		P_SetMobjState(actor, S_DISS);
		return;
	}

	actor->angle += FixedAngle(actor->info->damage*FRACUNIT);
	P_UnsetThingPosition(actor);
	{
		const angle_t fa = actor->angle>>ANGLETOFINESHIFT;
		actor->x = actor->target->x + FixedMul(FINECOSINE(fa),actor->info->speed);
		actor->y = actor->target->y + FixedMul(FINESINE(fa),actor->info->speed);
		actor->z = actor->target->z + FixedDiv(actor->target->height,3*FRACUNIT) - actor->height;
	}
	P_SetThingPosition(actor);
}

// Function: A_BuzzFly
//
// Description: Makes an object slowly fly after a player, in the manner of a Buzz.
//
// var1 = unused
// var2 = unused
//
void A_BuzzFly(mobj_t *actor)
{
	if (actor->flags & MF_AMBUSH)
		return;

	if (actor->reactiontime)
		actor->reactiontime--;

	// modify target threshold
	if (actor->threshold)
	{
		if (!actor->target || actor->target->health <= 0)
			actor->threshold = 0;
		else
			actor->threshold--;
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		actor->momz = actor->momy = actor->momx = 0;
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	// turn towards movement direction if not there yet
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if (actor->target->health <= 0 || (!actor->threshold && !P_CheckSight(actor, actor->target)))
	{
		if ((multiplayer || netgame) && P_LookForPlayers(actor, true, false, 3072*FRACUNIT))
			return; // got a new target

		actor->momx = actor->momy = actor->momz = 0;
		P_SetMobjState(actor, actor->info->spawnstate); // Go back to looking around
		return;
	}

	// If the player is over 3072 fracunits away, then look for another player
	if (P_AproxDistance(P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y),
		actor->target->z - actor->z) > 3072*FRACUNIT)
	{
		if (multiplayer || netgame)
			P_LookForPlayers(actor, true, false, 3072*FRACUNIT); // maybe get a new target

		return;
	}

	// chase towards player
	{
		INT32 dist, realspeed;
		const fixed_t mf = 5*(FRACUNIT/4);

		if (ultimatemode)
			realspeed = FixedMul(actor->info->speed,mf);
		else
			realspeed = actor->info->speed;

		dist = P_AproxDistance(P_AproxDistance(actor->target->x - actor->x,
			actor->target->y - actor->y), actor->target->z - actor->z);

		if (dist < 1)
			dist = 1;

		actor->momx = FixedMul(FixedDiv(actor->target->x - actor->x, dist), realspeed);
		actor->momy = FixedMul(FixedDiv(actor->target->y - actor->y, dist), realspeed);
		actor->momz = FixedMul(FixedDiv(actor->target->z - actor->z, dist), realspeed);

		if (actor->z+actor->momz >= actor->waterbottom && actor->watertop > actor->floorz
			&& actor->z+actor->momz > actor->watertop - 256*FRACUNIT)
		{
			actor->momz = 0;
			actor->z = actor->watertop;
		}
	}
}

// Function: A_GuardChase
//
// Description: Modified A_Chase for Egg Guard
//
// var1 = unused
// var2 = unused
//
void A_GuardChase(mobj_t *actor)
{
	INT32 delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	if (!actor->tracer && actor->threshold != 42)
	{
		actor->threshold = 42;
		P_SetMobjState(actor, actor->info->painstate);
		actor->flags |= MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE;
		return;
	}

	// turn towards movement direction if not there yet
	if (actor->movedir < NUMDIRS)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANGLE_45;
		else if (delta < 0)
			actor->angle += ANGLE_45;
	}

	if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers(actor, true, false, 0))
			return; // got a new target

		P_SetMobjStateNF(actor, actor->info->spawnstate);
		return;
	}

	// possibly choose another target
	if (multiplayer && (actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
		&& P_LookForPlayers(actor, true, false, 0))
		return; // got a new target

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor, (actor->flags & MF_AMBUSH) ? actor->info->speed * 2 : actor->info->speed))
	{
		P_NewChaseDir(actor);
		actor->movecount += 5; // Increase tics before change in direction allowed.
	}

	if (actor->tracer && actor->tracer->type == MT_EGGSHIELD)
	{
		INT32 i;
		player_t *player;
		fixed_t blockdist;
		fixed_t newx, newy;
		fixed_t movex, movey;
		angle_t angle;

		newx = actor->x + P_ReturnThrustX(actor, actor->angle, FRACUNIT);
		newy = actor->y + P_ReturnThrustY(actor, actor->angle, FRACUNIT);

		movex = newx - actor->tracer->x;
		movey = newy - actor->tracer->y;

		P_UnsetThingPosition(actor->tracer);
		actor->tracer->x = newx;
		actor->tracer->y = newy;
		actor->tracer->z = actor->z;
		actor->tracer->angle = actor->angle;
		actor->tracer->floorz = actor->floorz;
		actor->tracer->ceilingz = actor->ceilingz;
		P_SetThingPosition(actor->tracer);

		// Search for players to push
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			player = &players[i];

			if (!player->mo)
				continue;

			if (player->mo->z > actor->tracer->z + actor->tracer->height)
				continue;

			if (player->mo->z + player->mo->height < actor->tracer->z)
				continue;

			blockdist = actor->tracer->radius + player->mo->radius;

			if (abs(actor->tracer->x - player->mo->x) >= blockdist || abs(actor->tracer->y - player->mo->y) >= blockdist)
				continue; // didn't hit it

			angle = R_PointToAngle2(actor->tracer->x, actor->tracer->y, player->mo->x, player->mo->y) - actor->tracer->angle;

			if (angle > ANGLE_90 && angle < ANGLE_270)
				continue;

			// Blocked by the shield
			player->mo->momx = movex;
			player->mo->momy = movey;
			return;
		}
	}
}

// Function: A_SetReactionTime
//
// Description: Sets the object's reaction time.
//
// var1 = 1 (use value in var2); 0 (use info table value)
// var2 = if var1 = 1, then value to set
//
void A_SetReactionTime(mobj_t *actor)
{
	if (var1)
		actor->reactiontime = var2;
	else
		actor->reactiontime = actor->info->reactiontime;
}

// Function: A_Boss3TakeDamage
//
// Description: Called when Boss 3 takes damage.
//
// var1 = movecount value
// var2 = unused
//
void A_Boss3TakeDamage(mobj_t *actor)
{
	actor->movecount = var1;

	if (actor->target && actor->target->spawnpoint)
		actor->threshold = actor->target->spawnpoint->extrainfo;
}

// Function: A_LinedefExecute
//
// Description: Object's location is used to set the calling sector. The tag used is var1, or, if that is zero, the mobj's state number (beginning from 0) + 1000.
//
// var1 = tag (optional)
// var2 = unused
//
void A_LinedefExecute(mobj_t *actor)
{
	INT32 tagnum;
	INT32 locvar1 = var1;

	if (locvar1 > 0)
		tagnum = locvar1;
	else
		tagnum = (INT32)(1000 + (size_t)(actor->state - states));

	if (cv_debug)
		CONS_Printf("A_LinedefExecute: Running mobjtype %d's sector with tag %d\n", actor->type, tagnum);

	P_LinedefExecute(tagnum, actor, actor->subsector->sector);
}

// Function: A_PlaySeeSound
//
// Description: Plays the object's seesound.
//
// var1 = unused
// var2 = unused
//
void A_PlaySeeSound(mobj_t *actor)
{
	if (actor->info->seesound)
		S_StartScreamSound(actor, actor->info->seesound);
}

// Function: A_PlayAttackSound
//
// Description: Plays the object's attacksound.
//
// var1 = unused
// var2 = unused
//
void A_PlayAttackSound(mobj_t *actor)
{
	if (actor->info->attacksound)
		S_StartAttackSound(actor, actor->info->attacksound);
}

// Function: A_PlayActiveSound
//
// Description: Plays the object's activesound.
//
// var1 = unused
// var2 = unused
//
void A_PlayActiveSound(mobj_t *actor)
{
	if (actor->info->activesound)
		S_StartSound(actor, actor->info->activesound);
}

// Function: A_SmokeTrailer
//
// Description: Adds smoke trails to an object.
//
// var1 = object # to spawn as smoke
// var2 = unused
//
void A_SmokeTrailer(mobj_t *actor)
{
	mobj_t *th;
	INT32 locvar1 = var1;

	if (gametic % (4*NEWTICRATERATIO))
		return;

	// add the smoke behind the rocket
	th = P_SpawnMobj(actor->x-actor->momx, actor->y-actor->momy, actor->z, locvar1);

	th->momz = FRACUNIT;
	th->tics -= P_Random() & 3;
	if (th->tics < 1)
		th->tics = 1;
}

// Function: A_SpawnObjectAbsolute
//
// Description: Spawns an object at an absolute position
//
// var1:
//		var1 >> 16 = x
//		var1 & 65535 = y
// var2:
//		var2 >> 16 = z
//		var2 & 65535 = type
//
void A_SpawnObjectAbsolute(mobj_t *actor)
{
	INT16 x, y, z; // Want to be sure we can use negative values
	mobjtype_t type;
	mobj_t *mo;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	x = (INT16)(locvar1>>16);
	y = (INT16)(locvar1&65535);
	z = (INT16)(locvar2>>16);
	type = (mobjtype_t)(locvar2&65535);

	mo = P_SpawnMobj(x<<FRACBITS, y<<FRACBITS, z<<FRACBITS, type);

	if (actor->eflags & MFE_VERTICALFLIP)
		mo->flags2 |= MF2_OBJECTFLIP;
}

// Function: A_SpawnObjectRelative
//
// Description: Spawns an object relative to the location of the actor
//
// var1:
//		var1 >> 16 = x
//		var1 & 65535 = y
// var2:
//		var2 >> 16 = z
//		var2 & 65535 = type
//
void A_SpawnObjectRelative(mobj_t *actor)
{
	INT16 x, y, z; // Want to be sure we can use negative values
	mobjtype_t type;
	mobj_t *mo;
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (cv_debug)
		CONS_Printf("A_SpawnObjectRelative called from object type %d, var1: %d, var2: %d\n", actor->type, locvar1, locvar2);

	x = (INT16)(locvar1>>16);
	y = (INT16)(locvar1&65535);
	z = (INT16)(locvar2>>16);
	type = (mobjtype_t)(locvar2&65535);

	mo = P_SpawnMobj(actor->x + (x<<FRACBITS), actor->y + (y<<FRACBITS), actor->z + (z<<FRACBITS), type);

	if (actor->eflags & MFE_VERTICALFLIP)
		mo->flags2 |= MF2_OBJECTFLIP;
}

// Function: A_ChangeAngleRelative
//
// Description: Changes the angle to a random relative value between the min and max. Set min and max to the same value to eliminate randomness
//
// var1 = min
// var2 = max
//
void A_ChangeAngleRelative(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	angle_t angle = (P_Random()+1)<<24;
	const angle_t amin = FixedAngle(locvar1*FRACUNIT);
	const angle_t amax = FixedAngle(locvar2*FRACUNIT);

#ifdef PARANOIA
	if (amin > amax)
		I_Error("A_ChangeAngleRelative: var1 is greater then var2");
#endif

	if (angle < amin)
		angle = amin;
	if (angle > amax)
		angle = amax;

	actor->angle += angle;
}

// Function: A_ChangeAngleAbsolute
//
// Description: Changes the angle to a random absolute value between the min and max. Set min and max to the same value to eliminate randomness
//
// var1 = min
// var2 = max
//
void A_ChangeAngleAbsolute(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	angle_t angle = (P_Random()+1)<<24;
	const angle_t amin = FixedAngle(locvar1*FRACUNIT);
	const angle_t amax = FixedAngle(locvar2*FRACUNIT);

#ifdef PARANOIA
	if (amin > amax)
		I_Error("A_ChangeAngleAbsolute: var1 is greater then var2");
#endif

	if (angle < amin)
		angle = amin;
	if (angle > amax)
		angle = amax;

	actor->angle = angle;
}

// Function: A_PlaySound
//
// Description: Plays a sound
//
// var1 = sound # to play
// var2:
//		0 = Play sound without an origin
//		1 = Play sound using calling object as origin
//
void A_PlaySound(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	S_StartSound(locvar2 ? actor : NULL, locvar1);
}

// Function: A_FindTarget
//
// Description: Finds the nearest/furthest mobj of the specified type and sets actor->target to it.
//
// var1 = mobj type
// var2 = if (0) nearest; else furthest;
//
void A_FindTarget(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	mobj_t *targetedmobj = NULL;
	thinker_t *th;
	mobj_t *mo2;
	fixed_t dist1 = 0, dist2 = 0;

	if (cv_debug)
		CONS_Printf("A_FindTarget called from object type %d, var1: %d, var2: %d\n", actor->type, locvar1, locvar2);

	// scan the thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if (mo2->type == (mobjtype_t)locvar1)
		{
			if (targetedmobj == NULL)
			{
				targetedmobj = mo2;
				dist2 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);
			}
			else
			{
				dist1 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);

				if ((!locvar2 && dist1 < dist2) || (locvar2 && dist1 > dist2))
				{
					targetedmobj = mo2;
					dist2 = dist1;
				}
			}
		}
	}

	if (!targetedmobj)
	{
		if (cv_debug)
			CONS_Printf("A_FindTarget: Unable to find the specified object to target.\n");
		return; // Oops, nothing found..
	}

	if (cv_debug)
		CONS_Printf("A_FindTarget: Found a target.\n");

	P_SetTarget(&actor->target, targetedmobj);
}

// Function: A_FindTracer
//
// Description: Finds the nearest/furthest mobj of the specified type and sets actor->tracer to it.
//
// var1 = mobj type
// var2 = if (0) nearest; else furthest;
//
void A_FindTracer(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	mobj_t *targetedmobj = NULL;
	thinker_t *th;
	mobj_t *mo2;
	fixed_t dist1 = 0, dist2 = 0;

	if (cv_debug)
		CONS_Printf("A_FindTracer called from object type %d, var1: %d, var2: %d\n", actor->type, locvar1, locvar2);

	// scan the thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if (mo2->type == (mobjtype_t)locvar1)
		{
			if (targetedmobj == NULL)
			{
				targetedmobj = mo2;
				dist2 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);
			}
			else
			{
				dist1 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);

				if ((!locvar2 && dist1 < dist2) || (locvar2 && dist1 > dist2))
				{
					targetedmobj = mo2;
					dist2 = dist1;
				}
			}
		}
	}

	if (!targetedmobj)
	{
		if (cv_debug)
			CONS_Printf("A_FindTracer: Unable to find the specified object to target.\n");
		return; // Oops, nothing found..
	}

	if (cv_debug)
		CONS_Printf("A_FindTracer: Found a target.\n");

	P_SetTarget(&actor->tracer, targetedmobj);
}

// Function: A_SetTics
//
// Description: Sets the animation tics of an object
//
// var1 = tics to set to
// var2 = if this is set, and no var1 is supplied, the mobj's threshold value will be used.
//
void A_SetTics(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (locvar1)
		actor->tics = locvar1;
	else if (locvar2)
		actor->tics = actor->threshold;
}

// Function: A_SetRandomTics
//
// Description: Sets the animation tics of an object to a random value Note: upper bound should not be higher than lower + 256
//
// var1 = lower bound
// var2 = upper bound
//
void A_SetRandomTics(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	actor->tics = locvar1 + (P_Random() % locvar2);
}

// Function: A_ChangeColorRelative
//
// Description: Changes the color of an object
//
// var1 = if (var1 > 0), find target and add its color value to yours
// var2 = if (var1 = 0), color value to add
//
void A_ChangeColorRelative(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	actor->flags |= MF_TRANSLATION;
	if (locvar1)
	{
		// Have you ever seen anything so hideous?
		if (actor->target)
			actor->color = (UINT8)(actor->color + actor->target->color);
	}
	else
		actor->color = (UINT8)(actor->color + locvar2);
}

// Function: A_ChangeColorAbsolute
//
// Description: Changes the color of an object by an absolute value.
//
// var1 = if (var1 > 0), set your color to your target's color
// var2 = if (var1 = 0), color value to set to
//
void A_ChangeColorAbsolute(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	actor->flags |= MF_TRANSLATION;
	if (locvar1)
	{
		if (actor->target)
			actor->color = actor->target->color;
	}
	else
		actor->color = (UINT8)locvar2;
}

// Function: A_MoveRelative
//
// Description: Moves an object (wrapper for P_Thrust)
//
// var1 = angle
// var2 = force
//
void A_MoveRelative(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	P_Thrust(actor, actor->angle+FixedAngle(locvar1*FRACUNIT), locvar2*FRACUNIT);
}

// Function: A_MoveAbsolute
//
// Description: Moves an object (wrapper for P_InstaThrust)
//
// var1 = angle
// var2 = force
//
void A_MoveAbsolute(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	P_InstaThrust(actor, FixedAngle(locvar1*FRACUNIT), locvar2*FRACUNIT);
}

// Function: A_Thrust
//
// Description: Pushes the object horizontally at its current angle.
//
// var1 = amount of force
// var2 = If 1, xy momentum is lost. If 0, xy momentum is kept
//
void A_Thrust(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!locvar1)
		CONS_Printf("A_Thrust: Var1 not specified!\n");

	if (locvar2)
		P_InstaThrust(actor, actor->angle, locvar1*FRACUNIT);
	else
		P_Thrust(actor, actor->angle, locvar1*FRACUNIT);
}

// Function: A_ZThrust
//
// Description: Pushes the object up or down.
//
// var1 = amount of force
// var2:
//		lower 16 bits = If 1, xy momentum is lost. If 0, xy momentum is kept
//		upper 16 bits = If 1, z momentum is lost. If 0, z momentum is kept
//
void A_ZThrust(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (!locvar1)
		CONS_Printf("A_ZThrust: Var1 not specified!\n");

	if (locvar2)
		actor->momx = actor->momy = 0;

	if (actor->eflags & MFE_VERTICALFLIP)
		actor->z--;
	else
		actor->z++;

	P_SetObjectMomZ(actor, locvar1*FRACUNIT, !(locvar2 >> 16));
}

// Function: A_SetTargetsTarget
//
// Description: Sets your target to the object who has your target targeted. Yikes! If it happens to be NULL, you're just out of luck.
//
// var1 = unused
// var2 = unused
//
void A_SetTargetsTarget(mobj_t *actor)
{
	mobj_t *targetedmobj = NULL;
	thinker_t *th;
	mobj_t *mo2;

	if (!actor->target)
		return;

	if (!actor->target->target)
		return; // Don't search for nothing.

	// scan the thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if (mo2 == actor->target->target)
		{
			targetedmobj = mo2;
			break;
		}
	}

	if (!targetedmobj)
		return; // Oops, nothing found..

	P_SetTarget(&actor->target, targetedmobj);
}

// Function: A_SetObjectFlags
//
// Description: Sets the flags of an object
//
// var1 = flag value to set
// var2:
//		if var2 == 2, add the flag to the current flags
//		else if var2 == 1, remove the flag from the current flags
//		else if var2 == 0, set the flags to the exact value
//
void A_SetObjectFlags(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	P_UnsetThingPosition(actor);
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}

	if (locvar2 == 2)
		actor->flags |= locvar1;
	else if (locvar2 == 1)
		actor->flags &= ~locvar1;
	else
		actor->flags = locvar1;

	P_SetThingPosition(actor);
}

// Function: A_SetObjectFlags2
//
// Description: Sets the flags2 of an object
//
// var1 = flag value to set
// var2:
//		if var2 == 2, add the flag to the current flags
//		else if var2 == 1, remove the flag from the current flags
//		else if var2 == 0, set the flags to the exact value
//
void A_SetObjectFlags2(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (locvar2 == 2)
		actor->flags2 |= locvar1;
	else if (locvar2 == 1)
		actor->flags2 &= ~locvar1;
	else
		actor->flags2 = locvar1;
}

// Function: A_BossJetFume
//
// Description: Spawns jet fumes/other attachment miscellany for the boss. To only be used when he is spawned.
//
// var1:
//		0 - Triple jet fume pattern
//		1 - Boss 3's propeller
// var2 = unused
//
void A_BossJetFume(mobj_t *actor)
{
	mobj_t *filler;
	INT32 locvar1 = var1;

	if (locvar1 == 0) // Boss1 jet fumes
	{
		fixed_t jetx, jety;

		jetx = actor->x + P_ReturnThrustX(actor, actor->angle, -64*FRACUNIT);
		jety = actor->y + P_ReturnThrustY(actor, actor->angle, -64*FRACUNIT);

		filler = P_SpawnMobj(jetx, jety, actor->z + 38*FRACUNIT, MT_JETFUME1);
		P_SetTarget(&filler->target, actor);
		filler->fuse = 56;

		filler = P_SpawnMobj(jetx + P_ReturnThrustX(actor, actor->angle-ANGLE_90, 24*FRACUNIT), jety + P_ReturnThrustY(actor, actor->angle-ANGLE_90, 24*FRACUNIT), actor->z + 12*FRACUNIT, MT_JETFUME1);
		P_SetTarget(&filler->target, actor);
		filler->fuse = 57;

		filler = P_SpawnMobj(jetx + P_ReturnThrustX(actor, actor->angle+ANGLE_90, 24*FRACUNIT), jety + P_ReturnThrustY(actor, actor->angle+ANGLE_90, 24*FRACUNIT), actor->z + 12*FRACUNIT, MT_JETFUME1);
		P_SetTarget(&filler->target, actor);
		filler->fuse = 58;

		P_SetTarget(&actor->tracer, filler);
	}
	else if (locvar1 == 1) // Boss 3 propeller
	{
		fixed_t jetx, jety;

		jetx = actor->x + P_ReturnThrustX(actor, actor->angle, -60*FRACUNIT);
		jety = actor->y + P_ReturnThrustY(actor, actor->angle, -60*FRACUNIT);

		filler = P_SpawnMobj(jetx, jety, actor->z + 17*FRACUNIT, MT_PROPELLER);
		P_SetTarget(&filler->target, actor);
		filler->angle = actor->angle - ANGLE_180;

		P_SetTarget(&actor->tracer, filler);
	}
}

// Function: A_RandomState
//
// Description: Chooses one of the two state numbers supplied randomly.
//
// var1 = state number 1
// var2 = state number 2
//
void A_RandomState(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	P_SetMobjState(actor, P_Random()&1 ? locvar1 : locvar2);
}

// Function: A_RandomStateRange
//
// Description: Chooses a random state within the range supplied.
//
// var1 = Minimum state number to choose.
// var2 = Maximum state number to use.
//
void A_RandomStateRange(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	INT32 statenum;
	INT32 difference = locvar2 - locvar1;

	// Scale P_Random() to the difference.
	statenum = locvar1 + (P_Random() % (difference + 1));

	P_SetMobjState(actor, statenum);
}

// Function: A_DualAction
//
// Description: Calls two actions. Be careful, if you reference the same state this action is called from, you can create an infinite loop.
//
// var1 = state # to use 1st action from
// var2 = state # to use 2nd action from
//
void A_DualAction(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;

	if (cv_debug)
		CONS_Printf("A_DualAction called from object type %d, var1: %d, var2: %d\n", actor->type, locvar1, locvar2);

	var1 = states[locvar1].var1;
	var2 = states[locvar1].var2;

	if (cv_debug)
		CONS_Printf("A_DualAction: Calling First Action (state %d)...\n", locvar1);
	states[locvar1].action.acp1(actor);

	var1 = states[locvar2].var1;
	var2 = states[locvar2].var2;

	if (cv_debug)
		CONS_Printf("A_DualAction: Calling Second Action (state %d)...\n", locvar2);
	states[locvar2].action.acp1(actor);
}

// Function: A_RemoteAction
//
// Description: var1 is the remote object. var2 is the state reference for calling the action called on var1. var1 becomes the actor's target, the action (var2) is called on var1. actor's target is restored
//
// var1 = remote object (-2 uses tracer, -1 uses target)
// var2 = state reference for calling an action
//
void A_RemoteAction(mobj_t *actor)
{
	INT32 locvar1 = var1;
	INT32 locvar2 = var2;
	mobj_t *originaltarget = actor->target; // Hold on to the target for later.

	// If >=0, find the closest target.
	if (locvar1 >= 0)
	{
		///* DO A_FINDTARGET STUFF *///
		mobj_t *targetedmobj = NULL;
		thinker_t *th;
		mobj_t *mo2;
		fixed_t dist1 = 0, dist2 = 0;

		// scan the thinkers
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;

			if (mo2->type == (mobjtype_t)locvar1)
			{
				if (targetedmobj == NULL)
				{
					targetedmobj = mo2;
					dist2 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);
				}
				else
				{
					dist1 = R_PointToDist2(actor->x, actor->y, mo2->x, mo2->y);

					if ((locvar2 && dist1 < dist2) || (!locvar2 && dist1 > dist2))
					{
						targetedmobj = mo2;
						dist2 = dist1;
					}
				}
			}
		}

		if (!targetedmobj)
		{
			if (cv_debug)
				CONS_Printf("A_RemoteAction: Unable to find the specified object to target.\n");
			return; // Oops, nothing found..
		}

		if (cv_debug)
			CONS_Printf("A_FindTarget: Found a target.\n");

		P_SetTarget(&actor->target, targetedmobj);

		///* END A_FINDTARGET STUFF *///
	}

	// If -2, use the tracer as the target
	else if (locvar1 == -2)
		P_SetTarget(&actor->target, actor->tracer);
	// if -1 or anything else, just use the target.

	// Steal the var1 and var2 from "locvar2"
	var1 = states[locvar2].var1;
	var2 = states[locvar2].var2;

	if (cv_debug)
	{
		CONS_Printf("A_RemoteAction: Calling action on %p\n", actor->target);
		CONS_Printf("var1 is %d\nvar2 is %d\n", var1, var2);
	}
	states[locvar2].action.acp1(actor->target);

	P_SetTarget(&actor->target, originaltarget); // Restore the original target.
}

// Function: A_ToggleFlameJet
//
// Description: Turns a flame jet on and off.
//
// var1 = unused
// var2 = unused
//
void A_ToggleFlameJet(mobj_t* actor)
{
	// threshold - off delay
	// movecount - on timer

	if (actor->flags2 & MF2_FIRING)
	{
		actor->flags2 &= ~MF2_FIRING;

		if (actor->threshold)
			actor->tics = actor->threshold;
	}
	else
	{
		actor->flags2 |= MF2_FIRING;

		if (actor->movecount)
			actor->tics = actor->movecount;
	}
}
