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

#include "doomdef.h"
#include "doomstat.h"
#include "am_map.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "r_local.h"
#include "s_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_system.h"
#include "m_menu.h"
#include "dehacked.h"
#include "g_input.h"
#include "console.h"
#include "m_random.h"
#include "y_inter.h"

/** A screen of credits.
  */
typedef struct
{
	const char *header;        ///< Header text.
	UINT8 numnames;            ///< Number of names on this screen.
	UINT8 smallnames;          ///< Use small text to write names
	const char *fakenames[32]; ///< Nicknames, e.g. Graue, Alam_GBC.
	const char *realnames[32]; ///< Real names.
} credit_t;

// the array of credits of the game and mod
static credit_t credits[19];

// Stage of animation:
// 0 = text, 1 = art screen
static INT32 finalestage;
static INT32 finalecount;
INT32 titlescrollspeed = 80;

static INT32 timetonext; // Delay between screen changes
static INT32 finaletextcount;
static tic_t animtimer; // Used for some animation timings
static INT32 roidtics; // Asteroid spinning

static INT32 deplete;
static tic_t stoptimer;

// Intro
#define NUMINTROSCENES 16
static INT32 scene;
const char *intro01text = NULL;
const char *intro02text = NULL;
const char *intro03text = NULL;
const char *intro04text = NULL;
const char *intro05text = NULL;
const char *intro06text = NULL;
const char *intro07text = NULL;
const char *intro08text = NULL;
const char *intro09text = NULL;
const char *intro10text = NULL;
const char *intro11text = NULL;
const char *intro12text = NULL;
const char *intro13text = NULL;
const char *intro14text = NULL;
const char *intro15text = NULL;
const char *intro16text = NULL;

static tic_t introscenetime[NUMINTROSCENES] =
{
	5*TICRATE,					// STJr Presents
	10*TICRATE + (TICRATE/2),	// Two months had passed since...
	13*TICRATE + (TICRATE/2),	// As it was about to drain the rings...
	15*TICRATE,					// What Sonic, Tails, and Knuckles...
	15*TICRATE + (TICRATE/2),	// About once every year, a strange...
	18*TICRATE + (TICRATE/2),	// Curses! Eggman yelled. That ridiculous...
	20*TICRATE + (TICRATE/4)*3,	// It was only later that he had an idea...
	9*TICRATE + (TICRATE/2),	// Before beginning his scheme, Eggman decided to give Sonic...
	15*TICRATE,					// We're ready to fire in 15 seconds, the robot said...
	16*TICRATE,					// Meanwhile, Sonic was tearing across the zones...
	14*TICRATE,					// Sonic knew he was getting closer to the city...
	13*TICRATE,					// Greenflower City was gone...
	19*TICRATE,					// You're not quite as dead as we thought, huh?...
	17*TICRATE + (TICRATE/2),	// Eggman took this as his cue and blasted off...
	15*TICRATE,					// Easy! We go find Eggman and stop his...
	24*TICRATE,					// I'm just finding what mission obje...
};

#define TEXTSPEED 3
#define TEXTWAIT 250

static const char *finaletext = NULL;
static boolean keypressed = false;

static patch_t *currentanim;
static patch_t *nextanim;
static patch_t *first;
static patch_t *second;
static patch_t *third;

// De-Demo'd Title Screen
static patch_t *ttbanner; // white banner with "robo blast" and "2"
static patch_t *ttwing; // wing background
static patch_t *ttsonic; // "SONIC"
static patch_t *ttswave1; // Title Sonics
static patch_t *ttswave2;
static patch_t *ttswip1;
static patch_t *ttsprep1;
static patch_t *ttsprep2;
static patch_t *ttspop1;
static patch_t *ttspop2;
static patch_t *ttspop3;
static patch_t *ttspop4;
static patch_t *ttspop5;
static patch_t *ttspop6;
static patch_t *ttspop7;

static void F_SkyScroll(INT32 scrollspeed);

static boolean drawemblem = false, drawchaosemblem = false, runningprecutscene = false, precutresetplayer = false;

// De-Demo'd Title Screen
void F_StartTitleScreen(void)
{
	G_SetGamestate(GS_TITLESCREEN);
	CON_ClearHUD();

	// IWAD dependent stuff.

	S_ChangeMusic(mus_titles, looptitle);

	finalecount = 0;
	finalestage = 0;
	animtimer = 0;

	ttbanner = W_CachePatchName("TTBANNER", PU_LEVEL);
	ttwing = W_CachePatchName("TTWING", PU_LEVEL);
	ttsonic = W_CachePatchName("TTSONIC", PU_LEVEL);
	ttswave1 = W_CachePatchName("TTSWAVE1", PU_LEVEL);
	ttswave2 = W_CachePatchName("TTSWAVE2", PU_LEVEL);
	ttswip1 = W_CachePatchName("TTSWIP1", PU_LEVEL);
	ttsprep1 = W_CachePatchName("TTSPREP1", PU_LEVEL);
	ttsprep2 = W_CachePatchName("TTSPREP2", PU_LEVEL);
	ttspop1 = W_CachePatchName("TTSPOP1", PU_LEVEL);
	ttspop2 = W_CachePatchName("TTSPOP2", PU_LEVEL);
	ttspop3 = W_CachePatchName("TTSPOP3", PU_LEVEL);
	ttspop4 = W_CachePatchName("TTSPOP4", PU_LEVEL);
	ttspop5 = W_CachePatchName("TTSPOP5", PU_LEVEL);
	ttspop6 = W_CachePatchName("TTSPOP6", PU_LEVEL);
	ttspop7 = W_CachePatchName("TTSPOP7", PU_LEVEL);
}

// Game end thingy
void F_StartGameEnd(void)
{
	G_SetGamestate(GS_GAMEEND);

	gameaction = ga_nothing;
	playerdeadview = false;
	paused = false;
	CON_ToggleOff();
	CON_ClearHUD();
	S_StopMusic();

	timetonext = TICRATE;
}

void F_StartGameEvaluation(void)
{
	G_SetGamestate(GS_EVALUATION);

	gameaction = ga_nothing;
	playerdeadview = false;
	paused = false;
	CON_ToggleOff();
	CON_ClearHUD();

	if (ALL7EMERALDS(emeralds))
		animtimer = 64;

	finalecount = 0;
}

void F_StartCredits(void)
{
	size_t i = 0;
	size_t j = 0;

	if (creditscutscene)
	{
		F_StartCustomCutscene(creditscutscene - 1, false, false);
		return;
	}

	G_SetGamestate(GS_CREDITS);

	gameaction = ga_nothing;
	playerdeadview = false;
	paused = false;
	CON_ToggleOff();
	CON_ClearHUD();
	S_StopMusic();

	if (!modifiedgame && (grade & 8) && ALL7EMERALDS(emeralds))
		S_ChangeMusic(mus_mapl4m, false);
	else
		S_ChangeMusic(mus_credit, false);

	finalecount = 0;
	animtimer = 0;

	timetonext = 5*TICRATE-1;

	memset(credits, 0, sizeof(credit_t)*19);

	// Initalize the credits table
	// <Callum> Should we make the names of the people translatable?
	credits[i].header = M_GetText("Sonic Team Junior\n");
	credits[i].fakenames[j++] = M_GetText("Staff\n");
	j = 0;
	credits[i].realnames[j++] = M_GetText("Staff\n");
	credits[i].numnames = 1;
	i++;
	credits[i].header = M_GetText("Producer\n");
	j = 0;
	credits[i].fakenames[j++] = "SSNTails\n";
	credits[i].fakenames[j++] = "\n";
	credits[i].fakenames[j++] = M_GetText("Director\n");
	credits[i].fakenames[j++] = "Sonikku\n";
	j = 0;
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].realnames[j++] = "\n";
	credits[i].realnames[j++] = M_GetText( "Director\n");
	credits[i].realnames[j++] = "Johnny Wallbank\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Game Designers\n");
	j = 0;
	credits[i].fakenames[j++] = "Sonikku\n";
	credits[i].fakenames[j++] = "SSNTails\n";
	credits[i].fakenames[j++] = "Mystic\n";
	j = 0;
	credits[i].realnames[j++] = "Johnny Wallbank\n";
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].realnames[j++] = "Ben Geyer\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Character Designers\n");
	j = 0;
	credits[i].fakenames[j++] = "Sonikku\n";
	credits[i].fakenames[j++] = "Instant Sonic\n";
	j = 0;
	credits[i].realnames[j++] = "Johnny Wallbank\n";
	credits[i].realnames[j++] = "David Spencer Jr\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Visual Design\n");
	j = 0;
	credits[i].fakenames[j++] = "SSNTails\n";
	j = 0;
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Landscape Design\n");
	j = 0;
	credits[i].fakenames[j++] = "Sonikku\n";
	credits[i].fakenames[j++] = "JEV3\n";
	j = 0;
	credits[i].realnames[j++] = "Johnny Wallbank\n";
	credits[i].realnames[j++] = "Jarrett Voight\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Chief Programmer\n");
	j = 0;
	credits[i].fakenames[j++] = "SSNTails\n";
	j = 0;
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Programmers\n");
	j = 0;
	credits[i].fakenames[j++] = "Alam_GBC\n";
	credits[i].fakenames[j++] = "Jazz\n";
	credits[i].fakenames[j++] = "Graue\n";
	credits[i].fakenames[j++] = "Inuyasha\n";
	credits[i].fakenames[j++] = "Orospakr\n";
	credits[i].fakenames[j++] = "Callum\n";
	j = 0;
	credits[i].realnames[j++] = "Alam Arias\n";
	credits[i].realnames[j++] = "Nathan Giroux\n";
	credits[i].realnames[j++] = "Scott Feeney\n";
	credits[i].realnames[j++] = "Matthew Walsh\n";
	credits[i].realnames[j++] = "Andrew Clunis\n";
	credits[i].realnames[j++] = "Callum Dickinson\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Coding Assistants\n");
	j = 0;
	credits[i].fakenames[j++] = "StroggOnMeth\n";
	credits[i].fakenames[j++] = "Cyan Helkaraxe\n";
	credits[i].fakenames[j++] = "Logan_GBA\n";
	credits[i].fakenames[j++] = "Shuffle\n";
	credits[i].fakenames[j++] = "Oogaland\n";
	credits[i].fakenames[j++] = "Jason the Echidna\n";
	j = 0;
	credits[i].realnames[j++] = "Steven McGranahan\n";
	credits[i].realnames[j++] = "Cyan Helkaraxe\n";
	credits[i].realnames[j++] = "Logan Arias\n";
	credits[i].realnames[j++] = "Matt Marsalko\n";
	credits[i].realnames[j++] = "Gregor Dick\n";
	credits[i].realnames[j++] = "John J. Muniz\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Level Designers\n");
	j = 0;
	credits[i].fakenames[j++] = "Sonikku\n";
	credits[i].fakenames[j++] = "Mystic\n";
	credits[i].fakenames[j++] = "SSNTails\n";
	credits[i].fakenames[j++] = "Digiku\n";
	credits[i].fakenames[j++] = "Torgo\n";
	credits[i].fakenames[j++] = "Nev3r\n";
	credits[i].fakenames[j++] = "JEV3\n";
	credits[i].fakenames[j++] = "Spazzo\n";
	credits[i].fakenames[j++] = "Inuyasha\n";
	credits[i].fakenames[j++] = "Jazz\n";
	credits[i].fakenames[j++] = "Blitzzo\n";
	j = 0;
	credits[i].realnames[j++] = "Johnny Wallbank\n";
	credits[i].realnames[j++] = "Ben Geyer\n";
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].realnames[j++] = "Marco Zafra\n";
	credits[i].realnames[j++] = "Erik Nielsen\n";
	credits[i].realnames[j++] = "Pedro Iceta\n";
	credits[i].realnames[j++] = "Jarrett Voight\n";
	credits[i].realnames[j++] = "Michael Antonakes\n";
	credits[i].realnames[j++] = "Matthew Walsh\n";
	credits[i].realnames[j++] = "Nathan Giroux\n";
	credits[i].realnames[j++] = "Dan Hagerstrand\n";
	credits[i].numnames = (UINT8)j;
	credits[i].smallnames = true;
	i++;
	credits[i].header = M_GetText("Texture Artists\n");
	j = 0;
	credits[i].fakenames[j++] = "KinkaJoy\n";
	credits[i].fakenames[j++] = "SSNTails\n";
	credits[i].fakenames[j++] = "Blaze Hedgehog\n";
	credits[i].fakenames[j++] = "JEV3\n";
	credits[i].fakenames[j++] = "Nev3r\n";
	j = 0;
	credits[i].realnames[j++] = "Buddy Fischer\n";
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].realnames[j++] = "Ryan Bloom\n";
	credits[i].realnames[j++] = "Jarrett Voight\n";
	credits[i].realnames[j++] = "Pedro Iceta\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Music Production\n");
	j = 0;
	credits[i].fakenames[j++] = "Bulmybag\n";
	credits[i].fakenames[j++] = "Arrow\n";
	credits[i].fakenames[j++] = "Stuf\n";
	credits[i].fakenames[j++] = "SSNTails\n";
	credits[i].fakenames[j++] = "Cyan Helkaraxe\n";
	credits[i].fakenames[j++] = "Red XVI\n";
	credits[i].fakenames[j++] = "Spazzo\n";
	credits[i].fakenames[j++] = "Nev3r\n";
	j = 0;
	credits[i].realnames[j++] = "David Bulmer\n";
	credits[i].realnames[j++] = "Jarel Jones\n";
	credits[i].realnames[j++] = "Stefan Rimalia\n";
	credits[i].realnames[j++] = "Art Freda\n";
	credits[i].realnames[j++] = "Cyan Helkaraxe\n";
	credits[i].realnames[j++] = "Malcolm Brown\n";
	credits[i].realnames[j++] = "Michael Antonakes\n";
	credits[i].realnames[j++] = "Pedro Iceta\n";
	credits[i].smallnames = true;
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Lead Guitar\n");
	j = 0;
	credits[i].fakenames[j++] = "Big Wave Dave\n";
	credits[i].fakenames[j++] = "Shane Strife\n";
	j = 0;
	credits[i].realnames[j++] = "David Spencer Sr\n";
	credits[i].realnames[j++] = "Shane Sexton\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Sound Effects\n");
	j = 0;
	credits[i].fakenames[j++] = "Sega\n";
	credits[i].fakenames[j++] = "Instant Sonic\n";
	credits[i].fakenames[j++] = "Various Sources\n";
	j = 0;
	credits[i].realnames[j++] = "Sega\n";
	credits[i].realnames[j++] = "David Spencer Jr\n";
	credits[i].realnames[j++] = "Various Sources\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Official Mascot\n");
	j = 0;
	credits[i].fakenames[j++] = "Spazzo\n";
	j = 0;
	credits[i].realnames[j++] = "Michael Antonakes\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("Sky Sanctuary Team\n");
	j = 0;
	credits[i].fakenames[j++] = "Boinciel\n";
	credits[i].fakenames[j++] = "Rob\n";
	credits[i].fakenames[j++] = "Blitzzo\n";
	credits[i].fakenames[j++] = "Jazz\n";
	credits[i].fakenames[j++] = "JEV3\n";
	credits[i].fakenames[j++] = "Inuyasha\n";
	credits[i].fakenames[j++] = "Nev3r\n";
	credits[i].fakenames[j++] = "Senku\n";
	credits[i].fakenames[j++] = "Shadow Hog\n";
	credits[i].fakenames[j++] = "Sonict\n";
	credits[i].fakenames[j++] = "Spazzo\n";
	credits[i].fakenames[j++] = "SRB2 Playah\n";
	credits[i].fakenames[j++] = "ST218\n";
	credits[i].fakenames[j++] = "Tets\n";
	credits[i].fakenames[j++] = "Torgo\n";
	credits[i].fakenames[j++] = "Blade\n";
	credits[i].fakenames[j++] = "KO.T.E\n";
	credits[i].fakenames[j++] = "Chaos Zero 64\n";
	credits[i].fakenames[j++] = "Prime 2.0\n";
	j = 0;
	credits[i].realnames[j++] = "Paul Clempson\n";		// Boinciel
	credits[i].realnames[j++] = "Rob Tisdell\n";		// Rob
	credits[i].realnames[j++] = "Dan Hagerstrand\n";	// Blitzzo
	credits[i].realnames[j++] = "Nathan Giroux\n";		// Jazz
	credits[i].realnames[j++] = "Jarrett Voight\n";		// JEV3
	credits[i].realnames[j++] = "Matthew Walsh\n";		// Inuyasha
	credits[i].realnames[j++] = "Pedro Iceta\n";		// Nev3r
	credits[i].realnames[j++] = "Andrew Moran\n";		// Senku
	credits[i].realnames[j++] = "Thomas Igoe\n";		// Shadow Hog
	credits[i].realnames[j++] = "Colin Pfaff\n";		// Sonict
	credits[i].realnames[j++] = "Michael Antonakes\n"; 	// Spazzo
	credits[i].realnames[j++] = "Cody Koester\n";		// SRB2-Playah
	credits[i].realnames[j++] = "Nick Molina\n";		// ST218
	credits[i].realnames[j++] = "Bill Reed\n";		// Tets
	credits[i].realnames[j++] = "Erik Nielsen\n";		// Torgo
	credits[i].realnames[j++] = "Desmond D.\n";      	// Blade
	credits[i].realnames[j++] = "Sherman D.\n";      	// KO.T.E.
	credits[i].realnames[j++] = "Julio Guir\n";      	// Chaos Zero 64
	credits[i].realnames[j++] = "Samuel Peters\n";   	// Prime 2
	credits[i].numnames = (UINT8)j;
	credits[i].smallnames = true;
	i++;
	credits[i].header = M_GetText("Special Thanks\n");
	j = 0;
	credits[i].fakenames[j++] = "Doom Legacy Project\n";
	credits[i].fakenames[j++] = "iD Software\n";
	credits[i].fakenames[j++] = "Dave Perry\n";
	credits[i].fakenames[j++] = "MistaED\n";
	credits[i].fakenames[j++] = "Chrispy\n";
	j = 0;
	credits[i].realnames[j++] = "Doom Legacy Project\n";
	credits[i].realnames[j++] = "iD Software\n";
	credits[i].realnames[j++] = "Dave Perry\n";
	credits[i].realnames[j++] = "Alex Fuller\n";
	credits[i].realnames[j++] = "Chris Pryor\n";
	credits[i].numnames = (UINT8)j;
	i++;
	credits[i].header = M_GetText("In Fond Memory Of\n");
	j = 0;
	credits[i].fakenames[j++] = "Naoto Oshima\n";
	credits[i].fakenames[j++] = "Howard Drossin\n";
	credits[i].fakenames[j++] = "\n";
	credits[i].fakenames[j++] = "\n";
	j = 0;
	credits[i].realnames[j++] = "Naoto Oshima\n";
	credits[i].realnames[j++] = "Howard Drossin\n";
	credits[i].realnames[j++] = "\n";
	credits[i].realnames[j++] = "\n";
	credits[i].numnames = (UINT8)j;
	i++;
	if (modcredits)
	{
		credits[i].header = M_GetText("Modification By\n");
		j = 0;
		credits[i].fakenames[j++] = modcreditname;
		credits[i].fakenames[j++] = "\n";
		credits[i].fakenames[j++] = "\n";
		j = 0;
		credits[i].realnames[j++] = modcreditname;
		credits[i].realnames[j++] = "\n";
		credits[i].realnames[j++] = "\n";
		credits[i].numnames = (UINT8)j;
	}
}

static INT32 scenenum, cutnum;
static INT32 picxpos, picypos, picnum, pictime;
static INT32 textxpos, textypos;

void F_StartCustomCutscene(INT32 cutscenenum, boolean precutscene, boolean resetplayer)
{
	if (!cutscenes[cutscenenum])
		return;

	G_SetGamestate(GS_CUTSCENE);

	gameaction = ga_nothing;
	playerdeadview = false;
	paused = false;
	CON_ToggleOff();
	finaletext = cutscenes[cutscenenum]->scene[0].text;

	CON_ClearHUD();

	runningprecutscene = precutscene;

	if (runningprecutscene)
		precutresetplayer = resetplayer;

	scenenum = picnum = 0;
	cutnum = cutscenenum;
	picxpos = cutscenes[cutnum]->scene[0].xcoord[0];
	picypos = cutscenes[cutnum]->scene[0].ycoord[0];
	textxpos = cutscenes[cutnum]->scene[0].textxpos;
	textypos = cutscenes[cutnum]->scene[0].textypos;

	pictime = cutscenes[cutnum]->scene[0].picduration[0];

	keypressed = false;
	finalestage = 0;
	finalecount = 0;
	finaletextcount = 0;
	timetonext = 0;
	animtimer = cutscenes[cutnum]->scene[0].picduration[0]; // Picture duration
	stoptimer = 0;

	if (cutscenes[cutnum]->scene[scenenum].musicslot != 0)
		S_ChangeMusic(cutscenes[cutnum]->scene[scenenum].musicslot, cutscenes[cutnum]->scene[scenenum].musicloop);
}

static void F_IntroTextWrite(void);
// Introduction
void F_StartIntro(void)
{
	if (introtoplay)
	{
		F_StartCustomCutscene(introtoplay - 1, false, false);
		return;
	}

	intro01text = " #";

	intro02text = M_GetText("Two months had passed since Dr. Eggman\n"
	"tried to take over the world using his\n"
	"Ring Satellite.\n#");

	intro03text = M_GetText("As it was about to drain the rings\n"
	"away from the planet, Sonic burst into\n"
	"the Satellite and for what he thought\n"
	"would be the last time, defeated\n"
	"Dr. Eggman.\n#");

	intro04text = M_GetText("\nWhat Sonic, Tails, and Knuckles had\n"
	"not anticipated was that Eggman would\n"
	"return, bringing an all new threat.\n#");

	intro05text = M_GetText("About once every year, a strange asteroid\n"
	"hovers around the planet. it suddenly\n"
	"appears from nowhere, circles around, and\n"
	"- just as mysteriously as it arrives, it\n"
	"vanishes after about two months.\n"
	"No one knows why it appears, or how.\n#");

	intro06text = M_GetText("\"Curses!\" Eggman yelled. \"That hedgehog\n"
	"and his ridiculous friends will pay\n"
	"dearly for this!\" Just then his scanner\n"
	"blipped as the Black Rock made its\n"
	"appearance from nowhere. Eggman looked at\n"
	"the screen, and just shrugged it off.\n#");

	intro07text = M_GetText("It was only later\n"
	"that he had an\n"
	"idea. \"The Black\n"
	"Rock usually has a\n"
	"lot of energy\n"
	"within it... if I\n"
	"can somehow\n"
	"harness this, I\n"
	"can turn it into\n"
	"the ultimate\n"
	"battle station,\n"
	"and every last\n"
	"person will be\n"
	"begging for mercy,\n"
	"including Sonic!\"\n#");

	intro08text = M_GetText("\n\nBefore beginning his scheme,\n"
	"Eggman decided to give Sonic\n"
	"a reunion party...\n#");

	intro09text = M_GetText("\"We're ready to fire in 15 seconds!\"\n"
	"The robot said, his voice crackling a\n"
	"little down the com-link. \"Good!\"\n"
	"Eggman sat back in his Egg-Mobile and\n"
	"began to count down as he saw the\n"
	"GreenFlower city on the main monitor.\n#");

	intro10text = M_GetText("\"10...9...8...\"\n"
	"Meanwhile, Sonic was tearing across the\n"
	"zones, and everything became nothing but\n"
	"a blur as he ran around loops, skimmed\n"
	"over water, and catapulted himself off\n"
	"rocks with his phenomenal speed.\n#");

	intro11text = M_GetText("\"5...4...3...\"\n"
	"Sonic knew he was getting closer to the\n"
	"City, and pushed himself harder. Finally,\n"
	"the city appeared in the horizon.\n"
	"\"2...1...Zero.\"\n#");

	intro12text = M_GetText("GreenFlower City was gone.\n"
	"Sonic arrived just in time to see what\n"
	"little of the 'ruins' were left. Everyone\n"
	"and everything in the city had been\n"
	"obliterated.\n#");

	intro13text = M_GetText("\"You're not quite as dead as we thought,\n"
	"huh? Are you going to tell us your plan as\n"
	"usual or will I 'have to work it out' or\n"
	"something?\"                         \n"
	"\"We'll see... let's give you a quick warm\n"
	"up, Sonic! JETTYSYNS! Open fire!\"\n#");

	intro14text = M_GetText("Eggman took this\n"
	"as his cue and\n"
	"blasted off,\n"
	"leaving Sonic\n"
	"and Tails behind.\n"
	"Tails looked at\n"
	"the ruins of the\n"
	"Greenflower City\n"
	"with a grim face\n"
	"and sighed.           \n"
	"\"Now what do we\n"
	"do?\", he asked.\n#");

	intro15text = M_GetText("\"Easy! We go\n"
	"find Eggman\n"
	"and stop his\n"
	"latest\n"
	"insane plan.\n"
	"Just like\n"
	"we've always\n"
	"done, right?                 \n\n"
	"...                    \n\n"
	"\"Tails,what\n"
	"*ARE* you\n"
	"doing?\"\n#");

	intro16text = M_GetText("\"I'm just finding what mission obje...\n"
	"a-ha! Here it is! This will only give\n"
	"the robot's primary objective. It says,\n"
	"* LOCATE AND RETRIEVE CHAOS EMERALD.\n"
	"ESTIMATED LOCATION: GREENFLOWER ZONE *\"\n"
	"\"All right, then let's go!\"\n#");

/*
	"What are we waiting for? The first emerald is ours!" Sonic was about to
	run, when he saw a shadow pass over him, he recognized the silhouette
	instantly.
	"Knuckles!" Sonic said. The echidna stopped his glide and landed
	facing Sonic. "What are you doing here?"
	He replied, "This crisis affects the Floating Island,
	if that explosion I saw is anything to go by."
	If you're willing to help then... let's go!"
*/

	G_SetGamestate(GS_INTRO);
	gameaction = ga_nothing;
	playerdeadview = false;
	paused = false;
	CON_ToggleOff();
	CON_ClearHUD();
	finaletext = intro01text;

	finalestage = finaletextcount = finalecount = animtimer = stoptimer = 0;
	roidtics = BASEVIDWIDTH - 64;
	scene = 0;
	timetonext = introscenetime[scene];
}

// Intro
boolean F_IntroResponder(event_t *event)
{
	INT32 key = event->data1;

	// remap virtual keys (mouse & joystick buttons)
	switch (key)
	{
		case KEY_MOUSE1:
			key = KEY_ENTER;
			break;
		case KEY_MOUSE1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_JOY1:
		case KEY_JOY1 + 2:
			key = KEY_ENTER;
			break;
		case KEY_JOY1 + 3:
			key = 'n';
			break;
		case KEY_JOY1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_HAT1:
			key = KEY_UPARROW;
			break;
		case KEY_HAT1 + 1:
			key = KEY_DOWNARROW;
			break;
		case KEY_HAT1 + 2:
			key = KEY_LEFTARROW;
			break;
		case KEY_HAT1 + 3:
			key = KEY_RIGHTARROW;
			break;
	}

	if (event->type != ev_keydown && key != 301)
		return false;

	if (key != 27 && key != KEY_ENTER && key != KEY_SPACE && key != KEY_BACKSPACE)
		return false;

	if (keypressed)
		return false;

	keypressed = true;
	return true;
}

boolean F_CutsceneResponder(event_t *event)
{
	if (cutnum == introtoplay-1)
		return F_IntroResponder(event);

	return false;
}

boolean F_CreditResponder(event_t *event)
{
	INT32 key = event->data1;

	// remap virtual keys (mouse & joystick buttons)
	switch (key)
	{
		case KEY_MOUSE1:
			key = KEY_ENTER;
			break;
		case KEY_MOUSE1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_JOY1:
		case KEY_JOY1 + 2:
			key = KEY_ENTER;
			break;
		case KEY_JOY1 + 3:
			key = 'n';
			break;
		case KEY_JOY1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_HAT1:
			key = KEY_UPARROW;
			break;
		case KEY_HAT1 + 1:
			key = KEY_DOWNARROW;
			break;
		case KEY_HAT1 + 2:
			key = KEY_LEFTARROW;
			break;
		case KEY_HAT1 + 3:
			key = KEY_RIGHTARROW;
			break;
	}

	if (!(grade & 1))
		return false;

	if (event->type != ev_keydown)
		return false;

	if (key != 27 && key != KEY_ENTER && key != KEY_SPACE && key != KEY_BACKSPACE)
		return false;

	if (keypressed)
		return true;

	keypressed = true;
	return true;
}

// De-Demo'd Title Screen
void F_TitleScreenTicker(void)
{
	finalecount++;
	finalestage += 8;
}

// Game end thingy
//
// F_GameEndTicker
//
void F_GameEndTicker(void)
{
	if (timetonext > 0)
		timetonext--;
	else
		D_StartTitle();
}

void F_GameEvaluationTicker(void)
{
	finalecount++;

	if (finalecount > 10*TICRATE)
		F_StartGameEnd();
}

void F_CreditTicker(void)
{
	finalecount++;

	if (finalecount > 90*TICRATE)
		F_StartGameEvaluation();
}

//
// F_IntroTicker
//
void F_IntroTicker(void)
{
	// advance animation
	finalecount++;
	finaletextcount++;

	if (finalecount % 3 == 0)
		roidtics--;

	timetonext--;

	// check for skipping
	if (keypressed)
		keypressed = false;
}

void F_CutsceneTicker(void)
{
	INT32 i;

	// advance animation
	finalecount++;
	finaletextcount++;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (netgame && i != serverplayer && i != adminplayer)
			continue;

		if (players[i].cmd.buttons & BT_USE)
		{
			keypressed = false;
			finaletextcount += 64;
			if (timetonext)
				timetonext = 2;
		}
	}
}

//
// F_WriteText
//
static void F_WriteText(INT32 cx, INT32 cy)
{
	INT32 count, c, w, originalx = cx;
	const char *ch = finaletext; // draw some of the text onto the screen

	count = (finaletextcount - 10)/2;

	if (count < 0)
		count = 0;

	if (timetonext == 1 || !ch)
	{
		finaletextcount = 0;
		timetonext = 0;
		roidtics = BASEVIDWIDTH - 64;
		return;
	}

	for (; count; count--)
	{
		c = *ch++;
		if (!c)
			break;

		if (c == '#')
		{
			if (!timetonext)
			{
				if (finaletext == intro16text)
					timetonext = 12*TICRATE + 1;
				else
					timetonext = 5*TICRATE + 1;
			}
			break;
		}
		if (c == '\n')
		{
			cx = originalx;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART)) /// \note font end hack
		{
			cx += 4;
			continue;
		}

		w = SHORT(hu_font[c]->width);
		if (cx + w > BASEVIDWIDTH)
			break;
		V_DrawScaledPatch(cx, cy, 0, hu_font[c]);
		cx += w;
	}
}

static void F_WriteCutsceneText(void)
{
	INT32 count, c, w, originalx = textxpos, cx = textxpos, cy = textypos;
	const char *ch = finaletext; // draw some of the text onto the screen

	count = (finaletextcount - 10)/2;

	if (count < 0)
		count = 0;

	if (timetonext == 1 || !ch)
	{
		finaletextcount = 0;
		timetonext = 0;
		roidtics = BASEVIDWIDTH - 64;
		return;
	}

	for (; count; count--)
	{
		c = *ch++;
		if (!c)
			break;

		if (c == '#')
		{
			if (!timetonext)
				timetonext = 5*TICRATE + 1;
			break;
		}
		if (c == '\n')
		{
			cx = originalx;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || (c >= HU_REALFONTSIZE && c != '~' - HU_FONTSTART && c != '`' - HU_FONTSTART))
		{ /// \note font end hack
			cx += 4;
			continue;
		}

		w = SHORT(hu_font[c]->width);
		if (cx + w > vid.width)
			break;
		V_DrawScaledPatch(cx, cy, 0, hu_font[c]);
		cx += w;
	}
}

//
// F_IntroTextWrite
//
static void F_IntroTextWrite(void)
{
	boolean nobg = false, highres = false;
	INT32 cx = 8, cy = 128;
	patch_t *background = NULL;
	void *patch;

	// DRAW A FULL PIC INSTEAD OF FLAT!
	if (finaletext == intro01text)
		nobg = true;
	else if (finaletext == intro02text)
		background = W_CachePatchName("INTRO1", PU_CACHE);
	else if (finaletext == intro03text)
	{
		background = W_CachePatchName("INTRO2", PU_CACHE);
		highres = true;
	}
	else if (finaletext == intro04text)
		background = W_CachePatchName("INTRO3", PU_CACHE);
	else if (finaletext == intro05text)
		background = W_CachePatchName("INTRO4", PU_CACHE);
	else if (finaletext == intro06text)
	{
		background = W_CachePatchName("DRAT", PU_CACHE);
		highres = true;
	}
	else if (finaletext == intro07text)
	{
		background = W_CachePatchName("INTRO6", PU_CACHE);
		cx = 180;
		cy = 8;
	}
	else if (finaletext == intro08text)
		background = W_CachePatchName("SGRASS1", PU_CACHE);
	else if (finaletext == intro09text)
	{
		background = W_CachePatchName("WATCHING", PU_CACHE);
		highres = true;
	}
	else if (finaletext == intro10text)
	{
		background = W_CachePatchName("ZOOMING", PU_CACHE);
		highres = true;
	}
	else if (finaletext == intro11text)
		nobg = true;
	else if (finaletext == intro12text)
		background = W_CachePatchName("INTRO5", PU_CACHE);
	else if (finaletext == intro13text)
	{
		background = W_CachePatchName("REVENGE", PU_CACHE);
		highres = true;
	}
	else if (finaletext == intro14text)
	{
		nobg = true;
		cx = 8;
		cy = 8;
	}
	else if (finaletext == intro15text)
	{
		background = W_CachePatchName("SONICDO1", PU_CACHE);
		highres = true;
		cx = 224;
		cy = 8;
	}
	else if (finaletext == intro16text)
	{
		background = W_CachePatchName("INTRO7", PU_CACHE);
		highres = true;
	}

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (finaletext == intro01text)
	{
		V_DrawCreditString(160 - (V_CreditStringWidth("SONIC TEAM JR")/2), 80, 0, "SONIC TEAM JR");
		V_DrawCreditString(160 - (V_CreditStringWidth("PRESENTS")/2), 96, 0, "PRESENTS");
	}
	else if (finaletext == intro11text)
	{
		if (finaletextcount > 8*TICRATE && finaletextcount < 9*TICRATE)
		{
			if (!(finalecount & 3))
				background = W_CachePatchName("BRITEGG1", PU_CACHE);
			else
				background = W_CachePatchName("DARKEGG1", PU_CACHE);

			V_DrawScaledPatch(0, 0, 0, background);
		}
		else if (finaletextcount > 10*TICRATE && finaletextcount < 11*TICRATE)
		{
			if (!(finalecount & 3))
				background = W_CachePatchName("BRITEGG2", PU_CACHE);
			else
				background = W_CachePatchName("DARKEGG2", PU_CACHE);

			V_DrawScaledPatch(0, 0, 0, background);
		}
		else if (finaletextcount > 12*TICRATE && finaletextcount < 13*TICRATE)
		{
			if (!(finalecount & 3))
				background = W_CachePatchName("BRITEGG3", PU_CACHE);
			else
				background = W_CachePatchName("DARKEGG3", PU_CACHE);

			V_DrawScaledPatch(0, 0, 0, background);
		}
		else
		{
			F_SkyScroll(80*4);
			if (timetonext == 6)
			{
				stoptimer = finalecount;
				animtimer = finalecount % 16;
			}
			else if (timetonext >= 0 && timetonext < 6)
			{
				animtimer = stoptimer;
				deplete -= 32;
			}
			else
			{
				animtimer = finalecount % 16;
				deplete = 160;
			}

			if (finalecount & 1)
			{
				V_DrawScaledPatch(deplete, 8, 0, (patch = W_CachePatchName("RUN2", PU_CACHE)));
				W_UnlockCachedPatch(patch);
				V_DrawScaledPatch(deplete, 72, 0, (patch = W_CachePatchName("PEELOUT2", PU_CACHE)));
				W_UnlockCachedPatch(patch);
			}
			else
			{
				V_DrawScaledPatch(deplete, 8, 0, (patch = W_CachePatchName("RUN1", PU_CACHE)));
				W_UnlockCachedPatch(patch);
				V_DrawScaledPatch(deplete, 72, 0, (patch = W_CachePatchName("PEELOUT1", PU_CACHE)));
				W_UnlockCachedPatch(patch);
			}
			V_DrawFill(0, 112, BASEVIDWIDTH, BASEVIDHEIGHT - 112, 31);
		}
	}
	else if (finaletext == intro08text && timetonext > 0 && finaletextcount >= 5*TICRATE
		&& finaletextcount > 0)
	{
		background = W_CachePatchName("SGRASS5", PU_CACHE);
	}

	if (!nobg)
	{
		if (highres)
			V_DrawSmallScaledPatch(0, 0, 0, background);
		else
			V_DrawScaledPatch(0, 0, 0, background);
	}

	W_UnlockCachedPatch(background);

	if (finaletext == intro14text)
	{
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
		V_DrawSmallScaledPatch(144, 0, 0, (patch = W_CachePatchName("TAILSSAD", PU_CACHE)));
		W_UnlockCachedPatch(patch);
	}
	else if (finaletext == intro05text) // The asteroid SPINS!
	{
		if (roidtics >= 0)
		{
			V_DrawScaledPatch(roidtics, 24, 0,
				(patch = W_CachePatchName(va("ROID00%.2d", finaletextcount%35), PU_CACHE)));
			W_UnlockCachedPatch(patch);
		}
	}
	else if (finaletext == intro06text)
	{
		if (finaletextcount == 5*TICRATE + (TICRATE/5)*3)
		{
			if (rendermode != render_none)
			{
				F_WriteText(cx, cy);

				F_WipeStartScreen();

				V_DrawScaledPatch(0, 0, 0, (patch = W_CachePatchName("RADAR", PU_CACHE)));
				W_UnlockCachedPatch(patch);
				// draw some of the text onto the screen
				F_WriteText(cx, cy);

				F_WipeEndScreen(0, 0, vid.width, vid.height);

				F_RunWipe(TICRATE, true);
			}
		}
		else if (finaletextcount > 5*TICRATE+(TICRATE/5)*3)
		{
			V_DrawScaledPatch(0, 0, 0, (patch = W_CachePatchName("RADAR", PU_CACHE)));
			W_UnlockCachedPatch(patch);
		}
	}
	else if (finaletext == intro13text)
	{
		if (finaletextcount == 9*TICRATE)
		{
			if (rendermode != render_none)
			{
				F_WriteText(cx, cy);

				F_WipeStartScreen();

				V_DrawSmallScaledPatch(0, 0, 0, (patch = W_CachePatchName("CONFRONT", PU_CACHE)));
				W_UnlockCachedPatch(patch);

				// draw some of the text onto the screen
				F_WriteText(cx, cy);

				F_WipeEndScreen(0, 0, vid.width, vid.height);

				F_RunWipe(TICRATE, true);
			}
		}
		else if (finaletextcount > 9*TICRATE)
		{
			V_DrawSmallScaledPatch(0, 0, 0, (patch = W_CachePatchName("CONFRONT", PU_CACHE)));
			W_UnlockCachedPatch(patch);
		}
	}
	else if (finaletext == intro15text)
	{
		if (finaletextcount == 7*TICRATE)
		{
			if (rendermode != render_none)
			{
				F_WriteText(cx, cy);

				F_WipeStartScreen();

				V_DrawSmallScaledPatch(0, 0, 0, (patch = W_CachePatchName("SONICDO2", PU_CACHE)));
				W_UnlockCachedPatch(patch);

				// draw some of the text onto the screen
				F_WriteText(cx, cy);

				F_WipeEndScreen(0, 0, vid.width, vid.height);

				F_RunWipe(TICRATE, true);
			}
		}
		else if (finaletextcount > 7*TICRATE)
		{
			V_DrawSmallScaledPatch(0, 0, 0, (patch = W_CachePatchName("SONICDO2", PU_CACHE)));
			W_UnlockCachedPatch(patch);
		}
	}

	if (animtimer)
		animtimer--;

	if (finaletext == intro08text && finaletextcount > 5*TICRATE)
	{
		first = W_CachePatchName("SGRASS2", PU_CACHE);
		second = W_CachePatchName("SGRASS3", PU_CACHE);
		third = W_CachePatchName("SGRASS4", PU_CACHE);

		if (finaletextcount == 5*TICRATE + 1)
		{
			currentanim = first;
			nextanim = second;
			animtimer = TICRATE/7 + 1;
		}
		else if (animtimer == 1 && currentanim == first)
		{
			currentanim = second;
			nextanim = third;
			animtimer = TICRATE/7 + 1;
		}
		else if (animtimer == 1 && currentanim == second)
			currentanim = third;

		if (currentanim)
			V_DrawScaledPatch(123, 4, 0, currentanim);

		W_UnlockCachedPatch(third);
		W_UnlockCachedPatch(second);
		W_UnlockCachedPatch(first);
	}

	F_WriteText(cx, cy);
}

//
// F_DrawPatchCol
//
static void F_DrawPatchCol(INT32 x, patch_t *patch, INT32 col, INT32 yrepeat)
{
	const column_t *column;
	const UINT8 *source;
	UINT8 *desttop, *dest = NULL;
	const UINT8 *deststop;
	size_t count;

	column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[col]));
	desttop = screens[0] + x*vid.dupx;
	deststop = screens[0] + vid.width * vid.height * vid.bpp;

	// step through the posts in a column
	while (column->topdelta != 0xff)
	{
		source = (const UINT8 *)column + 3;
		dest = desttop + column->topdelta*vid.width;
		count = column->length;

		while (count--)
		{
			INT32 dupycount = vid.dupy;

			while (dupycount--)
			{
				INT32 dupxcount = vid.dupx;
				while (dupxcount-- && dest <= deststop)
					*dest++ = *source;

				dest += (vid.width - vid.dupx);
			}
			source++;
		}
		column = (const column_t *)((const UINT8 *)column + column->length + 4);
	}

	// repeat a second time, for yrepeat number of pixels
	if (yrepeat)
	{
		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[col]));
		while (column->topdelta != 0xff)
		{
			source = (const UINT8 *)column + 3;
			count = column->length;

			while (count--)
			{
				INT32 dupycount = vid.dupy;

				while (dupycount--)
				{
					INT32 dupxcount = vid.dupx;
					while (dupxcount-- && dest <= deststop)
						*dest++ = *source;

					dest += (vid.width - vid.dupx);
				}
				source++;
			}
			if (!--yrepeat)
				break;
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}
	}
}

//
// F_SkyScroll
//
static void F_SkyScroll(INT32 scrollspeed)
{
	INT32 scrolled, x, mx, fakedwidth;
	patch_t *pat;

	pat = W_CachePatchName("TITLESKY", PU_CACHE);

	animtimer = ((finalecount*scrollspeed)/16) % SHORT(pat->width);

	fakedwidth = vid.width / vid.dupx;

	if (rendermode == render_soft)
	{
		INT32 yr = 0;

		if (vid.fdupy > vid.dupy*FRACUNIT)
			yr = vid.height - vid.dupy*SHORT(pat->height);

		scrolled = BASEVIDWIDTH - animtimer;
		if (scrolled > BASEVIDWIDTH)
			scrolled = BASEVIDWIDTH;
		if (scrolled < 0)
			scrolled = 0;
		for (x = 0, mx = 0; x < fakedwidth; x++, mx++)
		{
			if (mx >= SHORT(pat->width))
				mx = 0;

			if (mx + scrolled < SHORT(pat->width))
				F_DrawPatchCol(x, pat, mx + scrolled, yr);
			else
				F_DrawPatchCol(x, pat, mx + scrolled - SHORT(pat->width), yr);
		}
	}
#ifdef HWRENDER
	else if (rendermode != render_none)
	{ // I wish it were as easy as this for software. I really do.
		scrolled = animtimer;
		if (scrolled > 0)
			V_DrawScaledPatch(scrolled - SHORT(pat->width), 0, 0, pat);
		while(scrolled < BASEVIDWIDTH)
		{
			V_DrawScaledPatch(scrolled, 0, 0, pat);
			scrolled += SHORT(pat->width);
		}
	}
#endif

	W_UnlockCachedPatch(pat);
}

// De-Demo'd Title Screen
void F_TitleScreenDrawer(void)
{
	// Draw that sky!
	F_SkyScroll(titlescrollspeed);

	V_DrawScaledPatch(30, 14, 0, ttwing);

	if (finalecount < 57)
	{
		if (finalecount == 35)
			V_DrawScaledPatch(115, 15, 0, ttspop1);
		else if (finalecount == 36)
			V_DrawScaledPatch(114, 15, 0,ttspop2);
		else if (finalecount == 37)
			V_DrawScaledPatch(113, 15, 0,ttspop3);
		else if (finalecount == 38)
			V_DrawScaledPatch(112, 15, 0,ttspop4);
		else if (finalecount == 39)
			V_DrawScaledPatch(111, 15, 0,ttspop5);
		else if (finalecount == 40)
			V_DrawScaledPatch(110, 15, 0, ttspop6);
		else if (finalecount >= 41 && finalecount <= 44)
			V_DrawScaledPatch(109, 15, 0, ttspop7);
		else if (finalecount >= 45 && finalecount <= 48)
			V_DrawScaledPatch(108, 12, 0, ttsprep1);
		else if (finalecount >= 49 && finalecount <= 52)
			V_DrawScaledPatch(107, 9, 0, ttsprep2);
		else if (finalecount >= 53 && finalecount <= 56)
			V_DrawScaledPatch(106, 6, 0, ttswip1);
		V_DrawScaledPatch(93, 106, 0, ttsonic);
	}
	else
	{
		V_DrawScaledPatch(93, 106, 0,ttsonic);
		if (finalecount/5 & 1)
			V_DrawScaledPatch(100, 3, 0,ttswave1);
		else
			V_DrawScaledPatch(100,3, 0,ttswave2);
	}

	V_DrawScaledPatch(48, 142, 0,ttbanner);
}

// Game End Sequence
//
// F_GameEndDrawer
//
void F_GameEndDrawer(void)
{
}

#define INTERVAL 50
#define TRANSLEVEL V_8020TRANS

void F_GameEvaluationDrawer(void)
{
	INT32 x, y;
	const fixed_t radius = 48*FRACUNIT;
	angle_t fa;

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	// Draw all the good crap here.
	if (animtimer == 64)
		V_DrawString(114, 16, 0, "GOT THEM ALL!");
	else
		V_DrawString(124, 16, 0, "TRY AGAIN!");

	finalestage++;
	timetonext = finalestage;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD1)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGA0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGA0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD2)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGB0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGB0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD3)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGC0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGC0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD4)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGD0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGD0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD5)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGE0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGE0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD6)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGF0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGF0", PU_CACHE));

	timetonext += INTERVAL;

	fa = (FixedAngle(timetonext*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
	x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
	y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

	if (emeralds & EMERALD7)
		V_DrawScaledPatch(x, y, 0, W_CachePatchName("CEMGG0", PU_CACHE));
	else
		V_DrawTranslucentPatch(x, y, TRANSLEVEL|V_TOPLEFT, W_CachePatchName("CEMGG0", PU_CACHE));

	if (finalestage >= 360)
		finalestage -= 360;

	if (finalecount == 5*TICRATE)
	{
		if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && mapheaderinfo[gamemap-1]->nextlevel == 1102)
		{
			boolean alreadyplayed = false;
			INT32 bitcount = 0;
			INT32 i;

			if (!(emblemlocations[MAXEMBLEMS-2].collected))
			{
				emblemlocations[MAXEMBLEMS-2].collected = true; // For completing the game.
				S_StartSound(NULL, sfx_ncitem);
				alreadyplayed = true;
				drawemblem = true;
			}

			if (ALL7EMERALDS(emeralds))
			{
				if (!(emblemlocations[MAXEMBLEMS-1].collected))
				{
					emblemlocations[MAXEMBLEMS-1].collected = true;

					if (!alreadyplayed)
						S_StartSound(NULL, sfx_ncitem);

					drawchaosemblem = true;
				}
				grade |= 2;
			}

			for (i = 0; i < MAXEMBLEMS; i++)
			{
				if (emblemlocations[i].collected)
					bitcount++;
			}


			if (bitcount >= numemblems/2) // Got half of the emblems
				grade |= 4;

			if (bitcount >= numemblems/4) // NiGHTS
				grade |= 16;

			if (bitcount == numemblems) // Got ALL emblems!
				grade |= 8;

			grade |= 1; // Just for completing the game.

			timesbeaten++;

			// An award for beating ultimate mode, perhaps?
			if (ultimatemode)
				grade |= 256;

			// Other unlockables
			if (gamemap == 29) // Cleared NiGHTS
				grade |= 64;
			else if (gamemap == 32) // Cleared Mario
				grade |= 32;
			else if (gamemap == 132) // Cleared SRB1
				grade |= 1024;

			if (cursaveslot != -1)
				G_SaveGame((UINT32)cursaveslot);
		}
	}

	G_SaveGameData();

	if (finalecount >= 5*TICRATE)
	{
		if (drawemblem)
			V_DrawScaledPatch(120, 192, 0, W_CachePatchName("NWNGA0", PU_CACHE));

		if (drawchaosemblem)
			V_DrawScaledPatch(200, 192, 0, W_CachePatchName("NWNGA0", PU_CACHE));

		V_DrawString(8, 16, V_YELLOWMAP, "Unlocked:");

		if (savemoddata)
		{
			INT32 i;
			boolean unlocked;
			INT32 startcoord = 32;

			for (i = 0; i < 15; i++)
			{
				unlocked = false;

				if (customsecretinfo[i].neededemblems)
				{
					unlocked = M_GotEnoughEmblems(customsecretinfo[i].neededemblems);

					if (unlocked && customsecretinfo[i].neededtime)
						unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);
				}
				else if (customsecretinfo[i].neededtime)
					unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);
				else
					unlocked = (grade & customsecretinfo[i].neededgrade);

				if (unlocked)
				{
					V_DrawString(8, startcoord, 0, customsecretinfo[i].name);

					startcoord += 8;
				}
			}
		}
		else
		{
			if (grade & 1)
				V_DrawString(8, 32, 0, "Level Select");

			if (grade & 2)
				V_DrawString(8, 40, 0, "SRB1 Remake");

			if (grade & 4)
				V_DrawString(8, 48, 0, "Mario");

			if (grade & 8)
				V_DrawString(8, 56, 0, "Pandora's Box");

			if (grade & 16)
				V_DrawString(8, 64, 0, "NiGHTS");

			if (netgame)
				V_DrawString(8, 96, V_YELLOWMAP, "Prizes only\nawarded in\nsingle player!");
			else if (modifiedgame)
				V_DrawString(8, 96, V_YELLOWMAP, "Prizes not\nawarded in\nmodified games!");
		}
	}
}

static void F_DrawCreditScreen(credit_t *creditpassed)
{
	INT32 i, height, y;

	if (creditpassed->smallnames)
		height = BASEVIDHEIGHT/((creditpassed->numnames/2)+1);
	else
		height = BASEVIDHEIGHT/(creditpassed->numnames+1);

	switch (animtimer)
	{
		case 1:
		case 2:
			if (ultimatemode)
				V_DrawSmallScaledPatch(204, 118, 0, W_CachePatchName("ACRED02", PU_CACHE));
			else
				V_DrawSmallScaledPatch(8, 112, 0, W_CachePatchName("CREDIT01", PU_CACHE));
			break;
		case 3:
		case 4:
			if (ultimatemode)
			{
				V_DrawSmallScaledPatch(234, 118, 0, W_CachePatchName("ACRED01", PU_CACHE));
				V_DrawSmallScaledPatch(4, 4, 0, W_CachePatchName("ACRED03", PU_CACHE));
			}
			else
			{
				V_DrawSmallScaledPatch(4, 4, 0, W_CachePatchName("CREDIT13", PU_CACHE));
				V_DrawSmallScaledPatch(250, 100, 0, W_CachePatchName("CREDIT12", PU_CACHE));
			}
			break;
		case 5:
			if (ultimatemode)
				V_DrawSmallScaledPatch(4, 72, 0, W_CachePatchName("ACRED04", PU_CACHE));
			else
				V_DrawSmallScaledPatch(8, 0, 0, W_CachePatchName("CREDIT03", PU_CACHE));
			break;
		case 6:
			if (ultimatemode)
				V_DrawSmallScaledPatch(55, 0, 0, W_CachePatchName("ACRED05", PU_CACHE));
			else
				V_DrawSmallScaledPatch(248, 110, 0, W_CachePatchName("CREDIT11", PU_CACHE));
			break;
		case 7:
		case 8:
			if (ultimatemode)
				V_DrawSmallScaledPatch(8, 108, 0, W_CachePatchName("ACRED06", PU_CACHE));
			else
				V_DrawSmallScaledPatch(8, 112, 0, W_CachePatchName("CREDIT04", PU_CACHE));
			break;
		case 9:
			if (ultimatemode)
				V_DrawSmallScaledPatch((BASEVIDWIDTH/2) - 46, 102, 0, W_CachePatchName("ACRED07", PU_CACHE));
			else
				V_DrawSmallScaledPatch((BASEVIDWIDTH/2) - 48, 108, 0, W_CachePatchName("CREDIT10", PU_CACHE));
			break;
		case 10:
			if (ultimatemode)
				V_DrawSmallScaledPatch(202, 128, 0, W_CachePatchName("ACRED09", PU_CACHE));
			else
				V_DrawSmallScaledPatch(240, 8, 0, W_CachePatchName("CREDIT05", PU_CACHE));
			break;
		case 11:
		case 12:
			if (ultimatemode)
				V_DrawSmallScaledPatch((BASEVIDWIDTH/2) - 46, 102, 0, W_CachePatchName("ACRED11", PU_CACHE));
			else
				V_DrawSmallScaledPatch(120, 120, 0, W_CachePatchName("CREDIT06", PU_CACHE));
			break;
		case 13:
		case 14:
			if (ultimatemode)
			{
				V_DrawSmallScaledPatch(174, 84, 0, W_CachePatchName("ACRED08", PU_CACHE));
				V_DrawSmallScaledPatch(2, 64, 0, W_CachePatchName("ACRED10", PU_CACHE));
			}
			else
				V_DrawSmallScaledPatch(8, 100, 0, W_CachePatchName("CREDIT07", PU_CACHE));
			break;
		case 15:
		case 16:
			if (ultimatemode)
			{
				V_DrawSmallScaledPatch(174, 84, 0, W_CachePatchName("ACRED08", PU_CACHE));
				V_DrawSmallScaledPatch(2, 52, 0, W_CachePatchName("ACRED10", PU_CACHE));
			}
			else
				V_DrawSmallScaledPatch(8, 0, 0, W_CachePatchName("CREDIT08", PU_CACHE));
			break;
		case 17:
		case 18:
			V_DrawSmallScaledPatch(112, 104, 0, W_CachePatchName("CREDIT09", PU_CACHE));
			break;
	}

	if (creditpassed->numnames == 1) // Shift it up a bit
		height -= 16;

	V_DrawCreditString((BASEVIDWIDTH - V_CreditStringWidth(creditpassed->header))/2, height, 0,
		creditpassed->header);

	if (creditpassed->smallnames)
		height += 32;

	y = 0;

	for (i = 0; i < creditpassed->numnames; i++)
	{
		if (creditpassed->smallnames)
		{
			INT32 x;

			if (cv_realnames.value)
			{
				if (i < creditpassed->numnames/2)
					x = (BASEVIDWIDTH/4) - (V_StringWidth(creditpassed->realnames[i])/2);
				else
					x = ((BASEVIDWIDTH/4)*3) - (V_StringWidth(creditpassed->realnames[i])/2);

				V_DrawString(x,
					height + y - 8, 0, creditpassed->realnames[i]); //shifted up to prevent cutoff. -Spazzo
			}
			else
			{
				if (i < creditpassed->numnames/2)
					x = (BASEVIDWIDTH/4) - (V_StringWidth(creditpassed->fakenames[i])/2);
				else
					x = ((BASEVIDWIDTH/4)*3) - (V_StringWidth(creditpassed->fakenames[i])/2);

				V_DrawString(x,
					height + y - 8, 0, creditpassed->fakenames[i]);
			}
			y += 16;

			if (i + 1 == creditpassed->numnames/2)
				y = 0;
		}
		else
		{
			if (cv_realnames.value)
				V_DrawCreditString((BASEVIDWIDTH - V_CreditStringWidth(creditpassed->realnames[i]))/2,
					height + (1+i)*24, 0, creditpassed->realnames[i]);
			else
				V_DrawCreditString((BASEVIDWIDTH - V_CreditStringWidth(creditpassed->fakenames[i]))/2,
					height + (1+i)*24, 0, creditpassed->fakenames[i]);
		}
	}
}

void F_CreditDrawer(void)
{
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (timetonext-- <= 0) // Fade to the next!
	{
		if (modcredits)
			timetonext = 165*NEWTICRATERATIO;
		else
			timetonext = 5*TICRATE-1;

		F_DrawCreditScreen(&credits[animtimer]);

		animtimer++;

		if (rendermode != render_none)
		{
			F_WipeStartScreen();
			V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
			F_DrawCreditScreen(&credits[animtimer]);
			F_WipeEndScreen(0, 0, vid.width, vid.height);

			F_RunWipe(TICRATE, true);
		}
	}
	else
		F_DrawCreditScreen(&credits[animtimer]);
}

//
// F_IntroDrawer
//
void F_IntroDrawer(void)
{
	if (timetonext <= 0)
	{
		if (finaletext == intro01text)
		{
			S_ChangeMusic(mus_read_m, false);
			finaletext = intro02text;
		}
		else if (finaletext == intro02text)
			finaletext = intro03text;
		else if (finaletext == intro03text)
			finaletext = intro04text;
		else if (finaletext == intro04text)
		{
			finaletext = intro05text;
			roidtics = BASEVIDWIDTH - 64;
		}
		else if (finaletext == intro05text)
			finaletext = intro06text;
		else if (finaletext == intro06text)
			finaletext = intro07text;
		else if (finaletext == intro07text)
			finaletext = intro08text;
		else if (finaletext == intro08text)
			finaletext = intro09text;
		else if (finaletext == intro09text)
			finaletext = intro10text;
		else if (finaletext == intro10text)
			finaletext = intro11text;
		else if (finaletext == intro11text)
		{
			if (rendermode != render_none)
			{
				F_WipeStartScreen();
				V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 0);
				F_WipeEndScreen(0, 0, vid.width, vid.height);

				F_RunWipe(TICRATE, true);
			}

			finaletext = intro12text;
		}
		else if (finaletext == intro12text)
			finaletext = intro13text;
		else if (finaletext == intro13text)
			finaletext = intro14text;
		else if (finaletext == intro14text)
			finaletext = intro15text;
		else if (finaletext == intro15text)
			finaletext = intro16text;
		else if (finaletext == intro16text)
		{
			if (rendermode != render_none)
			{
				F_WipeStartScreen();
				V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
				F_WipeEndScreen(0, 0, vid.width, vid.height);

				F_RunWipe(TICRATE, true);
			}

			// Stay on black for a bit. =)
			{
				tic_t quittime;
				quittime = I_GetTime() + TICRATE*2; // Shortened the quit time, used to be 2 seconds
				while (quittime > I_GetTime())
				{
					I_OsPolling();
					I_UpdateNoBlit();
					M_Drawer(); // menu is drawn even on top of wipes
					I_FinishUpdate(); // Update the screen with the image Tails 06-19-2001
				}
			}

			D_StartTitle();
			return;
		}

		if (gamestate == GS_INTRO)
			G_SetGamestate(GS_INTRO2);
		else
			G_SetGamestate(GS_INTRO);

#ifndef SHUFFLE
		if (rendermode == render_soft)
#endif
			F_WipeStartScreen();

		wipegamestate = -1;
		finaletextcount = 0;
		animtimer = 0;
		stoptimer = 0;
		scene++;
		timetonext = introscenetime[scene];
	}

	if (finaletext == intro08text && finaletextcount == 5*TICRATE) // Force a wipe here
	{
		if (rendermode != render_none)
		{
			patch_t *grass = W_CachePatchName("SGRASS5", PU_CACHE);

			F_WipeStartScreen();
			V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
			V_DrawScaledPatch(0, 0, 0, grass);
			W_UnlockCachedPatch(grass);
			F_IntroTextWrite();
			F_WipeEndScreen(0, 0, vid.width, vid.height);

			F_RunWipe(TICRATE, true);
		}
	}

	F_IntroTextWrite();
}

static void F_AdvanceToNextScene(void)
{
	scenenum++;

	if (scenenum < cutscenes[cutnum]->numscenes)
	{
		picnum = 0;
		picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
		picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
	}

	if (cutscenes[cutnum]->scene[scenenum].musicslot != 0)
		S_ChangeMusic(cutscenes[cutnum]->scene[scenenum].musicslot, cutscenes[cutnum]->scene[scenenum].musicloop);

	if (rendermode != render_none)
	{
		F_WipeStartScreen();
		V_DrawFill(0,0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
		if (scenenum < cutscenes[cutnum]->numscenes)
		{
			if (cutscenes[cutnum]->scene[scenenum].pichires[picnum])
				V_DrawSmallScaledPatch(picxpos, picypos, 0, W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
			else
				V_DrawScaledPatch(picxpos, picypos, 0, W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
		}
		F_WipeEndScreen(0, 0, vid.width, vid.height);

		F_RunWipe(TICRATE, true);
	}

	finaletextcount = 0;
	timetonext = 0;
	stoptimer = 0;

	if (scenenum >= cutscenes[cutnum]->numscenes)
	{
		if (cutnum == creditscutscene-1)
			F_StartGameEvaluation();
		else
			F_EndCutScene();
		return;
	}

	finaletext = cutscenes[cutnum]->scene[scenenum].text;

	picnum = 0;
	picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
	picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
	textxpos = cutscenes[cutnum]->scene[scenenum].textxpos;
	textypos = cutscenes[cutnum]->scene[scenenum].textypos;

	animtimer = pictime = cutscenes[cutnum]->scene[scenenum].picduration[picnum];
}

static void F_CutsceneTextWrite(void)
{
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (cutscenes[cutnum]->scene[scenenum].picname[picnum][0] != '\0')
	{
		if (cutscenes[cutnum]->scene[scenenum].pichires[picnum])
			V_DrawSmallScaledPatch(picxpos, picypos, 0,
				W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
		else
			V_DrawScaledPatch(picxpos,picypos, 0,
				W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
	}

	if (animtimer)
	{
		animtimer--;
		if (animtimer <= 0)
		{
			if (picnum < 7
				&& cutscenes[cutnum]->scene[scenenum].picname[picnum+1][0] != '\0')
			{
				picnum++;
				picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
				picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
				pictime = cutscenes[cutnum]->scene[scenenum].picduration[picnum];
				animtimer = pictime;
			}
			else
				timetonext = 2;
		}
	}

	F_WriteCutsceneText();
}

void F_EndCutScene(void)
{
	if (runningprecutscene && server)
	{
		D_MapChange(gamemap, gametype, ultimatemode, precutresetplayer, 0, true, false);
	}
	else
	{
		if (cutnum == introtoplay-1)
		{
			D_StartTitle();
			return;
		}

		if (nextmap < 1100-1)
			G_NextLevel();
		else
			Y_EndGame();
	}
}

//
// F_CutsceneDrawer
//
void F_CutsceneDrawer(void)
{
	if (timetonext)
		timetonext--;

	stoptimer++;

	if (timetonext == 1 && stoptimer > 0)
		F_AdvanceToNextScene();

	F_CutsceneTextWrite();
}
