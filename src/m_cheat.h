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
/// \brief Cheat code checking

#ifndef __M_CHEAT__
#define __M_CHEAT__

#include "d_event.h"

boolean cht_Responder(event_t *ev);
void cht_Init(void);

void Command_CheatNoClip_f(void);
void Command_CheatGod_f(void);
void Command_Savecheckpoint_f(void);
void Command_Getallemeralds_f(void);
void Command_Resetemeralds_f(void);
void Command_Unlockall_f(void);
void Command_Devmode_f(void);
void Command_Scale_f(void);
void Command_Gravflip_f(void);
void Command_Hurtme_f(void);
void Command_Charability_f(void);
void Command_Charspeed_f(void);
#ifdef _DEBUG
void Command_CauseCfail_f(void);
#endif

#endif
