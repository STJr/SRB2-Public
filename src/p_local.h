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
/// \brief Play functions, animation, global header

#ifndef __P_LOCAL__
#define __P_LOCAL__

#include "command.h"
#include "d_player.h"
#include "d_think.h"
#include "m_fixed.h"
#include "m_bbox.h"
#include "p_tick.h"
#include "r_defs.h"
#include "p_maputl.h"

#define FLOATSPEED (FRACUNIT*4)

#define VIEWHEIGHT 41
#define VIEWHEIGHTS "41"

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS 128
#define MAPBLOCKSIZE  (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT (FRACBITS+7)
#define MAPBMASK      (MAPBLOCKSIZE-1)
#define MAPBTOFRAC    (MAPBLOCKSHIFT-FRACBITS)

// player radius used only in am_map.c
#define PLAYERRADIUS (16*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS (32*FRACUNIT)

#define MAXMOVE (60*FRACUNIT/NEWTICRATERATIO)

// max Z move up or down without jumping
// above this, a height difference is considered as a 'dropoff'
#define MAXSTEPMOVE (24*FRACUNIT)

#define USERANGE (64*FRACUNIT)
#define MELEERANGE (64*FRACUNIT)
#define MISSILERANGE (32*64*FRACUNIT)

#define AIMINGTOSLOPE(aiming) FINESINE((aiming>>ANGLETOFINESHIFT) & FINEMASK)
#define FIXEDSCALE(x,y) FixedDiv(FixedMul((y)<<FRACBITS,(x)),100<<FRACBITS)
#define FIXEDUNSCALE(x,y) ((y) == 0 ? 0 : FixedDiv(FixedMul((100<<FRACBITS),(x)),(y)<<FRACBITS))

#define mariomode (maptol & TOL_MARIO)
#define twodlevel (maptol & TOL_2D)

//
// P_TICK
//

// both the head and tail of the thinker list
extern thinker_t thinkercap;
extern int runcount;

void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);

//
// P_USER
//
typedef struct camera_s
{
	boolean chase;
	angle_t aiming;

	// Things used by FS cameras.
	fixed_t viewheight;
	angle_t startangle;

	// Camera demobjerization
	// Info for drawing: position.
	fixed_t x, y, z;

	//More drawing info: to determine current sprite.
	angle_t angle; // orientation

	struct subsector_s *subsector;

	// The closest interval over all contacted Sectors (or Things).
	fixed_t floorz;
	fixed_t ceilingz;

	// For movement checking.
	fixed_t radius;
	fixed_t height;

	fixed_t relativex;

	// Momentums, used to update position.
	fixed_t momx, momy, momz;
} camera_t;

extern camera_t camera, camera2;
extern consvar_t cv_cam_dist, cv_cam_still, cv_cam_height;
extern consvar_t cv_cam_speed, cv_cam_rotate, cv_cam_rotspeed;

extern consvar_t cv_cam2_dist, cv_cam2_still, cv_cam2_height;
extern consvar_t cv_cam2_speed, cv_cam2_rotate, cv_cam2_rotspeed;

extern fixed_t t_cam_dist, t_cam_height, t_cam_rotate;
extern fixed_t t_cam2_dist, t_cam2_height, t_cam2_rotate;

fixed_t P_GetPlayerHeight(player_t *player);
fixed_t P_GetPlayerSpinHeight(player_t *player);
int P_GetPlayerControlDirection(player_t *player);
void P_AddPlayerScore(player_t *player, ULONG amount);
void P_ResetCamera(player_t *player, camera_t *thiscam);
int P_TryCameraMove(fixed_t x, fixed_t y, camera_t *thiscam);
void P_SlideCameraMove(camera_t *thiscam);
void P_MoveChaseCamera(player_t *player, camera_t *thiscam, boolean netcalled);
void P_DoPlayerPain(player_t *player, mobj_t *source, mobj_t *inflictor);
void P_ResetPlayer(player_t *player);
boolean P_IsLocalPlayer(player_t *player);
boolean P_IsObjectOnGround(mobj_t *mo);
void P_SetObjectMomZ(mobj_t *mo, fixed_t value, boolean relative);
void P_RestoreMusic(player_t *player);
void P_SpawnShieldOrb(player_t *player);
mobj_t *P_SpawnGhostMobj(mobj_t *mobj);
void P_GivePlayerRings(player_t *player, int num_rings, boolean flingring);
void P_GivePlayerLives(player_t *player, int numlives);
void P_GiveEmerald(void);
void P_ResetScore(player_t *player);
boolean P_FreezeObjectplace(void);

void P_PlayerThink(player_t *player);
void P_PlayerAfterThink(player_t *player);
void P_DoPlayerExit(player_t *player);
void P_NightserizePlayer(player_t *player, int ptime);

void P_InstaThrust(mobj_t *mo, angle_t angle, fixed_t move);
fixed_t P_ReturnThrustX(mobj_t *mo, angle_t angle, fixed_t move);
fixed_t P_ReturnThrustY(mobj_t *mo, angle_t angle, fixed_t move);
void P_InstaThrustEvenIn2D(mobj_t *mo, angle_t angle, fixed_t move);

boolean P_LookForEnemies(player_t *player);
void P_NukeEnemies(player_t *player);
void P_HomingAttack(mobj_t *source, mobj_t *enemy); /// \todo doesn't belong in p_user
void P_DoJump(player_t *player, boolean soundandstate);
boolean P_TransferToNextMare(player_t *player);
byte P_FindLowestMare(void);
void P_FindEmerald(void);
void P_TransferToAxis(player_t *player, int axisnum);
boolean P_PlayerMoving(int pnum);

//
// P_MOBJ
//
#define ONFLOORZ MININT
#define ONCEILINGZ MAXINT

// Time interval for item respawning.
// WARNING MUST be a power of 2
#define ITEMQUESIZE 1024

extern mapthing_t *itemrespawnque[ITEMQUESIZE];
extern tic_t itemrespawntime[ITEMQUESIZE];
extern size_t iquehead, iquetail;
extern consvar_t cv_gravity, cv_viewheight;

void P_RespawnSpecials(void);

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);

void P_RecalcPrecipInSector(sector_t *sector);

void P_RemoveMobj(mobj_t *th);
void P_RemoveSavegameMobj(mobj_t *th);
boolean P_SetPlayerMobjState(mobj_t *mobj, statenum_t state);
boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_RunShields(void);
void P_MobjThinker(mobj_t *mobj);
void P_RailThinker(mobj_t *mobj);
void P_PushableThinker(mobj_t *mobj);
void P_SceneryThinker(mobj_t *mobj);
boolean P_InsideANonSolidFFloor(mobj_t *mobj, ffloor_t *rover);

mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnXYZMissile(mobj_t *source, mobj_t *dest, mobjtype_t type, fixed_t x, fixed_t y, fixed_t z);
mobj_t *P_SPMAngle(mobj_t *source, mobjtype_t type, angle_t angle, boolean noaiming, boolean noautoaim, int flags2, boolean reflected);
#define P_SpawnPlayerMissile(s,t,f,r) P_SPMAngle(s,t,s->angle,false,false,f,r)
void P_ColorTeamMissile(mobj_t *missile, player_t *source);

void P_CameraThinker(camera_t *thiscam);

void P_Attract(mobj_t *source, mobj_t *enemy, boolean nightsgrab);
mobj_t *P_GetClosestAxis(mobj_t *source);

//
// P_ENEMY
//

// main player in game
extern player_t *stplyr; // for splitscreen correct palette changes and overlay

// Is there a better place for these?
extern int var1;
extern int var2;

boolean P_LookForPlayers(mobj_t *actor, boolean allaround, boolean tracer, fixed_t dist);

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;
extern fixed_t tmsectorceilingz; //added : 28-02-98: p_spawnmobj
extern boolean tmsprung;
extern mobj_t *tmfloorthing, *tmthing;

/* cphipps 2004/08/30 */
extern void P_MapStart(void);
extern void P_MapEnd(void);

extern line_t *ceilingline;
extern line_t *blockingline;
extern msecnode_t *sector_list;

extern mprecipsecnode_t *precipsector_list;

void P_UnsetThingPosition(mobj_t *thing);
void P_SetThingPosition(mobj_t *thing);

boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
boolean P_CheckCameraPosition(fixed_t x, fixed_t y, camera_t *thiscam);
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, boolean allowdropoff);
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z);
void P_SlideMove(mobj_t *mo);
void P_BounceMove(mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void P_CheckHoopPosition(mobj_t *hoopthing, fixed_t x, fixed_t y, fixed_t z, fixed_t radius);

boolean P_CheckSector(sector_t *sector, boolean crunch);

void P_DelSeclist(msecnode_t *node);
void P_DelPrecipSeclist(mprecipsecnode_t *node);

void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y);
void P_Initsecnode(void);

extern mobj_t *linetarget; // who got hit (or NULL)
extern fixed_t attackrange;

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance);
void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);

fixed_t P_FloorzAtPos(fixed_t x, fixed_t y, fixed_t z, fixed_t height);
boolean PIT_PushableMoved(mobj_t *thing);

//
// P_SETUP
//
extern byte *rejectmatrix; // for fast sight rejection
extern long *blockmaplump; // offsets in blockmap are from here
extern long *blockmap; // Big blockmap
extern int bmapwidth;
extern int bmapheight; // in mapblocks
extern fixed_t bmaporgx;
extern fixed_t bmaporgy; // origin of block map
extern mobj_t **blocklinks; // for thing chains

//
// P_INTER
//
typedef struct BasicFF_s
{
	long ForceX; ///< The X of the Force's Vel
	long ForceY; ///< The Y of the Force's Vel
	const player_t *player; ///< Player of Rumble
	//All
	unsigned long Duration; ///< The total duration of the effect, in microseconds
	long Gain; ///< /The gain to be applied to the effect, in the range from 0 through 10,000.
	//All, CONSTANTFORCE –10,000 to 10,000
	long Magnitude; ///< Magnitude of the effect, in the range from 0 through 10,000.
} BasicFF_t;

void P_ForceFeed(const player_t *player, int attack, int fade, tic_t duration, int period);
void P_ForceConstant(const BasicFF_t *FFInfo);
void P_RampConstant(const BasicFF_t *FFInfo, int Start, int End);
boolean P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage);
void P_KillMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source);
void P_PlayerRingBurst(player_t *player, int num_rings); /// \todo better fit in p_user.c
void P_PlayerEmeraldBurst(player_t *player, boolean toss);

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, boolean heightcheck);
void P_PlayerFlagBurst(player_t *player, boolean toss);
void P_CheckPointLimit(void);
void P_CheckSurvivors(void);
boolean P_CheckRacers(void);

void P_PlayRinglossSound(mobj_t *source);
void P_PlayDeathSound(mobj_t *source);
void P_PlayVictorySound(mobj_t *source);
void P_PlayTauntSound(mobj_t *source);

void P_ClearStarPost(player_t *player, int postnum);

//
// P_SIGHT
//

// slopes to top and bottom of target
extern fixed_t topslope;
extern fixed_t bottomslope;

//
// P_SPEC
//
#include "p_spec.h"

extern int ceilmovesound;

mobj_t *P_CheckOnmobj(mobj_t *thing);
void P_MixUp(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle);
boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle, boolean flash, boolean dontstopmove);
boolean P_SetMobjStateNF(mobj_t *mobj, statenum_t state);
boolean P_CheckMissileSpawn(mobj_t *th);
void P_Thrust(mobj_t *mo, angle_t angle, fixed_t move);
fixed_t P_ScaleMomentum(USHORT scale, fixed_t momentum);
void P_DoSuperTransformation(player_t *player, boolean giverings);
void P_ExplodeMissile(mobj_t *mo);
void P_CheckGravity(mobj_t *mo, boolean affect);

#endif // __P_LOCAL__
