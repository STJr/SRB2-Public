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
/// \brief Implements special effects:
///	 - Texture animation, height or lighting changes
///	 according to adjacent sectors, respective
///	 utility functions, etc.

#ifndef __P_SPEC__
#define __P_SPEC__

// GETSECSPECIAL (specialval, section)
//
// Pulls out the special # from a particular section.
//
#define GETSECSPECIAL(i,j) ((i >> ((j-1)*4))&15)

// at game start
void P_InitPicAnims(void);

// at map load (sectors)
void P_SetupLevelFlatAnims(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);
sector_t *P_PlayerTouchingSectorSpecial(player_t *player, int section, int number);
void P_PlayerInSpecialSector(player_t *player);
void P_ProcessSpecialSector(player_t *player, sector_t *sector, sector_t *roversector);

fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);

fixed_t P_FindNextHighestFloor(sector_t *sec, fixed_t currentheight);
fixed_t P_FindNextLowestFloor(sector_t *sec, fixed_t currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);

long P_FindSectorFromLineTag(line_t *line, long start);
long P_FindSectorFromTag(short tag, long start);
long P_FindSpecialLineFromTag(short special, short tag, long start);

int P_FindMinSurroundingLight(sector_t *sector, int max);

void P_SetupSignExit(player_t *player);

void P_SwitchWeather(int weathernum);

void P_LinedefExecute(long tag, mobj_t *actor, sector_t *caller);
void P_ChangeSectorTag(ULONG sector, short newtag);

//
// P_LIGHTS
//
/** Fire flicker action structure.
  */
typedef struct
{
	thinker_t thinker; ///< The thinker in use for the effect.
	sector_t *sector;  ///< The sector where action is taking place.
	long count;
	long resetcount;
	long maxlight;     ///< The brightest light level to use.
	long minlight;     ///< The darkest light level to use.
} fireflicker_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	long maxlight;
	long minlight;
} lightflash_t;

/** Laser block thinker.
  */
typedef struct
{
	thinker_t thinker; ///< Thinker structure for laser.
	ffloor_t *ffloor;  ///< 3Dfloor that is a laser.
	sector_t *sector;  ///< Sector in which the effect takes place.
} laserthink_t;

/** Strobe light action structure..
  */
typedef struct
{
	thinker_t thinker; ///< The thinker in use for the effect.
	sector_t *sector;  ///< The sector where the action is taking place.
	long count;
	long minlight;     ///< The minimum light level to use.
	long maxlight;     ///< The maximum light level to use.
	long darktime;     ///< How long to use minlight.
	long brighttime;   ///< How long to use maxlight.
} strobe_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	long minlight;
	long maxlight;
	long direction;
	long speed;
} glow_t;

/** Thinker struct for fading lights.
  */
typedef struct
{
	thinker_t thinker; ///< Thinker in use for the effect.
	sector_t *sector;  ///< Sector where action is taking place.
	long destlevel;    ///< Light level we're fading to.
	long speed;        ///< Speed at which to change light level.
} lightlevel_t;

#define GLOWSPEED 8
#define STROBEBRIGHT 5
#define FASTDARK 15
#define SLOWDARK 35

void T_FireFlicker(fireflicker_t *flick);
fireflicker_t *P_SpawnAdjustableFireFlicker(sector_t *minsector, sector_t *maxsector, int length);
void T_LightningFlash(lightflash_t *flash);
void T_StrobeFlash(strobe_t *flash);

void P_SpawnLightningFlash(sector_t *sector);
strobe_t * P_SpawnAdjustableStrobeFlash(sector_t *minsector, sector_t *maxsector, int darktime, int brighttime, boolean inSync);

void T_Glow(glow_t *g);
glow_t *P_SpawnAdjustableGlowingLight(sector_t *minsector, sector_t *maxsector, int length);

void P_FadeLight(short tag, int destvalue, int speed);
void T_LightFade(lightlevel_t *ll);

typedef enum
{
	floor_special,
	ceiling_special,
	lighting_special,
} special_e;

//
// P_CEILNG
//
typedef enum
{
	raiseToHighest,
	lowerToLowest,
	raiseToLowest,
	lowerToLowestFast,

	instantRaise, // instant-move for ceilings

	lowerAndCrush,
	crushAndRaise,
	fastCrushAndRaise,
	crushCeilOnce,
	crushBothOnce,

	moveCeilingByFrontSector,
	instantMoveCeilingByFrontSector,

	lowerCeilingByLine,
	raiseCeilingByLine,

	bounceCeiling,
	bounceCeilingCrush,
} ceiling_e;

/** Ceiling movement structure.
  */
typedef struct
{
	thinker_t thinker;    ///< Thinker for the type of movement.
	ceiling_e type;       ///< Type of movement.
	sector_t *sector;     ///< Sector where the action is taking place.
	fixed_t bottomheight; ///< The lowest height to move to.
	fixed_t topheight;    ///< The highest height to move to.
	fixed_t speed;        ///< Ceiling speed.
	fixed_t oldspeed;
	fixed_t delay;
	fixed_t delaytimer;
	boolean crush;        ///< Whether to crush things or not.

	long texture;         ///< The number of a flat to use when done.
	long direction;       ///< 1 = up, 0 = waiting, -1 = down.

	// ID
	long tag;
	long olddirection;

	struct ceilinglist *list;

	fixed_t origspeed;    ///< The original, "real" speed.
	long sourceline;       ///< Index of the source linedef
} ceiling_t;

#define CEILSPEED (FRACUNIT/NEWTICRATERATIO)

int EV_DoCeiling(line_t *line, ceiling_e type);

int EV_DoCrush(line_t *line, ceiling_e type);
void T_CrushCeiling(ceiling_t *ceiling);

void T_MoveCeiling(ceiling_t *ceiling);

//
// P_FLOOR
//
typedef enum
{
	// lower floor to lowest surrounding floor
	lowerFloorToLowest,

	// raise floor to next highest surrounding floor
	raiseFloorToNearestFast,

	// move the floor down instantly
	instantLower,

	moveFloorByFrontSector,
	instantMoveFloorByFrontSector,

	lowerFloorByLine,
	raiseFloorByLine,

	bounceFloor,
	bounceFloorCrush,

	crushFloorOnce,
} floor_e;

typedef enum
{
	elevateUp,
	elevateDown,
	elevateCurrent,
	elevateContinuous,
	elevateBounce,
	elevateHighest,
	bridgeFall,
} elevator_e;

typedef struct
{
	thinker_t thinker;
	floor_e type;
	boolean crush;
	sector_t *sector;
	long direction;
	long texture;
	fixed_t floordestheight;
	fixed_t speed;
	fixed_t origspeed;
	fixed_t delay;
	fixed_t delaytimer;
} floormove_t;

typedef struct
{
	thinker_t thinker;
	elevator_e type;
	sector_t *sector;
	sector_t *actionsector; // The sector the rover action is taking place in.
	long direction;
	fixed_t floordestheight;
	fixed_t ceilingdestheight;
	fixed_t speed;
	fixed_t origspeed;
	fixed_t low;
	fixed_t high;
	fixed_t distance;
	fixed_t delay;
	fixed_t delaytimer;
	fixed_t floorwasheight; // Height the floor WAS at
	fixed_t ceilingwasheight; // Height the ceiling WAS at
	player_t *player; // Player who initiated the thinker (used for airbob)
	line_t *sourceline;
} elevator_t;

// Generic thinker for various level specials
typedef enum
{
	lt_spikes,
} levelspec_e;

typedef struct
{
	thinker_t thinker;
	levelspec_e type;
	fixed_t vars[16];   // Misc. variables
	mobj_t *activator;  // Mobj that activated this thinker
	line_t *sourceline; // Source line of the thinker
	sector_t *sector;   // Sector the thinker is from
} levelspecthink_t;

#define ELEVATORSPEED (FRACUNIT*4/NEWTICRATERATIO)
#define FLOORSPEED (FRACUNIT/NEWTICRATERATIO)

typedef enum
{
	ok,
	crushed,
	pastdest
} result_e;

result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, boolean crush,
	int floorOrCeiling, int direction);
int EV_DoFloor(line_t *line, floor_e floortype);
int EV_DoElevator(line_t *line, elevator_e elevtype, boolean customspeed);
void EV_CrumbleChain(sector_t *sec, ffloor_t *rover);
int EV_BounceSector(sector_t *sector, fixed_t momz, line_t *sourceline);

// Some other special 3dfloor types
int EV_StartCrumble(sector_t *sector, ffloor_t *rover,
	boolean floating, player_t *player, fixed_t origalpha, boolean crumblereturn);

int EV_DoContinuousFall(sector_t *sec, sector_t *pbacksector, fixed_t spd, boolean backwards);

int EV_MarioBlock(sector_t *sector, sector_t *roversector, fixed_t topheight, mobj_t *puncher);

void T_MoveFloor(floormove_t *movefloor);

void T_MoveElevator(elevator_t *elevator);
void T_ContinuousFalling(levelspecthink_t *faller);
void T_BounceCheese(levelspecthink_t *bouncer);
void T_StartCrumble(elevator_t *elevator);
void T_MarioBlock(levelspecthink_t *block);
void T_SpikeSector(levelspecthink_t *spikes);
void T_FloatSector(levelspecthink_t *floater);
void T_BridgeThinker(levelspecthink_t *bridge);
void T_MarioBlockChecker(levelspecthink_t *block);
void T_ThwompSector(levelspecthink_t *thwomp);
void T_NoEnemiesSector(levelspecthink_t *nobaddies);
void T_EachTimeThinker(levelspecthink_t *eachtime);
void T_CameraScanner(elevator_t *elevator);
void T_RaiseSector(levelspecthink_t *sraise);

typedef struct
{
	thinker_t thinker; // Thinker for linedef executor delay
	line_t *line;      // Pointer to line that is waiting.
	mobj_t *caller;    // Pointer to calling mobj
	long timer;        // Delay timer
} executor_t;

void T_ExecutorDelay(executor_t *e);

/** Generalized scroller.
  */
typedef struct
{
	thinker_t thinker;   ///< Thinker structure for scrolling.
	fixed_t dx, dy;      ///< (dx,dy) scroll speeds.
	long affectee;       ///< Number of affected sidedef or sector.
	long control;        ///< Control sector (-1 if none) used to control scrolling.
	fixed_t last_height; ///< Last known height of control sector.
	fixed_t vdx, vdy;    ///< Accumulated velocity if accelerative.
	long accel;          ///< Whether it's accelerative.
	int exclusive;       ///< If a conveyor, same property as in pusher_t
	/** Types of generalized scrollers.
	*/
	enum
	{
		sc_side,         ///< Scroll wall texture on a sidedef.
		sc_floor,        ///< Scroll floor.
		sc_ceiling,      ///< Scroll ceiling.
		sc_carry,        ///< Carry objects on floor.
		sc_carry_ceiling,///< Carry objects on ceiling (for 3Dfloor conveyors).
	} type;
} scroll_t;

void T_Scroll(scroll_t *s);
void T_LaserFlash(laserthink_t *flash);

/** Friction for ice/sludge effects.
  */
typedef struct
{
	thinker_t thinker;  ///< Thinker structure for friction.
	long friction;      ///< Friction value, 0xe800 = normal.
	long movefactor;    ///< Inertia factor when adding to momentum.
	long affectee;      ///< Number of affected sector.
	long referrer;      ///< If roverfriction == true, then this will contain the sector # of the control sector where the effect was applied.
	byte roverfriction; ///< flag for whether friction originated from a FOF or not
} friction_t;

// Friction defines.
#define ORIG_FRICTION          (0xE8 << (FRACBITS-8)) ///< Original value.
#define ORIG_FRICTION_FACTOR   (8 << (FRACBITS-8))    ///< Original value.

void T_Friction(friction_t *f);

typedef enum
{
	p_push,        ///< Point pusher or puller.
	p_wind,        ///< Wind.
	p_current,     ///< Current.
	p_upcurrent,   ///< Upwards current.
	p_downcurrent, ///< Downwards current.
	p_upwind,      ///< Upwards wind.
	p_downwind     ///< Downwards wind.
} pushertype_e;

// Model for pushers for push/pull effects
typedef struct
{
	thinker_t thinker; ///< Thinker structure for push/pull effect.
	/** Types of push/pull effects.
	*/
	pushertype_e type; ///< Type of push/pull effect.
	mobj_t *source;    ///< Point source if point pusher/puller.
	long x_mag;        ///< X strength.
	long y_mag;        ///< Y strength.
	long magnitude;    ///< Vector strength for point pusher/puller.
	long radius;       ///< Effective radius for point pusher/puller.
	long x, y, z;      ///< Point source if point pusher/puller.
	long affectee;     ///< Number of affected sector.
	byte roverpusher;  ///< flag for whether pusher originated from a FOF or not
	long referrer;     ///< If roverpusher == true, then this will contain the sector # of the control sector where the effect was applied.
	int exclusive;     /// < Once this affect has been applied to a mobj, no other pushers may affect it.
	int slider;        /// < Should the player go into an uncontrollable slide?
} pusher_t;

// Model for disappearing/reappearing FOFs
typedef struct
{
	thinker_t thinker;  ///< Thinker structure for effect.
	tic_t appeartime;   ///< Tics to be appeared for
	tic_t disappeartime;///< Tics to be disappeared for
	tic_t offset;       ///< Time to wait until thinker starts
	tic_t timer;        ///< Timer between states
	int affectee;       ///< Number of affected line
	int sourceline;     ///< Number of source line
	int exists;         ///< Exists toggle
} disappear_t;

void T_Disappear(disappear_t *d);

// Prototype functions for pushers
void T_Pusher(pusher_t *p);
mobj_t *P_GetPushThing(ULONG s);

void P_CalcHeight(player_t *player);

sector_t *P_ThingOnSpecial3DFloor(mobj_t *mo);

#endif
