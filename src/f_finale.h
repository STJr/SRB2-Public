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
/// \brief Title screen, intro, game evaluation, and credits.
///	Also includes protos for screen wipe functions.

#ifndef __F_FINALE__
#define __F_FINALE__

#include "doomtype.h"
#include "d_event.h"

//
// FINALE
//

// Called by main loop.
boolean F_IntroResponder(event_t *ev);
boolean F_CutsceneResponder(event_t *ev);
boolean F_CreditResponder(event_t *ev);

// Called by main loop.
void F_GameEndTicker(void);
void F_IntroTicker(void);
void F_TitleScreenTicker(void);
void F_CutsceneTicker(void);

// Called by main loop.
void F_GameEndDrawer(void);
void F_IntroDrawer(void);
void F_TitleScreenDrawer(void);

void F_GameEvaluationDrawer(void);
void F_StartGameEvaluation(void);
void F_GameEvaluationTicker(void);

void F_CreditTicker(void);
void F_CreditDrawer(void);

void F_StartCustomCutscene(INT32 cutscenenum, boolean precutscene, boolean resetplayer);
void F_CutsceneDrawer(void);
void F_EndCutScene(void);

void F_StartGameEnd(void);
void F_StartIntro(void);
void F_StartTitleScreen(void);
void F_StartCredits(void);

extern INT32 titlescrollspeed;

//
// WIPE
//
extern boolean WipeInAction;

void F_WipeStartScreen(void);
void F_WipeEndScreen(INT32 x, INT32 y, INT32 width, INT32 height);
INT32 F_ScreenWipe(INT32 x, INT32 y, INT32 width, INT32 height, tic_t ticks);
void F_RunWipe(tic_t duration, boolean drawMenu);

#endif
