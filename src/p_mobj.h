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
/// \brief Map Objects, MObj, definition and handling

#ifndef __P_MOBJ__
#define __P_MOBJ__

// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things, from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
	// Call P_TouchSpecialThing when touched.
	MF_SPECIAL          = 0x0001,
	// Blocks.
	MF_SOLID            = 0x0002,
	// Can be hit.
	MF_SHOOTABLE        = 0x0004,
	// Don't use the sector links (invisible but touchable).
	MF_NOSECTOR         = 0x0008,
	// Don't use the blocklinks (inert but displayable)
	MF_NOBLOCKMAP       = 0x0010,
	// Not to be activated by sound, deaf monster.
	MF_AMBUSH           = 0x0020,
	// You can push this object. It can activate switches and things by pushing it on top.
	MF_PUSHABLE         = 0x0040,
	// Object is a boss.
	MF_BOSS             = 0x0080,
	// On level spawning (initial position), hang from ceiling instead of stand on floor.
	MF_SPAWNCEILING     = 0x0100,
	// Don't apply gravity (every tic); object will float, keeping current height
	//  or changing it actively.
	MF_NOGRAVITY        = 0x0200,
	// This object is an ambient sound.
	MF_AMBIENT          = 0x0400,
	// Slide this object when it hits a wall.
	MF_SLIDEME          = 0x0800,
	// Player cheat.
	MF_NOCLIP           = 0x1000,
	// This object does not adhere to regular flag/z properties for object placing.
	MF_SPECIALFLAGS     = 0x2000,
	// Allow moves to any height, no gravity. For active floaters.
	MF_FLOAT            = 0x4000,
	// Monitor powerup icon. These rise a bit.
	MF_BOXICON          = 0x8000,
	// Don't hit same species, explode on block.
	// Player missiles as well as fireballs of various kinds.
	MF_MISSILE          = 0x10000,
	// Item is a spring.
	MF_SPRING           = 0x20000,
	// Bounce off walls and things.
	MF_BOUNCE           = 0x40000,
	// Object uses a high-resolution sprite
	MF_HIRES            = 0x80000,
	// Item box
	MF_MONITOR          = 0x100000,
	// Don't run the thinker for this object.
	MF_NOTHINK          = 0x200000,
	// Fire object. Doesn't harm if you have fire shield.
	MF_FIRE             = 0x400000,
	// Don't adjust z if below or above floorz/ceilingz
	MF_NOCLIPHEIGHT     = 0x800000,
	// This mobj is an enemy!
	MF_ENEMY            = 0x1000000,
	// Scenery (uses scenery thinker).
	MF_SCENERY          = 0x2000000,
	// Player sprites in multiplayer modes are modified
	//  using an internal color lookup table for re-indexing.
	// This flag merely signals to use the proper drawing function for mobj->color.
	// As this got changed from containing the color in the flags itself,
	// some FREE FLAGS opened up: 0x8000000 0x10000000 and 0x20000000
	MF_TRANSLATION      = 0x4000000,
	// for chase camera, don't be blocked by things (partial clipping)
	// (need comma at end of this for SOC editor)
	MF_NOCLIPTHING      = 0x40000000,
	// Run the action thinker on spawn.
	MF_RUNSPAWNFUNC   = 0x80000000,
} mobjflag_t;

typedef enum
{
	MF2_PUSHED         =     0x00000001, // Mobj was already pushed this tic
	MF2_AXIS           =     0x00000002, // It's a NiGHTS axis! (For faster checking)
	MF2_DONTRESPAWN    =     0x00000004, // Don't respawn this object!
	MF2_RINGORHOOP     =     0x00000008, // Ring or Hoop object
	MF2_FRET           =     0x00000010, // Flashing from a previous hit
	MF2_INFLOAT        =     0x00000020, // Floating to a height for a move, don't auto float to target's height.
	MF2_DEBRIS         =     0x00000040, // Splash ring from explosion ring
	MF2_TWOD           =     0x00000080, // Moves like it's in a 2D level
	MF2_NIGHTSPULL     =     0x00000100, // Attracted from a paraloop
	MF2_JUSTATTACKED   =     0x00000200, // can be pushed by other moving mobjs
	MF2_FIRING         =     0x00000400, // turret fire
	MF2_ONMOBJ         =     0x00000800, // mobj is resting on top of another mobj
	MF2_SKULLFLY       =     0x00001000, // Special handling: skull in flight.
	MF2_CHAOSBOSS      =     0x00002000, // Boss has been spawned in chaos mode
	MF2_BOSSNOTRAP     =     0x00004000, // No Egg Trap after boss
	MF2_BOSSFLEE       =     0x00008000, // Boss is fleeing!
	MF2_AUTOMATIC      =     0x00010000, // Thrown ring has automatic properties
	MF2_RAILRING       =     0x00020000, // Thrown ring has rail properties
	MF2_BOUNCERING     =     0x00040000, // Thrown ring has bounce properties
	MF2_EXPLOSION      =     0x00080000, // Thrown ring has explosive properties
	MF2_SCATTER        =     0x00100000, // Thrown ring has scatter properties
	MF2_GRENADE        =     0x00200000, // Thrown ring has grenade properties
	MF2_DONTDRAW       =     0X00400000, // Don't generate a vissprite
	MF2_SLIDEPUSH      =     0x00800000, // MF_PUSHABLE that pushes continuously.
	MF2_SHADOW         =     0x01000000, // Fuzzy draw, makes targeting harder.
	MF2_STANDONME      =     0x02000000, // While not pushable, stand on me anyway.
	MF2_SUPERFIRE      =     0x04000000, // Firing something with Super Sonic-stopping properties. Or, if mobj has MF_MISSILE, this is the actual fire from it.
	MF2_CLASSICPUSH    =     0x08000000, // Drops straight down when object has negative Z.
	MF2_STRONGBOX      =     0x10000000, // Flag used for "strong" random monitors.
	MF2_REFLECTED      =     0x20000000, // Flag for reflected weapons.
	MF2_OBJECTFLIP     =     0x40000000, // Flag for objects that always have flipped gravity.
} mobjflag2_t;

//
// Mobj extra flags
//
typedef enum
{
	// The mobj stands on solid floor (not on another mobj or in air)
	MFE_ONGROUND          = 1,
	// The mobj just hit the floor while falling, this is cleared on next frame
	// (instant damage in lava/slime sectors to prevent jump cheat..)
	MFE_JUSTHITFLOOR      = 2,
	// The mobj stands in a sector with water, and touches the surface
	// this bit is set once and for all at the start of mobjthinker
	MFE_TOUCHWATER        = 4,
	// The mobj stands in a sector with water, and his waist is BELOW the water surface
	// (for player, allows swimming up/down)
	MFE_UNDERWATER        = 8,
	// used for ramp sectors
	MFE_JUSTSTEPPEDDOWN   = 16,
	// Vertically flip sprite/allow upside-down physics
	MFE_VERTICALFLIP      = 32,
} mobjeflag_t;

// Map Object definition.
typedef struct mobj_s
{
	// List: thinker links.
	thinker_t thinker;

	// Info for drawing: position.
	fixed_t x, y, z;

	// More list: links in sector (if needed)
	struct mobj_s *snext;
	struct mobj_s **sprev; // killough 8/11/98: change to ptr-to-ptr

	// More drawing info: to determine current sprite.
	angle_t angle;  // orientation
	spritenum_t sprite; // used to find patch_t and flip value
	UINT32 frame; // frame number, plus bits see p_pspr.h

	struct msecnode_s *touching_sectorlist; // a linked list of sectors where this object appears

	struct subsector_s *subsector; // Subsector the mobj resides in.

	// The closest interval over all contacted sectors (or things).
	fixed_t floorz; // Nearest floor below.
	fixed_t ceilingz; // Nearest ceiling above.

	// For movement checking.
	fixed_t radius;
	fixed_t height;

	// Momentums, used to update position.
	fixed_t momx, momy, momz;
	fixed_t pmomz; // If you're on a moving floor, its "momz" would be here

	INT32 tics; // state tic counter
	state_t *state;
	INT32 flags; // flags from mobjinfo tables

	void *skin; // overrides 'sprite' when non-NULL (for player bodies to 'remember' the skin)
	UINT8 color; // MF_TRANSLATION

	// Interaction info, by BLOCKMAP.
	// Links in blocks (if needed).
	struct mobj_s *bnext;
	struct mobj_s **bprev; // killough 8/11/98: change to ptr-to-ptr

	// Additional pointers for NiGHTS hoops
	struct mobj_s *hnext;
	struct mobj_s *hprev;

	mobjtype_t type;
	const mobjinfo_t *info; // &mobjinfo[mobj->type]

	UINT32 eflags; // extra flags
	INT32 flags2; // MF2_ flags
	INT32 health; // for player this is rings + 1

	// Movement direction, movement generation (zig-zagging).
	angle_t movedir; // 0-7; also used by Deton for up/down angle
	INT32 movecount; // when 0, select a new dir

	struct mobj_s *target; // Thing being chased/attacked (or NULL), and originator for missiles.

	INT32 reactiontime; // If not 0, don't attack yet.

	INT32 threshold; // If >0, the target will be chased no matter what.

	// Additional info record for player avatars only.
	// Only valid if type == MT_PLAYER
	struct player_s *player;

	INT32 lastlook; // Player number last looked for.

	mapthing_t *spawnpoint; // Used for CTF flags, objectplace, and a handful other applications.

	struct mobj_s *tracer; // Thing being chased/attacked for tracers.

	INT32 friction;
	INT32 movefactor;

	INT32 fuse; // Does something in P_MobjThinker on reaching 0.
	INT32 watertop; // top of the water FOF the mobj is in
	INT32 waterbottom; // bottom of the water FOF the mobj is in

	UINT32 mobjnum; // A unique number for this mobj. Used for restoring pointers on save games.

	UINT16 scale;
	UINT16 destscale;
	UINT8 scalespeed;

	// WARNING: New fields must be added separately to savegame.
} mobj_t;

//
// For precipitation
//
// Sometimes this is casted to a mobj_t,
// so please keep the start of the
// structure the same.
//
typedef struct precipmobj_s
{
	// List: thinker links.
	thinker_t thinker;

	// Info for drawing: position.
	fixed_t x, y, z;

	// More list: links in sector (if needed)
	struct precipmobj_s *snext;
	struct precipmobj_s **sprev; // killough 8/11/98: change to ptr-to-ptr

	// More drawing info: to determine current sprite.
	angle_t angle;  // orientation
	spritenum_t sprite; // used to find patch_t and flip value
	INT32 frame; // frame number, plus bits see p_pspr.h

	struct mprecipsecnode_s *touching_sectorlist; // a linked list of sectors where this object appears

	struct subsector_s *subsector; // Subsector the mobj resides in.

	// The closest interval over all contacted sectors (or things).
	fixed_t floorz; // Nearest floor below.
	fixed_t ceilingz; // Nearest ceiling above.

	// For movement checking.
	fixed_t radius; // Fixed at 2*FRACUNIT
	fixed_t height; // Fixed at 4*FRACUNIT

	// Momentums, used to update position.
	fixed_t momx, momy, momz;
	fixed_t invisible; // fixed_t so it uses the same spot as "pmomz."

	INT32 tics; // state tic counter
	state_t *state;
	INT32 flags; // flags from mobjinfo tables
} precipmobj_t;

typedef struct actioncache_s
{
	struct actioncache_s *next;
	struct actioncache_s *prev;
	struct mobj_s *mobj;
	INT32 statenum;
} actioncache_t;

extern actioncache_t actioncachehead;

void P_InitCachedActions(void);
void P_RunCachedActions(void);
void P_AddCachedAction(mobj_t *mobj, INT32 statenum);

// check mobj against water content, before movement code
void P_MobjCheckWater(mobj_t *mobj);

void P_SpawnMapThing(mapthing_t *mthing);
void P_SpawnPlayer(mapthing_t *mthing, INT32 playernum);
void P_SpawnStarpostPlayer(mobj_t *mobj, INT32 playernum);
void P_SpawnHoopsAndRings(mapthing_t *mthing);
void P_SpawnHoopOfSomething(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle);
void P_SpawnPrecipitation(void);
void P_SpawnParaloop(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle, boolean spawncenter, boolean ghostit);
boolean P_SupermanLook4Players(mobj_t *actor);
void P_DestroyRobots(void);
void P_SnowThinker(precipmobj_t *mobj);
void P_RainThinker(precipmobj_t *mobj);
void P_NullPrecipThinker(precipmobj_t *mobj);
void P_RemovePrecipMobj(precipmobj_t *mobj);
void P_SetScale(mobj_t *mobj, UINT16 newscale);
void P_XYMovement(mobj_t *mo);
void P_EmeraldManager(void);

#define MAXHUNTEMERALDS 64
extern mapthing_t *huntemeralds[MAXHUNTEMERALDS];
extern INT32 numhuntemeralds;
extern boolean runemeraldmanager;
extern INT32 numstarposts;
#endif
