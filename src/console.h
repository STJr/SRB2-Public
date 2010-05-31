// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
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
/// \brief Console drawing and input

#include "d_event.h"
#include "command.h"

// for debugging shopuld be replaced by nothing later.. so debug is inactive
#define LOG(x) CONS_Printf(x)

void CON_Init(void);

boolean CON_Responder(event_t *ev);

// set true when screen size has changed, to adapt console
extern boolean con_recalc;

extern boolean con_startup;

// top clip value for view render: do not draw part of view hidden by console
extern INT32 con_clipviewtop;

// 0 means console if off, or moving out
extern INT32 con_destlines;

extern INT32 con_clearlines; // lines of top of screen to refresh
extern boolean con_hudupdate; // hud messages have changed, need refresh

extern consvar_t cons_backcolor;

extern UINT8 *yellowmap, *purplemap, *lgreenmap, *bluemap, *graymap, *redmap, *orangemap;

// Console bg colors:
extern UINT8 *cwhitemap, *corangemap, *cbluemap, *cgreenmap, *cgraymap,
	*credmap;

void CON_ReSetupBackColormap(UINT16 num);
void CON_ClearHUD(void); // clear heads up messages

void CON_Ticker(void);
void CON_Drawer(void);
void CONS_Error(const char *msg); // print out error msg, and wait a key

// force console to move out
void CON_ToggleOff(void);

void CON_LogMessage(const char *msg);
