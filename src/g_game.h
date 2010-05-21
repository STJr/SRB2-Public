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
/// \brief Game loop, events handling.

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"

extern char gamedatafilename[64];
extern char timeattackfolder[64];
extern char customversionstring[32];
#define GAMEDATASIZE (1*8192)

#ifdef SEENAMES
extern player_t *seenplayer;
#endif
extern char player_names[MAXPLAYERS][MAXPLAYERNAME+1];

extern player_t players[MAXPLAYERS];
extern boolean playeringame[MAXPLAYERS];

// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording
extern boolean demoplayback, demorecording, timingdemo;

// Quit after playing a demo from cmdline.
extern boolean singledemo;

// gametic at level start
extern tic_t levelstarttic;

// for modding?
extern INT16 prevmap, nextmap;
extern INT32 gameovertics;
extern tic_t timeinmap; // Ticker for time spent in level (used for levelcard display)

// used in game menu
extern consvar_t cv_crosshair, cv_crosshair2;
extern consvar_t cv_invertmouse, cv_alwaysfreelook, cv_mousemove;
extern consvar_t cv_sideaxis,cv_turnaxis,cv_moveaxis,cv_lookaxis,cv_fireaxis,cv_firenaxis;
extern consvar_t cv_sideaxis2,cv_turnaxis2,cv_moveaxis2,cv_lookaxis2,cv_fireaxis2,cv_firenaxis2;

// mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED (1<<25)
#define MAXPLMOVE (50/NEWTICRATERATIO)
#define SLOWTURNTICS (6*NEWTICRATERATIO)

// build an internal map name MAPxx from map number
const char *G_BuildMapName(INT32 map);
void G_BuildTiccmd(ticcmd_t *cmd, INT32 realtics);
void G_BuildTiccmd2(ticcmd_t *cmd, INT32 realtics);

// copy ticcmd_t to and fro the normal way
ticcmd_t *G_CopyTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n);
// copy ticcmd_t to and fro network packets
ticcmd_t *G_MoveTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n);

// clip the console player aiming to the view
INT16 G_ClipAimingPitch(INT32 *aiming);

extern angle_t localangle, localangle2;
extern INT32 localaiming, localaiming2; // should be an angle_t but signed

//
// GAME
//
void G_DoReborn(INT32 playernum);
void G_DeathMatchSpawnPlayer(INT32 playernum);
void G_CoopSpawnPlayer(INT32 playernum, boolean starpost);
void G_PlayerReborn(INT32 player);
void G_DoCompleted(void);
void G_InitNew(UINT8 pultmode, const char *mapname, boolean resetplayer,
	boolean skipprecutscene);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1, but a warp test can start elsewhere
void G_DeferedInitNew(boolean pultmode, const char *mapname, INT32 pickedchar,
	boolean SSSG, boolean FLS);
void G_DoLoadLevel(boolean resetplayer);

void G_DeferedPlayDemo(const char *demo);

// Can be called by the startup code or M_Responder, calls P_SetupLevel.
void G_LoadGame(UINT32 slot, INT16 mapoverride);

void G_SaveGameData(void);

void G_SaveGame(UINT32 slot);

// Only called by startup code.
void G_RecordDemo(const char *name);
void G_BeginRecording(void);

void G_DoPlayDemo(char *defdemoname);
void G_TimeDemo(const char *name);
void G_MovieMode(boolean enable);
void G_DoneLevelLoad(void);
void G_StopDemo(void);
boolean G_CheckDemoStatus(void);

boolean G_IsSpecialStage(INT32 mapnum);
void G_ExitLevel(void);
void G_NextLevel(void);
void G_AfterIntermission(void);

void G_Ticker(void);
boolean G_Responder(event_t *ev);

void G_AddPlayer(INT32 playernum);

void G_SetExitGameFlag(void);
void G_ClearExitGameFlag(void);
boolean G_GetExitGameFlag(void);

void G_LoadGameData(void);
void G_LoadGameSettings(void);

void G_SetGamestate(gamestate_t newstate);

FUNCMATH INT32 G_TicsToHours(tic_t tics);
FUNCMATH INT32 G_TicsToMinutes(tic_t tics, boolean full);
FUNCMATH INT32 G_TicsToSeconds(tic_t tics);
FUNCMATH INT32 G_TicsToCentiseconds(tic_t tics);
FUNCMATH INT32 G_TicsToMilliseconds(tic_t tics);

#endif
