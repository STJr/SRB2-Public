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
/// \brief Commonly used routines
//
///	Default Config File.
///	PCX Screenshots.
///	File i/o

#ifndef __M_MISC__
#define __M_MISC__

#include "doomtype.h"
#include "w_wad.h"

extern boolean moviemode;

// the file where game vars and settings are saved
#ifdef DC
#define CONFIGFILENAME "srb2dc.cfg"
#elif defined (PSP)
#define CONFIGFILENAME "srb2psp.cfg"
#else
#define CONFIGFILENAME "config.cfg"
#endif

INT32 M_MapNumber(char first, char second);

boolean FIL_WriteFile(char const *name, const void *source, size_t length);
size_t FIL_ReadFile(char const *name, UINT8 **buffer);

boolean FIL_FileExists(const char *name);
boolean FIL_WriteFileOK(char const *name);
boolean FIL_ReadFileOK(char const *name);
boolean FIL_FileOK(char const *name);

void FIL_DefaultExtension (char *path, const char *extension);
void FIL_ForceExtension(char *path, const char *extension);
boolean FIL_CheckExtension(const char *in);

#ifdef HAVE_PNG
boolean M_SavePNG(const char *filename, void *data, int width, int height, const UINT8 *palette);
boolean M_StartMovie(void);
void M_SaveFrame(void);
boolean M_StopMovie(void);
#endif

extern boolean takescreenshot;
void M_ScreenShot(void);
void M_DoScreenShot(void);

extern char configfile[MAX_WADPATH];

void Command_SaveConfig_f(void);
void Command_LoadConfig_f(void);
void Command_ChangeConfig_f(void);

void M_FirstLoadConfig(void);
// save game config: cvars, aliases..
void M_SaveConfig(const char *filename);

// s1 = s2+s3+s1 (1024 lenghtmax)
void strcatbf(char *s1, const char *s2, const char *s3);

void M_SetupMemcpy(void);

#endif
