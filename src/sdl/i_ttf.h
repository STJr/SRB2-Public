// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2011 by Callum Dickinson.
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
/// \brief SDL_ttf interface code. Necessary for platforms with SDL inits that need to run immediately.

#ifndef __I_TTF__
#define __I_TTF__

#include "SDL_ttf.h"

// Default name for standard TTF file.
#define FONTFILE "srb2.ttf"
#define FONTPOINTSIZE 12

// Default font foreground colours
#define DEFAULTFONTFGR 255
#define DEFAULTFONTFGG 255
#define DEFAULTFONTFGB 255
#define DEFAULTFONTFGA 255

// Default font background colours
#define DEFAULTFONTBGR 0
#define DEFAULTFONTBGG 0
#define DEFAULTFONTBGB 0
#define DEFAULTFONTBGA 255

#ifndef SDL_TTF_COMPILEDVERSION
#define SDL_TTF_COMPILEDVERSION \
	SDL_VERSIONNUM(TTF_MAJOR_VERSION, TTF_MINOR_VERSION, TTF_PATCHLEVEL)
#endif

#ifndef SDL_TTF_VERSION_ATLEAST
#define SDL_TTF_VERSION_ATLEAST(X, Y, Z) \
	(SDL_TTF_COMPILEDVERSION >= SDL_VERSIONNUM(X, Y, Z))
#endif

TTF_Font* currentfont;
int currentfontpoint;
int currentfontstyle;
#if SDL_TTF_VERSION_ATLEAST(2,0,10)
int currentfontkerning;
int currentfonthinting;
int currentfontoutline;
#endif

#ifndef _PS3
typedef struct
{
	int width;
	int height;
} VideoResolution;
#endif
int bitsperpixel;

typedef enum
{
	solid,
	shaded,
	blended
} TextQuality;

// Load TTF font from file.
int I_TTFLoadFont(char* file, int ptsize);

// Draw TTF text to screen. It will accept four colour vales (red, green, blue and alpha)
// with foreground for draw modes Solid and Blended, and an extra four values for background
// colour with draw type Shaded.
void I_TTFDrawText(TTF_Font *FONTTODRAW, TextQuality TTFQUALITY, int FGR, int FGG, int FGB, int FGA, int BGR, int BGG, int BGB, int BGA, char* text);

// Initialise SDL_ttf.
void I_StartupTTF(int fontpointsize, Uint32 initflags, Uint32 vidmodeflags);

void I_ShutdownTTF(void);
#endif
