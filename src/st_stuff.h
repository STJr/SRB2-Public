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
/// \brief Status bar header

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "d_player.h"
#include "r_defs.h"

//
// STATUS BAR
//

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// Called by G_Responder() when pressing F12 while viewing a demo.
void ST_changeDemoView(void);

void ST_UnloadGraphics(void);
void ST_LoadGraphics(void);
void ST_ReloadSkinFaceGraphics(void);

// face load graphics, called when skin changes
void ST_LoadFaceGraphics(char *facestr, char *superstr, INT32 playernum);
void ST_UnLoadFaceGraphics(INT32 skinnum);
void ST_LoadFaceNameGraphics(char *facestr, INT32 playernum);
void ST_UnLoadFaceNameGraphics(INT32 skinnum);
void ST_doPaletteStuff(void);

// return if player a is in the same team as player b
boolean ST_SameTeam(player_t *a, player_t *b);

//--------------------
// status bar overlay
//--------------------

// Draw a number, scaled, over the view
// Always draw the number completely since it's overlay
void ST_DrawOverlayNum(INT32 x /* right border */, INT32 y, INT32 num,
	patch_t **numpat);

extern boolean st_overlay; // sb overlay on or off when fullscreen

extern lumpnum_t st_borderpatchnum;
// patches, also used in intermission
extern patch_t *tallnum[10];
extern patch_t *sboscore;
extern patch_t *sbotime;
extern patch_t *sbocolon;
extern patch_t *faceprefix[MAXSKINS]; // face status patches
extern patch_t *superprefix[MAXSKINS]; // super face status patches

/** HUD location information (don't move this comment)
  */
typedef struct
{
	INT32 x, y;
} hudinfo_t;

typedef enum
{
	HUD_LIVESNAME,
	HUD_LIVESPIC,
	HUD_LIVESNUM,
	HUD_LIVESX,
	HUD_RINGSSPLIT,
	HUD_RINGSNUMSPLIT,
	HUD_RINGS,
	HUD_RINGSNUM,
	HUD_SCORE,
	HUD_SCORENUM,
	HUD_TIMESPLIT,
	HUD_LOWSECONDSSPLIT,
	HUD_SECONDSSPLIT,
	HUD_MINUTESSPLIT,
	HUD_TIMECOLONSPLIT,
	HUD_TIME,
	HUD_LOWTICS,
	HUD_TICS,
	HUD_LOWSECONDS,
	HUD_SECONDS,
	HUD_MINUTES,
	HUD_TIMECOLON,
	HUD_TIMETICCOLON,
	HUD_SS_TOTALRINGS_SPLIT,
	HUD_SS_TOTALRINGS,
	HUD_GETRINGS,
	HUD_GETRINGSNUM,
	HUD_TIMELEFT,
	HUD_TIMELEFTNUM,
	HUD_TIMEUP,
	HUD_HUNTPIC1,
	HUD_HUNTPIC2,
	HUD_HUNTPIC3,
	HUD_GRAVBOOTSICO,
	HUD_LAP,

	NUMHUDITEMS
} hudnum_t;

extern hudinfo_t hudinfo[NUMHUDITEMS];

extern UINT16 objectsdrawn;

#endif
