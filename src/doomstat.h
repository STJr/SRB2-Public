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
/// \brief All the global variables that store the internal state.
///
///	Theoretically speaking, the internal state of the engine
///	should be found by looking at the variables collected
///	here, and every relevant module will have to include
///	this header file.
///	In practice, things are a bit messy.

#ifndef __DOOMSTAT__
#define __DOOMSTAT__

// We need globally shared data structures, for defining the global state variables.
#include "doomdata.h"

// We need the player data structure as well.
#include "d_player.h"

// =============================
// Selected map etc.
// =============================

// Selected by user.
extern INT16 gamemap;
extern musicenum_t mapmusic;
extern INT16 maptol;
extern UINT8 globalweather;
extern INT32 curWeather;
extern INT32 cursaveslot;
extern INT16 lastmapsaved;
extern boolean gamecomplete;

#define PRECIP_NONE  0
#define PRECIP_STORM 1
#define PRECIP_SNOW  2
#define PRECIP_RAIN  3
#define PRECIP_BLANK 4
#define PRECIP_STORM_NORAIN 5
#define PRECIP_STORM_NOSTRIKES 6
#define PRECIP_HEATWAVE 7

// Set if homebrew PWAD stuff has been added.
extern boolean modifiedgame;
extern UINT16 mainwads;
extern boolean savemoddata; // This mod saves time/emblem data.
extern boolean timeattacking;
extern boolean disableSpeedAdjust; // Don't alter the duration of player states if true
extern boolean imcontinuing; // Temporary flag while continuing to ignore "noreload" mapheaders

// Netgame? only true in a netgame
extern boolean netgame;
extern boolean addedtogame; // true after the server has added you
// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern boolean multiplayer;

extern INT16 gametype;
extern boolean splitscreen;
extern boolean circuitmap; // Does this level have 'circuit mode'?
extern boolean fromlevelselect;
extern INT32 cv_debug;

// ========================================
// Internal parameters for sound rendering.
// ========================================

extern boolean nomidimusic; // defined in d_main.c
extern boolean nosound;
extern boolean nodigimusic;
extern boolean music_disabled;
extern boolean sound_disabled;
extern boolean digital_disabled;

// =========================
// Status flags for refresh.
// =========================
//

extern boolean menuactive; // Menu overlaid?
extern UINT8 paused; // Game paused?

extern boolean nodrawers;
extern boolean noblit;
extern boolean lastdraw;
extern postimg_t postimgtype;
extern INT32 postimgparam;

extern INT32 viewwindowx, viewwindowy;
extern INT32 viewwidth, scaledviewwidth;

extern boolean gamedataloaded;

// Player taking events, and displaying.
extern INT32 consoleplayer;
extern INT32 displayplayer;
extern INT32 secondarydisplayplayer; // for splitscreen

// Maps of special importance
extern INT16 spstage_start;
extern INT16 spstage_end;
extern INT16 sstage_start;
extern INT16 sstage_end;
extern INT16 nsstage_start;
extern INT16 nsstage_end;
extern INT16 racestage_start;

extern boolean looptitle;
extern boolean useNightsSS;

extern tic_t countdowntimer;
extern UINT8 countdowntimeup;

typedef struct
{
	UINT8 numpics;
	char picname[8][8];
	boolean pichires[8];
	char *text;
	UINT16 xcoord[8];
	UINT16 ycoord[8];
	UINT16 picduration[8];
	musicenum_t musicslot;
	boolean musicloop;
	UINT16 textxpos;
	UINT16 textypos;
} scene_t;

typedef struct
{
	scene_t scene[128]; // 128 scenes per cutscene.
	INT32 numscenes; // Number of scenes in this cutscene
} cutscene_t;

extern cutscene_t cutscenes[128];

// For the Custom Exit linedef.
extern INT16 nextmapoverride;
extern INT32 nextmapgametype;
extern boolean skipstats;

extern UINT32 totalrings; //  Total # of rings in a level

// Fun extra stuff
extern INT16 lastmap; // Last level you were at (returning from special stages).
extern mobj_t *redflag, *blueflag; // Pointers to physical flags
extern mapthing_t *rflagpoint, *bflagpoint; // Pointers to the flag spawn locations
#define MF_REDFLAG 1
#define MF_BLUEFLAG 2

#define LEVELARRAYSIZE 1035+2
extern char lvltable[LEVELARRAYSIZE+3][64];

/** Map header information.
  */
typedef struct
{
	// The original eight.
	char lvlttl[33];      ///< Level name without "Zone".
	char subttl[33];      ///< Subtitle for level
	UINT8 actnum;          ///< Act number or 0 for none.
	INT16 typeoflevel;    ///< Combination of typeoflevel flags.
	INT16 nextlevel;      ///< Map number of next level, or 1100-1102 to end.
	musicenum_t musicslot;///< Music slot number to play. 0 for no music.
	UINT8 forcecharacter;  ///< Skin number to switch to or 255 to disable.
	UINT8 weather;         ///< 0 = sunny day, 1 = storm, 2 = snow, 3 = rain, 4 = blank, 5 = thunder w/o rain, 6 = rain w/o lightning, 7 = heat wave.
	INT16 skynum;         ///< Sky number to use.

	// Extra information.
	char interscreen[8];  ///< 320x200 patch to display at intermission.
	char scriptname[192]; ///< Script to use when the map is switched to.
	boolean scriptislump; ///< True if the script is a lump, not a file.
	UINT8 precutscenenum;  ///< Cutscene number to play BEFORE a level starts.
	UINT8 cutscenenum;     ///< Cutscene number to use, 0 for none.
	INT16 countdown;      ///< Countdown until level end?
	boolean nozone;       ///< True to hide "Zone" in level name.
	boolean hideinmenu;   ///< True to hide in the multiplayer menu.
	boolean nossmusic;    ///< True to disable Super Sonic music in this level.
	boolean speedmusic;   ///< Speed up the music for super sneakers?
	boolean noreload;     ///< True to retain level state when you die in single player
	boolean timeattack;   ///< Count in time attack calculations
	boolean levelselect;  ///< Does it appear in the level select?
	boolean noperfectbns; ///< Is the perfect bonus allowed to be obtained?
	char runsoc[64];      ///< SOC to execute at start of level
	UINT16 palette;      ///< PAL lump to use on this map
} mapheader_t;

extern mapheader_t mapheaderinfo[NUMMAPS];

#define TOL_COOP        1 ///< Cooperative
#define TOL_RACE        2 ///< Race
#define TOL_MATCH       4 ///< Match
#define TOL_TAG         8 ///< Tag
#define TOL_CTF        16 ///< Capture the Flag
#ifdef CHAOSISNOTDEADYET
#define TOL_CHAOS      32 ///< Chaos
#endif
#define TOL_NIGHTS     64 ///< NiGHTS
#define TOL_UNUSED    128 ///< (Was Adventure)
#define TOL_MARIO     256 ///< Mario
#define TOL_2D        512 ///< 2D
#define TOL_UNUSED2  1024 ///< (Was Xmas)
#define TOL_ERZ3     2048 ///< ERZ3
#define TOL_SP       4096 ///< Single Player
#define TOL_SRB1     8192 ///< SRB1

// Gametypes
#define GT_COOP  0 // also used in single player
#define GT_MATCH 1
#define GT_RACE  2
#define GT_TAG   3
#define GT_CTF   4 // capture the flag
#ifndef CHAOSISNOTDEADYET
#define NUMGAMETYPES 5
#else
#define GT_CHAOS 5
#define NUMGAMETYPES 6
#endif
// If you alter this list, update gametype_cons_t in m_menu.c

// Fake gametypes
#define GTF_TEAMMATCH 42
#define GTF_CLASSICRACE 43
#define GTF_HIDEANDSEEK 44

// Emeralds stored as bits to throw savegame hackers off.
extern UINT16 emeralds;
extern tic_t totalplaytime;
#define EMERALD1 1
#define EMERALD2 2
#define EMERALD3 4
#define EMERALD4 8
#define EMERALD5 16
#define EMERALD6 32
#define EMERALD7 64
#define ALL7EMERALDS(v) ((v & (EMERALD1|EMERALD2|EMERALD3|EMERALD4|EMERALD5|EMERALD6|EMERALD7)) == (EMERALD1|EMERALD2|EMERALD3|EMERALD4|EMERALD5|EMERALD6|EMERALD7))

#define MAXEMBLEMS 512 // If you have more emblems than this in your game, you seriously need to get a life.
extern INT32 numemblems;

extern INT32 nummaprings; //keep track of spawned rings/coins

/** Hidden emblem/egg structure.
  */
typedef struct
{
	INT16 x; ///< X coordinate.
	INT16 y; ///< Y coordinate.
	INT16 z; ///< Z coordinate.
	UINT8 player;    ///< Player who can access this emblem.
	INT16 level;     ///< Level on which this emblem/egg can be found.
	UINT8 collected; ///< Do you have this emblem?
} emblem_t;

extern emblem_t emblemlocations[MAXEMBLEMS];

/** Time attack information, currently a very small structure.
  */
typedef struct
{
	tic_t time; ///< Time in which the level was finished.
} timeattack_t;

extern timeattack_t timedata[NUMMAPS];
extern UINT8 mapvisited[NUMMAPS];

extern UINT32 token; ///< Number of tokens collected in a level
extern UINT32 tokenlist; ///< List of tokens collected
extern INT32 tokenbits; ///< Used for setting token bits
extern INT32 sstimer; ///< Time allotted in the special stage
extern UINT32 bluescore; ///< Blue Team Scores
extern UINT32 redscore;  ///< Red Team Scores

// Eliminates unnecessary searching.
extern boolean CheckForBustableBlocks;
extern boolean CheckForBouncySector;
extern boolean CheckForQuicksand;
extern boolean CheckForMarioBlocks;
extern boolean CheckForFloatBob;
extern boolean CheckForReverseGravity;

// Powerup durations
extern tic_t invulntics;
extern tic_t sneakertics;
extern INT32 flashingtics;
extern tic_t tailsflytics;
extern INT32 underwatertics;
extern tic_t spacetimetics;
extern tic_t extralifetics;
extern tic_t gravbootstics;
// NiGHTS Powerups
extern tic_t paralooptics;
extern tic_t helpertics;

extern UINT8 introtoplay;
extern UINT8 creditscutscene;

extern mobj_t *hunt1, *hunt2, *hunt3; // Emerald hunt locations

// For racing
extern UINT32 countdown;
extern UINT32 countdown2;

extern fixed_t gravity;

//for CTF balancing
extern INT16 autobalance;
extern INT16 teamscramble;
extern INT16 scrambleplayers[MAXPLAYERS]; //for CTF team scramble
extern INT16 scrambleteams[MAXPLAYERS]; //for CTF team scramble
extern INT16 scrambletotal; //for CTF team scramble
extern INT16 scramblecount; //for CTF team scramble

extern INT32 cheats;

extern INT32 matchtype;
extern INT32 tagtype;
extern tic_t hidetime;

// Grading
// 0 = No grade
// 1 = F
// 2 = E
// 3 = D
// 4 = C
// 5 = B
// 6 = A
// 7 = A+
extern UINT32 grade;

extern UINT32 timesbeaten; // # of times the game has been beaten.

// ===========================
// Internal parameters, fixed.
// ===========================
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern tic_t gametic;
#define localgametic leveltime

// Player spawn spots.
extern mapthing_t *playerstarts[MAXPLAYERS]; // Cooperative
extern mapthing_t *bluectfstarts[MAXPLAYERS]; // CTF
extern mapthing_t *redctfstarts[MAXPLAYERS]; // CTF
extern mapthing_t *tagstarts[MAXPLAYERS]; // Tag

// =====================================
// Internal parameters, used for engine.
// =====================================

#if defined (macintosh)
#define DEBFILE(msg) I_OutputMsg(msg)
extern FILE *debugfile;
#else
#define DEBUGFILE
#ifdef DEBUGFILE
#define DEBFILE(msg) { if (debugfile) { fputs(msg, debugfile); fflush(debugfile); } }
extern FILE *debugfile;
#else
#define DEBFILE(msg) {}
extern FILE *debugfile;
#endif
#endif

#ifdef DEBUGFILE
extern INT32 debugload;
#endif

// if true, load all graphics at level load
extern boolean precache;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t wipegamestate;

// debug flag to cancel adaptiveness
extern boolean singletics;

// =============
// Netgame stuff
// =============

#include "d_clisrv.h"

extern consvar_t cv_timetic; // display high resolution timer
extern consvar_t cv_forceskin; // force clients to use the server's skin
extern consvar_t cv_downloading; // allow clients to downloading WADs.
extern ticcmd_t netcmds[BACKUPTICS][MAXPLAYERS];
extern INT32 adminplayer, serverplayer;

/// \note put these in d_clisrv outright?

#endif //__DOOMSTAT__
