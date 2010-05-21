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
/// \brief player data structures

#ifndef __D_PLAYER__
#define __D_PLAYER__

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

// Extra abilities/settings for skins (combinable stuff)
typedef enum
{
	SF_SUPERANIMS       =    1, // If super, use the super sonic animations
	SF_SUPERSPIN        =    2, // Should spin frames be played while super?
	SF_GHOSTTHOKITEM    =    4, // Use ghost feature on thokitem
	SF_GHOSTSPINITEM    =    8, // Use ghost feature on spinitem
} skinflags_t;

//Primary and secondary skin abilities
#define CA_THOK              0
#define CA_FLY               1
#define CA_GLIDEANDCLIMB     2
#define CA_DOUBLEJUMP        3
#define CA_FLOAT             4
#define CA_SLOWFALL          5
#define CA_SWIM              6
#define CA_HOMINGTHOK        7

//Secondary skin abilities
#define CA2_SPINDASH         0
#define CA2_MULTIABILITY     1

//
// Player states.
//
typedef enum
{
	// Playing or camping.
	PST_LIVE,
	// Dead on the ground, view follows killer.
	PST_DEAD,
	// Ready to restart/respawn???
	PST_REBORN
} playerstate_t;

// If more player frames are added after S_PLAY_SUPERTRANS9, update this.
extern INT32 playerstatetics[MAXPLAYERS][S_PLAY_SUPERTRANS9+1];

//
// Player internal flags
//
typedef enum
{
	PF_AUTOAIM           =        0x1,

	// True if button down last tic.
	PF_ATTACKDOWN        =        0x2,
	PF_USEDOWN           =        0x4,
	PF_JUMPDOWN          =        0x8,
	PF_WPNDOWN           =       0x10,

	// No damage, no health loss.
	PF_GODMODE           =       0x20,

	// No clipping, walk through barriers.
	PF_NOCLIP            =       0x40,

	// Did you get a time-over?
	PF_TIMEOVER          =       0x80,

	// Ready for Super?
	PF_SUPERREADY        =      0x100,

	// Are animation frames playing?
	PF_WALKINGANIM       =      0x200,
	PF_RUNNINGANIM       =      0x400,
	PF_SPINNINGANIM      =      0x800,

	// Character action status
	PF_JUMPED            =     0x1000,
	PF_SPINNING          =     0x2000,
	PF_STARTDASH         =     0x4000,
	PF_THOKKED           =     0x8000,

	// Are you gliding?
	PF_GLIDING           =    0x10000,

	// Tails pickup!
	PF_CARRIED           =    0x20000,

	// Sliding (usually in water) like Labyrinth/Oil Ocean
	PF_SLIDING           =    0x40000,

	// Hanging on an item of some kind - zipline, chain, etc. (->tracer)
	PF_ITEMHANG          =    0x80000,

	// On the mace chain spinning around (->tracer)
	PF_MACESPIN          =   0x100000,

	/*** NIGHTS STUFF ***/
	// Is the player in NiGHTS mode?
	PF_NIGHTSMODE        =   0x200000,
	PF_TRANSFERTOCLOSEST =   0x400000,

	// Spill rings after falling
	PF_NIGHTSFALL        =   0x800000,
	PF_DRILLING          =  0x1000000,
	PF_SKIDDOWN          =  0x2000000,

	PF_TAGGED            =  0x4000000, // Player has been tagged and awaits the next round in hide and seek.
	PF_STASIS            =  0x8000000, // Player is not allowed to move in certain tag mode situations.
	PF_TAGIT             = 0x10000000, // The player is it! For Tag Mode

	PF_ROPEHANG          = 0x20000000, // Hanging on a rope
	PF_MINECART          = 0x40000000, // Sonic as a 'mine cart'
} pflags_t;

// Player powers. (don't edit this comment)
typedef enum
{
	pw_invulnerability,
	pw_sneakers,
	pw_flashing,
	pw_jumpshield, // jump shield
	pw_forceshield, // force shield
	pw_tailsfly, // tails flying
	pw_underwater, // underwater timer
	pw_spacetime, // In space, no one can hear you spin!
	pw_extralife, // Extra Life timer
	pw_ringshield, // ring shield
	pw_bombshield, // bomb shield
	pw_watershield, // water shield

	// Sonic 3 types
	pw_lightningshield,
	pw_bubbleshield,
	pw_flameshield,

	pw_super, // Are you super?
	pw_gravityboots, // gravity boots

	// Mario-specific
	pw_fireflower,

	// Weapon ammunition
	pw_bouncering,
	pw_railring,
	pw_automaticring,
	pw_explosionring,
	pw_scatterring,
	pw_grenadering,

	// Power Stones
	pw_emeralds, // stored like global 'emeralds' variable

	// NiGHTS powerups
	pw_superparaloop,
	pw_nightshelper,

	//for linedef exec 427
	pw_nocontrol,
	pw_ingoop, // In goop

	NUMPOWERS
} powertype_t;

#define MAX_BOUNCE    100
#define MAX_RAIL       50
#define MAX_AUTOMATIC 300
#define MAX_EXPLOSION  50
#define MAX_SCATTER   100
#define MAX_GRENADE    50

#define WEP_AUTO    1
#define WEP_BOUNCE  2
#define WEP_SCATTER 3
#define WEP_GRENADE 4
#define WEP_EXPLODE 5
#define WEP_RAIL    6
#define NUM_WEAPONS 7

typedef enum
{
	RW_BOUNCE  =  1,
	RW_RAIL    =  2,
	RW_AUTO    =  4,
	RW_EXPLODE =  8,
	RW_SCATTER = 16,
	RW_GRENADE = 32
} ringweapons_t;

// ========================================================================
//                          PLAYER STRUCTURE
// ========================================================================
typedef struct player_s
{
	mobj_t *mo;

	playerstate_t playerstate;
	ticcmd_t cmd;

	// Determine POV, including viewpoint bobbing during movement.
	// Focal origin above r.z
	fixed_t viewz;
	// Base height above floor for viewz.
	fixed_t viewheight;
	// Bob/squat speed.
	fixed_t deltaviewheight;
	// bounded/scaled total momentum.
	fixed_t bob;

	// Mouse aiming, where the guy is looking at!
	// It is updated with cmd->aiming.
	angle_t aiming;

	angle_t awayviewaiming; // Used for cut-away view

	// This is only used between levels,
	// mo->health is used during levels.
	INT32 health;

	INT32 currentweapon; // current weapon selected.
	INT32 ringweapons; // weapons currently obtained.
	fixed_t tossstrength; // grenade toss strength

	// Power ups. invinc and invis are tic counters.
	INT32 powers[NUMPOWERS];

	// Bit flags.
	// See pflags_t, above.
	pflags_t pflags;

	// For screen flashing (bright).
	INT32 bonuscount;

	// Player skin colorshift, 0-15 for which color to draw player.
	INT32 skincolor;

	INT32 skin;

	UINT32 score; // player score
	INT32 dashspeed; // dashing speed

	INT32 normalspeed; // Normal ground

	INT32 runspeed; // Speed you break into the run animation

	INT32 thrustfactor; // Thrust = thrustfactor * acceleration
	INT32 accelstart; // Starting acceleration if speed = 0.
	INT32 acceleration; // Acceleration

	// See CA_ and CA2_ #ifdefs for more information.
	INT32 charability; // Ability definition
	INT32 charability2; // Secondary ability definition

	UINT32 charflags; // Extra abilities/settings for skins (combinable stuff)
	                 // See SF_ flags

	mobjtype_t thokitem; // Object # to spawn for the thok
	mobjtype_t spinitem; // Object # to spawn for spindash/spinning

	INT32 actionspd; // Speed of thok/glide/fly
	INT32 mindash; // Minimum spindash speed
	INT32 maxdash; // Maximum spindash speed

	INT32 jumpfactor; // How high can the player jump?

#ifndef TRANSFIX
	INT32 starttranscolor; // Start position for the changeable color of a skin
#endif

	INT32 prefcolor; // forced color in single player, default in multi

	INT32 lives;
	INT32 continues; // continues that player has acquired

	INT32 xtralife; // Ring Extra Life counter

	INT32 speed; // Player's speed (distance formula of MOMX and MOMY values)
	INT32 jumping; // Jump counter

	UINT8 secondjump;

	INT32 fly1; // Tails flying
	UINT32 scoreadd; // Used for multiple enemy attack bonus
	tic_t glidetime; // Glide counter for thrust
	INT32 climbing; // Climbing on the wall
	INT32 deadtimer; // End game if game over lasts too long
	INT32 splish; // Don't make splish repeat tons
	tic_t exiting; // Exitlevel timer
	INT32 blackow;

	UINT8 homing; // Are you homing?

	////////////////////////////
	// Conveyor Belt Movement //
	////////////////////////////
	fixed_t cmomx; // Conveyor momx
	fixed_t cmomy; // Conveyor momy
	fixed_t rmomx; // "Real" momx (momx - cmomx)
	fixed_t rmomy; // "Real" momy (momy - cmomy)

	/////////////////////
	// Race Mode Stuff //
	/////////////////////
	INT32 numboxes; // Number of item boxes obtained for Race Mode
	INT32 totalring; // Total number of rings obtained for Race Mode
	tic_t realtime; // integer replacement for leveltime
	UINT32 racescore; // Total of won categories
	UINT32 laps; // Number of laps (optional)

	////////////////////
	// CTF Mode Stuff //
	////////////////////
	INT32 ctfteam; // 0 == Spectator, 1 == Red, 2 == Blue
	UINT16 gotflag; // 1 == Red, 2 == Blue Do you have the flag?

	INT32 dbginfo; // Debugger
	INT32 emeraldhunt; // # of emeralds found

	INT32 weapondelay; // Delay (if any) to fire the weapon again
	INT32 tossdelay;   // Delay (if any) to toss a flag/emeralds again
	INT32 shielddelay; // force shield counter
	tic_t taunttimer; // Delay before you can use the taunt again

	// Starpost information
	INT32 starpostx;
	INT32 starposty;
	INT32 starpostz;
	INT32 starpostnum; // The number of the last starpost you hit
	tic_t starposttime; // Your time when you hit the starpost
	angle_t starpostangle; // Angle that the starpost is facing - you respawn facing this way
	UINT32 starpostbit; // List of starposts hit

	/////////////////
	// NiGHTS Stuff//
	/////////////////
	angle_t angle_pos;
	angle_t old_angle_pos;

	mobj_t *axis1;
	mobj_t *axis2;
	tic_t bumpertime; // Currently being bounced by MT_NIGHTSBUMPER
	INT32 flyangle;
	tic_t drilltimer;
	INT32 linkcount;
	tic_t linktimer;
	INT32 anotherflyangle;
	tic_t nightstime; // How long you can fly as NiGHTS.
	INT32 drillmeter;
	UINT8 drilldelay;
	UINT8 bonustime; // Capsule destroyed, now it's bonus time!
	mobj_t *capsule; // Go inside the capsule
	UINT8 mare; // Current mare

	INT16 lastsidehit, lastlinehit;

	INT32 losscount; // # of times you've lost only 1 ring

	mobjtype_t currentthing; // For the object placement mode

	INT32 onconveyor; // You are on a conveyor belt if nonzero

	mobj_t *awayviewmobj;
	INT32 awayviewtics;

	UINT8 spectator;

	tic_t jointime; // Timer when player joins game to change skin/color
#ifdef HWRENDER
	fixed_t fovadd; // adjust FOV for hw rendering
#endif
} player_t;

#endif
