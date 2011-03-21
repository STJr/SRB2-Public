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
/// \brief SRB2 selection menu, options, sliders and icons. Kinda widget stuff.
///
///	\warning: \n
///	All V_DrawPatchDirect() has been replaced by V_DrawScaledPatch()
///	so that the menu is scaled to the screen size. The scaling is always
///	an integer multiple of the original size, so that the graphics look
///	good.

#ifdef __GNUC__
#include <unistd.h>
#endif

#include "am_map.h"

#include "doomdef.h"
#include "d_main.h"
#include "d_netcmd.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"

#include "f_finale.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "d_net.h"
#include "mserv.h"
#include "m_misc.h"

#include "byteptr.h"

#include "st_stuff.h"

#include "i_sound.h"

#include "p_setup.h"

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

boolean menuactive = false;
boolean fromlevelselect = false;
static boolean pandoralevelselect = false;
static INT32 fromloadgame = 0;

customsecrets_t customsecretinfo[15];
INT32 inlevelselect = 0;

static INT32 lastmapnum;
static INT32 oldlastmapnum;

#define SKULLXOFF -32
#define LINEHEIGHT 16
#define STRINGHEIGHT 8
#define FONTBHEIGHT 20
#define SMALLLINEHEIGHT 8
#define SLIDER_RANGE 10
#define SLIDER_WIDTH (8*SLIDER_RANGE+6)
#define MAXSTRINGLENGTH 32
#define SERVERS_PER_PAGE 10

typedef enum
{
	QUITMSG,
	QUITMSG1,
	QUITMSG2,
	QUITMSG3,
	QUITMSG4,
	QUITMSG5,
	QUITMSG6,
	QUITMSG7,

	QUIT2MSG,
	QUIT2MSG1,
	QUIT2MSG2,
	QUIT2MSG3,
	QUIT2MSG4,
	QUIT2MSG5,
	QUIT2MSG6,

	QUIT3MSG,
	QUIT3MSG1,
	QUIT3MSG2,
	QUIT3MSG3,
	QUIT3MSG4,
	QUIT3MSG5,
	QUIT3MSG6
} text_enum;

const char *quitmsg[NUM_QUITMESSAGES];

// Stuff for customizing the player select screen Tails 09-22-2003
description_t description[15] =
{
	{"             Fastest\n                 Speed Thok\n             Not a good pick\nfor starters, but when\ncontrolled properly,\nSonic is the most\npowerful of the three.", "SONCCHAR", "", "SONIC"},
	{"             Slowest\n                 Fly/Swim\n             Good for\nbeginners. Tails\nhandles the best. His\nflying and swimming\nwill come in handy.", "TAILCHAR", "", "TAILS"},
	{"             Medium\n                 Glide/Climb\n             A well rounded\nchoice, Knuckles\ncompromises the speed\nof Sonic with the\nhandling of Tails.", "KNUXCHAR", "", "KNUCKLES"},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
	{"             Unknown\n                 Unknown\n             None", "SONCCHAR", "", ""},
};

static INT32 saveSlotSelected = 0;
static char joystickInfo[8][25];
#ifndef NONET
static UINT32 serverlistpage;
#endif

typedef struct
{
	char playername[SKINNAMESIZE];
	char levelname[32];
	UINT8 actnum;
	UINT8 skincolor;
	UINT8 skinnum;
	UINT8 numemeralds;
	INT32 lives;
	INT32 continues;
	INT32 gamemap;
	UINT8 netgame;
} saveinfo_t;

static saveinfo_t savegameinfo[10]; // Extra info about the save games.

#ifndef NONET
static char setupm_ip[16];
#endif
INT16 startmap; // Mario, NiGHTS, or just a plain old normal game?

static INT16 itemOn = 1; // menu item skull is on, Hack by Tails 09-18-2002
static INT16 skullAnimCounter = 10; // skull animation counter

//
// PROTOTYPES
//
static void M_DrawSaveLoadBorder(INT32 x,INT32 y);

static void M_DrawThermo(INT32 x,INT32 y,consvar_t *cv);
static void M_DrawSlider(INT32 x, INT32 y, const consvar_t *cv);
static void M_CentreText(INT32 y, const char *string); // write text centered
static void M_CustomLevelSelect(INT32 choice);
static void M_StopMessage(INT32 choice);
static void M_GameOption(INT32 choice);
static void M_NetOption(INT32 choice);

static void M_GametypeOptions(INT32 choice);

#if defined (HWRENDER) && defined (SHUFFLE)
static void M_OpenGLOption(INT32 choice);
#endif

#ifndef NONET
static void M_NextServerPage(void);
static void M_PrevServerPage(void);
static void M_RoomInfoMenu(INT32 choice);
#endif
static void M_SortServerList(void);

static const char *ALREADYPLAYING = "You are already playing.\nDo you wish to end the\ncurrent game? (Y/N)\n";

// current menudef
menu_t *currentMenu = &MainDef;
//===========================================================================
//Generic Stuffs (more easy to create menus :))
//===========================================================================

static void M_DrawMenuTitle(void)
{
	if (currentMenu->menutitlepic)
	{
		patch_t *p = W_CachePatchName(currentMenu->menutitlepic, PU_CACHE);

		INT32 xtitle = (BASEVIDWIDTH - SHORT(p->width))/2;
		INT32 ytitle = (currentMenu->y - SHORT(p->height))/2;

		if (xtitle < 0)
			xtitle = 0;
		if (ytitle < 0)
			ytitle = 0;
		V_DrawScaledPatch(xtitle, ytitle, 0, p);
	}
}

void M_DrawGenericMenu(void)
{
	INT32 x, y, i, cursory = 0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();

	for (i = 0; i < currentMenu->numitems; i++)
	{
		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
				{
					if (currentMenu->menuitems[i].status & IT_CENTER)
					{
						patch_t *p;
						p = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
						V_DrawScaledPatch((BASEVIDWIDTH - SHORT(p->width))/2, y, 0, p);
					}
					else
					{
						V_DrawScaledPatch(x, y, 0,
							W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
					}
				}
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y += LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;

				if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
					V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
				else
					V_DrawString(x, y, V_YELLOWMAP, currentMenu->menuitems[i].text);

				// Cvar specific handling
				switch (currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
						switch (currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider(x, y, cv);
							case IT_CV_NOPRINT: // color use this
								break;
							case IT_CV_STRING:
								M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
								V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, cv->string);
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter(x + 8 + V_StringWidth(cv->string), y + 12,
										'_' | 0x80, false);
								y += 16;
								break;
							default:
								V_DrawString(BASEVIDWIDTH - x - V_StringWidth(cv->string), y,
									V_YELLOWMAP, cv->string);
								break;
						}
						break;
					}
					y += STRINGHEIGHT;
					break;
			case IT_STRING2:
				V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
			case IT_DYLITLSPACE:
				y += SMALLLINEHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawMappedPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch,PU_CACHE), graymap);
				y += LINEHEIGHT;
				break;
		}
	}

	// DRAW THE SKULL CURSOR
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
		|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
	{
		V_DrawScaledPatch(currentMenu->x + SKULLXOFF, cursory - 5, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
	}
	else
	{
		V_DrawScaledPatch(currentMenu->x - 24, cursory, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawString(currentMenu->x, cursory, V_YELLOWMAP, currentMenu->menuitems[itemOn].text);
	}
}

static void M_DrawCenteredMenu(void)
{
	INT32 x, y, i, cursory = 0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();

	for (i = 0; i < currentMenu->numitems; i++)
	{
		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
				{
					if (currentMenu->menuitems[i].status & IT_CENTER)
					{
						patch_t *p;
						p = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
						V_DrawScaledPatch((BASEVIDWIDTH - SHORT(p->width))/2, y, 0, p);
					}
					else
					{
						V_DrawScaledPatch(x, y, 0,
							W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
					}
				}
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y += LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;

				if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
					V_DrawCenteredString(x, y, 0, currentMenu->menuitems[i].text);
				else
					V_DrawCenteredString(x, y, V_YELLOWMAP, currentMenu->menuitems[i].text);

				// Cvar specific handling
				switch(currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
						switch(currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider(x, y, cv);
							case IT_CV_NOPRINT: // color use this
								break;
							case IT_CV_STRING:
								M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
								V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, cv->string);
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter(x + 8 + V_StringWidth(cv->string), y + 12,
										'_' | 0x80, false);
								y += 16;
								break;
							default:
								V_DrawString(BASEVIDWIDTH - x - V_StringWidth(cv->string), y,
									V_YELLOWMAP, cv->string);
								break;
						}
						break;
					}
					y += STRINGHEIGHT;
					break;
			case IT_STRING2:
				V_DrawCenteredString(x, y, 0, currentMenu->menuitems[i].text);
			case IT_DYLITLSPACE:
				y += SMALLLINEHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawMappedPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch,PU_CACHE), graymap);
				y += LINEHEIGHT;
				break;
		}
	}

	// DRAW THE SKULL CURSOR
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
		|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
	{
		V_DrawScaledPatch(x + SKULLXOFF, cursory - 5, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
	}
	else
	{
		V_DrawScaledPatch(x - V_StringWidth(currentMenu->menuitems[itemOn].text)/2 - 24, cursory, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawCenteredString(x, cursory, V_YELLOWMAP, currentMenu->menuitems[itemOn].text);
	}
}

//
// M_StringHeight
//
// Find string height from hu_font chars
//
static inline size_t M_StringHeight(const char *string)
{
	size_t h = 8, i;

	for (i = 0; i < strlen(string); i++)
		if (string[i] == '\n')
			h += 8;

	return h;
}

//===========================================================================
//MAIN MENU
//===========================================================================

static void M_QuitSRB2(INT32 choice);
static void M_OptionsMenu(INT32 choice);
static void M_SecretsMenu(INT32 choice);
static void M_CustomSecretsMenu(INT32 choice);
static void M_MapChange(INT32 choice);
static void M_TeamChange(INT32 choice);
static void M_ConfirmSpectate(INT32 choice);
static void M_TeamScramble(INT32 choice);
static void M_ConfirmTeamScramble(INT32 choice);

typedef enum
{
	scramble = 0,
	spectate,
	switchteam,
	switchmap,
	secrets,
	singleplr,
	multiplr,
	options,
	quitdoom,
	main_end
} main_e;

static menuitem_t MainMenu[] =
{
	{IT_STRING  | IT_CALL,   NULL, "Scramble Teams...", M_TeamScramble,        56},
	{IT_STRING  | IT_CALL,   NULL, "Spectate..."      , M_ConfirmSpectate,     64},
	{IT_STRING  | IT_CALL,   NULL, "Switch Team..."   , M_TeamChange,          64},
	{IT_STRING  | IT_CALL,   NULL, "Switch Map..."    , M_MapChange,           72},
	{IT_CALL    | IT_STRING, NULL, "secrets"          , M_SecretsMenu,         84},
	{IT_SUBMENU | IT_STRING, NULL, "1 player"         , &SinglePlayerDef,      92},
	{IT_SUBMENU | IT_STRING, NULL, "multiplayer"      , &MultiPlayerDef,      100},
	{IT_CALL    | IT_STRING, NULL, "options"          , M_OptionsMenu,        108},
	{IT_CALL    | IT_STRING, NULL, "quit  game"       , M_QuitSRB2,           116},
};

menu_t MainDef =
{
	NULL,
	NULL,
	main_end,
	NULL,
	MainMenu,
	M_DrawCenteredMenu,
	BASEVIDWIDTH/2, 72,
	0,
	NULL
};

static void M_DrawStats(void);
static void M_DrawStats2(void);
static void M_DrawStats3(void);
static void M_DrawStats4(void);
static void M_DrawStats5(void);
static void M_Stats2(INT32 choice);
static void M_Stats3(INT32 choice);
static void M_Stats4(INT32 choice);

// Empty thingy for stats5 menu
typedef enum
{
	statsempty5,
	stats5_end
} stats5_e;

static menuitem_t Stats5Menu[] =
{
	{IT_SUBMENU | IT_STRING, NULL, "NEXT", &StatsDef, 192},
};

menu_t Stats5Def =
{
	NULL,
	NULL,
	stats5_end,
	&SinglePlayerDef,
	Stats5Menu,
	M_DrawStats5,
	280, 185,
	0,
	NULL
};

// Empty thingy for stats4 menu
typedef enum
{
	statsempty4,
	stats4_end
} stats4_e;

static menuitem_t Stats4Menu[] =
{
	{IT_SUBMENU | IT_STRING, NULL, "NEXT", &Stats5Def, 192},
};

menu_t Stats4Def =
{
	NULL,
	NULL,
	stats4_end,
	&SinglePlayerDef,
	Stats4Menu,
	M_DrawStats4,
	280, 185,
	0,
	NULL
};

// Empty thingy for stats3 menu
typedef enum
{
	statsempty3,
	stats3_end
} stats3_e;

static menuitem_t Stats3Menu[] =
{
	{IT_CALL | IT_STRING, NULL, "NEXT", M_Stats4, 192},
};

menu_t Stats3Def =
{
	NULL,
	NULL,
	stats3_end,
	&SinglePlayerDef,
	Stats3Menu,
	M_DrawStats3,
	280, 185,
	0,
	NULL
};

// Empty thingy for stats2 menu
typedef enum
{
	statsempty2,
	stats2_end
} stats2_e;

static menuitem_t Stats2Menu[] =
{
	{IT_CALL | IT_STRING, NULL, "NEXT", M_Stats3, 192},
};

menu_t Stats2Def =
{
	NULL,
	NULL,
	stats2_end,
	&SinglePlayerDef,
	Stats2Menu,
	M_DrawStats2,
	280, 185,
	0,
	NULL
};

// Empty thingy for stats menu
typedef enum
{
	statsempty1,
	stats_end
} stats_e;

static menuitem_t StatsMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "NEXT", M_Stats2, 192},
};

menu_t StatsDef =
{
	NULL,
	NULL,
	stats_end,
	&SinglePlayerDef,
	StatsMenu,
	M_DrawStats,
	280, 185,
	0,
	NULL
};

//===========================================================================
//SINGLE PLAYER MENU
//===========================================================================
// Menu Revamp! Tails 11-30-2000
static void M_NewGame(void);
static void M_LoadGame(INT32 choice);
static void M_Statistics(INT32 choice);
static void M_TimeAttack(INT32 choice);

typedef enum
{
	newgame = 0,
	timeattack,
	statistics,
	endgame,
	single_end
} single_e;

static menuitem_t SinglePlayerMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "Start Game", M_LoadGame,     92},
	{IT_CALL | IT_STRING, NULL, "Time Attack",M_TimeAttack,  100},
	{IT_CALL | IT_STRING, NULL, "Statistics", M_Statistics,  108},
	{IT_CALL | IT_STRING, NULL, "End Game",   M_EndGame,     116},
};

menu_t SinglePlayerDef =
{
	0,
	"Single Player",
	single_end,
	&MainDef,
	SinglePlayerMenu,
	M_DrawGenericMenu,
	130, 72, // Tails 11-30-2000
	0,
	NULL
};

//===========================================================================
// Connect Menu
//===========================================================================
static CV_PossibleValue_t serversort_cons_t[] = {
	{0,"Ping"},
	{1,"Players"},
	{2,"Gametype"},
	{0,NULL}};

consvar_t cv_serversort = {"serversort", "Ping", CV_HIDEN | CV_CALL, serversort_cons_t, M_SortServerList, 0, NULL, NULL, 0, 0, NULL};

#ifndef NONET
static CV_PossibleValue_t serversearch_cons_t[] = {
	{0,"Local Lan"},
	{1,"Internet"},
	{0,NULL}};

static consvar_t cv_serversearch = {"serversearch", "Internet", CV_HIDEN, serversearch_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static void M_Chooseroom_Onchange(void);
static CV_PossibleValue_t rooms_cons_t[MAXROOMS+1] = {{1, "Offline"}};
consvar_t cv_chooseroom = {"chooseroom", "Offline", CV_HIDEN | CV_CALL, rooms_cons_t, M_Chooseroom_Onchange, 0, NULL, NULL, 0, 0, NULL};
const char *cv_chosenroom_motd;
#endif

#define FIRSTSERVERLINE 7
#define FIRSTLANSERVERLINE 5

#ifndef NONET
static void M_Connect(INT32 choice)
{
	// do not call menuexitfunc
	M_ClearMenus(false);

	COM_BufAddText(va("connect node %d\n", serverlist[choice-FIRSTSERVERLINE + serverlistpage * SERVERS_PER_PAGE].node));

	// A little "please wait" message.
	M_DrawTextBox(56, BASEVIDHEIGHT/2-12, 24, 2);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Connecting to server...");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer
}

// Tails 11-19-2002
static void M_ConnectIP(INT32 choice)
{
	(void)choice;
	COM_BufAddText(va("connect %s\n", setupm_ip));

	// A little "please wait" message.
	M_DrawTextBox(56, BASEVIDHEIGHT/2-12, 24, 2);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Connecting to server...");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer
}

static UINT32 localservercount;

static void M_Refresh(INT32 choice)
{
	(void)choice;
	CL_UpdateServerList(cv_serversearch.value, cv_chooseroom.value);

	// first page of servers
	serverlistpage = 0;
}

static menuitem_t  ConnectMenu[] =
{
	{IT_STRING | IT_CVAR,  NULL, "Room",		   &cv_chooseroom,	 0},
	{IT_STRING | IT_CALL,  NULL, "Room Info",	   M_RoomInfoMenu,	 0},
	{IT_STRING | IT_CVAR,  NULL, "Sort By",        &cv_serversort,   0},
	{IT_STRING | IT_CALL,  NULL, "Next Page",      M_NextServerPage, 0},
	{IT_STRING | IT_CALL,  NULL, "Previous Page",  M_PrevServerPage, 0},
	{IT_STRING | IT_CALL,  NULL, "Refresh",        M_Refresh,        0},
	{IT_WHITESTRING | IT_SPACE,
	                       NULL, "Server Name",
	                                              NULL,              0}, // Tails 01-18-2001
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
};

static menuitem_t  ConnectLANMenu[] =
{
	{IT_STRING | IT_CVAR,  NULL, "Sort By",        &cv_serversort,   0},
	{IT_STRING | IT_CALL,  NULL, "Next Page",      M_NextServerPage, 0},
	{IT_STRING | IT_CALL,  NULL, "Previous Page",  M_PrevServerPage, 0},
	{IT_STRING | IT_CALL,  NULL, "Refresh",        M_Refresh,        0},
	{IT_WHITESTRING | IT_SPACE,
	                       NULL, "Server Name",
	                                              NULL,              0}, // Tails 01-18-2001
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         0},
};

static void M_DrawConnectMenu(void)
{
	UINT16 i;
	char *p;
	char cgametype;
	char servername[21];

	for (i = FIRSTSERVERLINE; i < min(localservercount, SERVERS_PER_PAGE)+FIRSTSERVERLINE; i++)
		ConnectMenu[i].status = IT_STRING | IT_SPACE;

	V_DrawRightAlignedString(currentMenu->x+284, currentMenu->y+((FIRSTSERVERLINE-1)*STRINGHEIGHT), V_YELLOWMAP, "PING   PLYS  GT");

	if (rooms_cons_t[0].value < 0)
		V_DrawCenteredString(BASEVIDWIDTH/2,currentMenu->y+FIRSTSERVERLINE*STRINGHEIGHT,0,"Error contacting the Master Server");
	else if (serverlistcount <= 0)
		V_DrawString (currentMenu->x,currentMenu->y+FIRSTSERVERLINE*STRINGHEIGHT,0,"No servers found");
	else
	for (i = 0; i < min(serverlistcount - serverlistpage * SERVERS_PER_PAGE, SERVERS_PER_PAGE); i++)
	{
		INT32 slindex = i + serverlistpage * SERVERS_PER_PAGE;
		UINT32 globalflags = ((serverlist[slindex].info.numberofplayer >= serverlist[slindex].info.maxplayer) ? V_TRANSLUCENT : 0)
			| ((itemOn == FIRSTSERVERLINE+i) ? V_YELLOWMAP : 0);

		strlcpy(servername, serverlist[slindex].info.servername, sizeof (servername));
		servername[20] = '\0';

		V_DrawString(currentMenu->x,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags | V_ALLOWLOWERCASE, servername);

		if (serverlist[slindex].info.modifiedgame)
			V_DrawString(currentMenu->x+164,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, 0, "\x85" "M");
		if (serverlist[slindex].info.cheatsenabled)
			V_DrawString(currentMenu->x+172,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, 0, "\x83" "C");

		p = va("%u", (UINT32)LONG(serverlist[slindex].info.time));
		V_DrawCenteredString (currentMenu->x+200,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);

		switch (serverlist[slindex].info.gametype)
		{
			case GT_COOP:
				cgametype = 'C';
				break;
			case GT_MATCH:
				cgametype = 'M';
				break;
			case GT_RACE:
				cgametype = 'R';
				break;
			case GT_TAG:
				cgametype = 'T';
				break;
			case GT_CTF:
				cgametype = 'F';
				break;
			default:
				cgametype = 'U';
				DEBPRINT(va("M_DrawConnectMenu: Unknown gametype %d\n", serverlist[slindex].info.gametype));
				break;
		}

		p = va("%02d/%02d", serverlist[slindex].info.numberofplayer,
		                    serverlist[slindex].info.maxplayer); // Tails 01-18-2001
		V_DrawRightAlignedString(currentMenu->x+264,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);

		p = va("%c", cgametype);
		V_DrawString(currentMenu->x+272,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);


		ConnectMenu[i+FIRSTSERVERLINE].status = IT_STRING | IT_CALL;
	}

	if (cv_chooseroom.value != oldroomnum)
	{
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-20, V_YELLOWMAP, "Warning: Room has been changed");
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-12, V_YELLOWMAP, "since the last refresh.");
	}
	else
	{
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-20, 0, "\x85" "M" "\x82" " = Game is modified.    ");
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-12, 0, "\x83" "C" "\x82" " = Cheats are enabled.");
	}

	localservercount = serverlistcount;

	M_DrawGenericMenu();
}

static void M_DrawConnectLANMenu(void)
{
	UINT16 i;
	char *p;
	char cgametype;
	char servername[21];

	for (i = FIRSTLANSERVERLINE; i < min(localservercount, SERVERS_PER_PAGE)+FIRSTLANSERVERLINE; i++)
		ConnectLANMenu[i].status = IT_STRING | IT_SPACE;

	if (serverlistcount <= 0)
		V_DrawString (currentMenu->x,currentMenu->y+FIRSTLANSERVERLINE*STRINGHEIGHT,0,"No servers found");
	else
	for (i = 0; i < min(serverlistcount - serverlistpage * SERVERS_PER_PAGE, SERVERS_PER_PAGE); i++)
	{
		INT32 slindex = i + serverlistpage * SERVERS_PER_PAGE;
		UINT32 globalflags = ((serverlist[slindex].info.numberofplayer >= serverlist[slindex].info.maxplayer) ? V_TRANSLUCENT : 0)
			| ((itemOn == FIRSTSERVERLINE+i) ? V_YELLOWMAP : 0);

		strlcpy(servername, serverlist[slindex].info.servername, sizeof (servername));
		servername[20] = '\0';

		V_DrawString(currentMenu->x,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags | V_ALLOWLOWERCASE, servername);

		if (serverlist[slindex].info.modifiedgame)
			V_DrawString(currentMenu->x+164,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, 0, "\x85" "M");
		if (serverlist[slindex].info.cheatsenabled)
			V_DrawString(currentMenu->x+172,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, 0, "\x83" "C");

		p = va("%u", (UINT32)LONG(serverlist[slindex].info.time));
		V_DrawCenteredString (currentMenu->x+200,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);

		switch (serverlist[slindex].info.gametype)
		{
			case GT_COOP:
				cgametype = 'C';
				break;
			case GT_MATCH:
				cgametype = 'M';
				break;
			case GT_RACE:
				cgametype = 'R';
				break;
			case GT_TAG:
				cgametype = 'T';
				break;
			case GT_CTF:
				cgametype = 'F';
				break;
			default:
				cgametype = 'U';
				DEBPRINT(va("M_DrawConnectLANMenu: Unknown gametype %d\n", serverlist[slindex].info.gametype));
				break;
		}

		p = va("%02d/%02d", serverlist[slindex].info.numberofplayer,
		                    serverlist[slindex].info.maxplayer); // Tails 01-18-2001
		V_DrawRightAlignedString(currentMenu->x+264,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);

		p = va("%c", cgametype);
		V_DrawString(currentMenu->x+272,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT, globalflags, p);

		ConnectLANMenu[i+FIRSTLANSERVERLINE].status = IT_STRING | IT_CALL;
	}

	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-20, 0, "\x85" "M" "\x82" " = Game is modified.    ");
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-12, 0, "\x83" "C" "\x82" " = Cheats are enabled.");

	localservercount = serverlistcount;

	M_DrawGenericMenu();
}

static boolean M_CancelConnect(void)
{
	D_CloseConnection();
	return true;
}

menu_t Connectdef =
{
	0,
	"Connect Server",
	sizeof (ConnectMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	ConnectMenu,
	M_DrawConnectMenu,
	27,40,
	0,
	M_CancelConnect
};

menu_t ConnectLANdef =
{
	0,
	"Connect LAN Server",
	sizeof (ConnectLANMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	ConnectLANMenu,
	M_DrawConnectLANMenu,
	27,40,
	0,
	M_CancelConnect
};

// Connect using IP address Tails 11-19-2002
static void M_HandleConnectIP(INT32 choice);
static menuitem_t  ConnectIPMenu[] =
{
	{IT_KEYHANDLER | IT_STRING, NULL, "  IP Address:", M_HandleConnectIP, 0},
};

static menuitem_t  RoomInfoMenu[] =
{
	{IT_STRING | IT_CVAR,  NULL, "Room", &cv_chooseroom,	 0},
};

static void M_DrawConnectIPMenu(void);

menu_t ConnectIPdef =
{
	0,
	"Connect Server",
	sizeof (ConnectIPMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	ConnectIPMenu,
	M_DrawConnectIPMenu,
	27,40,
	0,
	M_CancelConnect
};

static void M_DrawRoomInfoMenu(void)
{
	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();
	M_DrawTextBox(0, 56, 38, 12);
#ifndef NONET
	V_DrawString(8, 64, V_WORDWRAP|V_ALLOWLOWERCASE, cv_chosenroom_motd);
#endif
}

menu_t RoomInfodef =
{
	0,
	"Room Info",
	sizeof (RoomInfoMenu)/sizeof (menuitem_t),
	&Connectdef,
	RoomInfoMenu,
	M_DrawRoomInfoMenu,
	27,40,
	0,
	NULL
};

static void M_Chooseroom_Onchange(void)
{
#ifndef NONET
	if (currentMenu == &RoomInfodef)
	{
		M_AlterRoomInfo();
	}
#endif
}

//
// M_PatchRoomsTable
//
// Like M_PatchSkinNameTable, but for cv_chooseroom.
//

static int M_PatchRoomsTable(boolean hosting)
{
	INT32 i = -1;
	memset(rooms_cons_t, 0, sizeof (rooms_cons_t));

	if(GetRoomsList(hosting) < 0)
	{
		return false;
	}

	for (i = 0; room_list[i].header.buffer[0]; i++)
	{
		if(room_list[i].name != '\0')
		{
			rooms_cons_t[i].strvalue = room_list[i].name;
			rooms_cons_t[i].value = room_list[i].id;
		}
		else
		{
			rooms_cons_t[i].strvalue = NULL;
			rooms_cons_t[i].value = 0;
		}
	}

	CV_SetValue(&cv_chooseroom, rooms_cons_t[0].value);
	CV_AddValue(&cv_chooseroom, 1);
	CV_AddValue(&cv_chooseroom, -1);

	return true;
}

#ifdef UPDATE_ALERT
static int M_CheckMODVersion(void)
{
	char updatestring[500];
	const char *updatecheck = GetMODVersion();
	if(updatecheck)
	{
		sprintf(updatestring, UPDATE_ALERT_STRING, VERSIONSTRING, updatecheck);
		M_StartMessage(updatestring, NULL, MM_NOTHING);
		return false;
	} else
		return true;
}
#endif

static void M_ConnectMenu(INT32 choice)
{
	(void)choice;
	if (modifiedgame)
	{
		M_StartMessage("You have wad files loaded and/or\nmodified the game in some way.\nPlease restart SRB2 before\nconnecting.", NULL, MM_NOTHING);
		return;
	}

	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}

	// Display a little "please wait" message.
	M_DrawTextBox(52, BASEVIDHEIGHT/2-10, 25, 3);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Contacting list server...");
	V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2)+12, 0, "Please wait.");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer

	// first page of servers
	serverlistpage = 0;
	cv_serversearch.value = 1;
	if(!M_PatchRoomsTable(false))
		return;
#ifdef UPDATE_ALERT
	if(M_CheckMODVersion())
	{
#endif
		M_SetupNextMenu(&Connectdef);
		M_Refresh(0);
#ifdef UPDATE_ALERT
	}
#endif
}

static void M_ConnectLANMenu(INT32 choice)
{
	(void)choice;
	if (modifiedgame)
	{
		M_StartMessage("You have wad files loaded and/or\nmodified the game in some way.\nPlease restart SRB2 before\nconnecting.", NULL, MM_NOTHING);
		return;
	}

	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}

	// Display a little "please wait" message.
	M_DrawTextBox(52, BASEVIDHEIGHT/2-10, 25, 3);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Searching Local Network for Servers...");
	V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2)+12, 0, "Please wait.");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer

	// first page of servers
	cv_serversearch.value = 0;
	serverlistpage = 0;
	M_SetupNextMenu(&ConnectLANdef);
	M_Refresh(0);
}

// Connect using IP address Tails 11-19-2002
static void M_ConnectIPMenu(INT32 choice)
{
	(void)choice;
	if (modifiedgame)
	{
		M_StartMessage("You have wad files loaded and/or\nmodified the game in some way.\nPlease restart SRB2 before\nconnecting.", NULL, MM_NOTHING);
		return;
	}

	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}

	M_SetupNextMenu(&ConnectIPdef);
}

static void M_RoomInfoMenu(INT32 choice)
{
	(void)choice;

	M_AlterRoomInfo();

	RoomInfodef.prevMenu = currentMenu;

	M_SetupNextMenu(&RoomInfodef);
}

void M_AlterRoomInfo(void)
{
	INT32 i = -1;

	for (i = 0; room_list[i].header.buffer[0]; i++)
	{
		if(cv_chooseroom.value == room_list[i].id)
		{
			cv_chosenroom_motd = room_list[i].motd;
			break;
		}
	}
}
#endif

//===========================================================================
// Start Server Menu
//===========================================================================

static CV_PossibleValue_t map_cons_t[LEVELARRAYSIZE] = {
	{1,"MAP01"},
	{2,"MAP02"},
	{3,"MAP03"},
	{4,"MAP04"},
	{5,"MAP05"},
	{6,"MAP06"},
	{7,"MAP07"},
	{8,"MAP08"},
	{9,"MAP09"},
	{10,"MAP10"},
	{11,"MAP11"},
	{12,"MAP12"},
	{13,"MAP13"},
	{14,"MAP14"},
	{15,"MAP15"},
	{16,"MAP16"},
	{17,"MAP17"},
	{18,"MAP18"},
	{19,"MAP19"},
	{20,"MAP20"},
	{21,"MAP21"},
	{22,"MAP22"},
	{23,"MAP23"},
	{24,"MAP24"},
	{25,"MAP25"},
	{26,"MAP26"},
	{27,"MAP27"},
	{28,"MAP28"},
	{29,"MAP29"},
	{30,"MAP30"},
	{31,"MAP31"},
	{32,"MAP32"},
	{33,"MAP33"},
	{34,"MAP34"},
	{35,"MAP35"},
	{36,"MAP36"},
	{37,"MAP37"},
	{38,"MAP38"},
	{39,"MAP39"},
	{40,"MAP40"},
	{41,"MAP41"},
	{42,"MAP42"},
	{43,"MAP43"},
	{44,"MAP44"},
	{45,"MAP45"},
	{46,"MAP46"},
	{47,"MAP47"},
	{48,"MAP48"},
	{49,"MAP49"},
	{50,"MAP50"},
	{51,"MAP51"},
	{52,"MAP52"},
	{53,"MAP53"},
	{54,"MAP54"},
	{55,"MAP55"},
	{56,"MAP56"},
	{57,"MAP57"},
	{58,"MAP58"},
	{59,"MAP59"},
	{60,"MAP60"},
	{61,"MAP61"},
	{62,"MAP62"},
	{63,"MAP63"},
	{64,"MAP64"},
	{65,"MAP65"},
	{66,"MAP66"},
	{67,"MAP67"},
	{68,"MAP68"},
	{69,"MAP69"},
	{70,"MAP70"},
	{71,"MAP71"},
	{72,"MAP72"},
	{73,"MAP73"},
	{74,"MAP74"},
	{75,"MAP75"},
	{76,"MAP76"},
	{77,"MAP77"},
	{78,"MAP78"},
	{79,"MAP79"},
	{80,"MAP80"},
	{81,"MAP81"},
	{82,"MAP82"},
	{83,"MAP83"},
	{84,"MAP84"},
	{85,"MAP85"},
	{86,"MAP86"},
	{87,"MAP87"},
	{88,"MAP88"},
	{89,"MAP89"},
	{90,"MAP90"},
	{91,"MAP91"},
	{92,"MAP92"},
	{93,"MAP93"},
	{94,"MAP94"},
	{95,"MAP95"},
	{96,"MAP96"},
	{97,"MAP97"},
	{98,"MAP98"},
	{99,"MAP99"},
	{100, "MAP100"},
	{101, "MAP101"},
	{102, "MAP102"},
	{103, "MAP103"},
	{104, "MAP104"},
	{105, "MAP105"},
	{106, "MAP106"},
	{107, "MAP107"},
	{108, "MAP108"},
	{109, "MAP109"},
	{110, "MAP110"},
	{111, "MAP111"},
	{112, "MAP112"},
	{113, "MAP113"},
	{114, "MAP114"},
	{115, "MAP115"},
	{116, "MAP116"},
	{117, "MAP117"},
	{118, "MAP118"},
	{119, "MAP119"},
	{120, "MAP120"},
	{121, "MAP121"},
	{122, "MAP122"},
	{123, "MAP123"},
	{124, "MAP124"},
	{125, "MAP125"},
	{126, "MAP126"},
	{127, "MAP127"},
	{128, "MAP128"},
	{129, "MAP129"},
	{130, "MAP130"},
	{131, "MAP131"},
	{132, "MAP132"},
	{133, "MAP133"},
	{134, "MAP134"},
	{135, "MAP135"},
	{136, "MAP136"},
	{137, "MAP137"},
	{138, "MAP138"},
	{139, "MAP139"},
	{140, "MAP140"},
	{141, "MAP141"},
	{142, "MAP142"},
	{143, "MAP143"},
	{144, "MAP144"},
	{145, "MAP145"},
	{146, "MAP146"},
	{147, "MAP147"},
	{148, "MAP148"},
	{149, "MAP149"},
	{150, "MAP150"},
	{151, "MAP151"},
	{152, "MAP152"},
	{153, "MAP153"},
	{154, "MAP154"},
	{155, "MAP155"},
	{156, "MAP156"},
	{157, "MAP157"},
	{158, "MAP158"},
	{159, "MAP159"},
	{160, "MAP160"},
	{161, "MAP161"},
	{162, "MAP162"},
	{163, "MAP163"},
	{164, "MAP164"},
	{165, "MAP165"},
	{166, "MAP166"},
	{167, "MAP167"},
	{168, "MAP168"},
	{169, "MAP169"},
	{170, "MAP170"},
	{171, "MAP171"},
	{172, "MAP172"},
	{173, "MAP173"},
	{174, "MAP174"},
	{175, "MAP175"},
	{176, "MAP176"},
	{177, "MAP177"},
	{178, "MAP178"},
	{179, "MAP179"},
	{180, "MAP180"},
	{181, "MAP181"},
	{182, "MAP182"},
	{183, "MAP183"},
	{184, "MAP184"},
	{185, "MAP185"},
	{186, "MAP186"},
	{187, "MAP187"},
	{188, "MAP188"},
	{189, "MAP189"},
	{190, "MAP190"},
	{191, "MAP191"},
	{192, "MAP192"},
	{193, "MAP193"},
	{194, "MAP194"},
	{195, "MAP195"},
	{196, "MAP196"},
	{197, "MAP197"},
	{198, "MAP198"},
	{199, "MAP199"},
	{200, "MAP200"},
	{201, "MAP201"},
	{202, "MAP202"},
	{203, "MAP203"},
	{204, "MAP204"},
	{205, "MAP205"},
	{206, "MAP206"},
	{207, "MAP207"},
	{208, "MAP208"},
	{209, "MAP209"},
	{210, "MAP210"},
	{211, "MAP211"},
	{212, "MAP212"},
	{213, "MAP213"},
	{214, "MAP214"},
	{215, "MAP215"},
	{216, "MAP216"},
	{217, "MAP217"},
	{218, "MAP218"},
	{219, "MAP219"},
	{220, "MAP220"},
	{221, "MAP221"},
	{222, "MAP222"},
	{223, "MAP223"},
	{224, "MAP224"},
	{225, "MAP225"},
	{226, "MAP226"},
	{227, "MAP227"},
	{228, "MAP228"},
	{229, "MAP229"},
	{230, "MAP230"},
	{231, "MAP231"},
	{232, "MAP232"},
	{233, "MAP233"},
	{234, "MAP234"},
	{235, "MAP235"},
	{236, "MAP236"},
	{237, "MAP237"},
	{238, "MAP238"},
	{239, "MAP239"},
	{240, "MAP240"},
	{241, "MAP241"},
	{242, "MAP242"},
	{243, "MAP243"},
	{244, "MAP244"},
	{245, "MAP245"},
	{246, "MAP246"},
	{247, "MAP247"},
	{248, "MAP248"},
	{249, "MAP249"},
	{250, "MAP250"},
	{251, "MAP251"},
	{252, "MAP252"},
	{253, "MAP253"},
	{254, "MAP254"},
	{255, "MAP255"},
	{256, "MAP256"},
	{257, "MAP257"},
	{258, "MAP258"},
	{259, "MAP259"},
	{260, "MAP260"},
	{261, "MAP261"},
	{262, "MAP262"},
	{263, "MAP263"},
	{264, "MAP264"},
	{265, "MAP265"},
	{266, "MAP266"},
	{267, "MAP267"},
	{268, "MAP268"},
	{269, "MAP269"},
	{270, "MAP270"},
	{271, "MAP271"},
	{272, "MAP272"},
	{273, "MAP273"},
	{274, "MAP274"},
	{275, "MAP275"},
	{276, "MAP276"},
	{277, "MAP277"},
	{278, "MAP278"},
	{279, "MAP279"},
	{280, "MAP280"},
	{281, "MAP281"},
	{282, "MAP282"},
	{283, "MAP283"},
	{284, "MAP284"},
	{285, "MAP285"},
	{286, "MAP286"},
	{287, "MAP287"},
	{288, "MAP288"},
	{289, "MAP289"},
	{290, "MAP290"},
	{291, "MAP291"},
	{292, "MAP292"},
	{293, "MAP293"},
	{294, "MAP294"},
	{295, "MAP295"},
	{296, "MAP296"},
	{297, "MAP297"},
	{298, "MAP298"},
	{299, "MAP299"},
	{300, "MAP300"},
	{301, "MAP301"},
	{302, "MAP302"},
	{303, "MAP303"},
	{304, "MAP304"},
	{305, "MAP305"},
	{306, "MAP306"},
	{307, "MAP307"},
	{308, "MAP308"},
	{309, "MAP309"},
	{310, "MAP310"},
	{311, "MAP311"},
	{312, "MAP312"},
	{313, "MAP313"},
	{314, "MAP314"},
	{315, "MAP315"},
	{316, "MAP316"},
	{317, "MAP317"},
	{318, "MAP318"},
	{319, "MAP319"},
	{320, "MAP320"},
	{321, "MAP321"},
	{322, "MAP322"},
	{323, "MAP323"},
	{324, "MAP324"},
	{325, "MAP325"},
	{326, "MAP326"},
	{327, "MAP327"},
	{328, "MAP328"},
	{329, "MAP329"},
	{330, "MAP330"},
	{331, "MAP331"},
	{332, "MAP332"},
	{333, "MAP333"},
	{334, "MAP334"},
	{335, "MAP335"},
	{336, "MAP336"},
	{337, "MAP337"},
	{338, "MAP338"},
	{339, "MAP339"},
	{340, "MAP340"},
	{341, "MAP341"},
	{342, "MAP342"},
	{343, "MAP343"},
	{344, "MAP344"},
	{345, "MAP345"},
	{346, "MAP346"},
	{347, "MAP347"},
	{348, "MAP348"},
	{349, "MAP349"},
	{350, "MAP350"},
	{351, "MAP351"},
	{352, "MAP352"},
	{353, "MAP353"},
	{354, "MAP354"},
	{355, "MAP355"},
	{356, "MAP356"},
	{357, "MAP357"},
	{358, "MAP358"},
	{359, "MAP359"},
	{360, "MAP360"},
	{361, "MAP361"},
	{362, "MAP362"},
	{363, "MAP363"},
	{364, "MAP364"},
	{365, "MAP365"},
	{366, "MAP366"},
	{367, "MAP367"},
	{368, "MAP368"},
	{369, "MAP369"},
	{370, "MAP370"},
	{371, "MAP371"},
	{372, "MAP372"},
	{373, "MAP373"},
	{374, "MAP374"},
	{375, "MAP375"},
	{376, "MAP376"},
	{377, "MAP377"},
	{378, "MAP378"},
	{379, "MAP379"},
	{380, "MAP380"},
	{381, "MAP381"},
	{382, "MAP382"},
	{383, "MAP383"},
	{384, "MAP384"},
	{385, "MAP385"},
	{386, "MAP386"},
	{387, "MAP387"},
	{388, "MAP388"},
	{389, "MAP389"},
	{390, "MAP390"},
	{391, "MAP391"},
	{392, "MAP392"},
	{393, "MAP393"},
	{394, "MAP394"},
	{395, "MAP395"},
	{396, "MAP396"},
	{397, "MAP397"},
	{398, "MAP398"},
	{399, "MAP399"},
	{400, "MAP400"},
	{401, "MAP401"},
	{402, "MAP402"},
	{403, "MAP403"},
	{404, "MAP404"},
	{405, "MAP405"},
	{406, "MAP406"},
	{407, "MAP407"},
	{408, "MAP408"},
	{409, "MAP409"},
	{410, "MAP410"},
	{411, "MAP411"},
	{412, "MAP412"},
	{413, "MAP413"},
	{414, "MAP414"},
	{415, "MAP415"},
	{416, "MAP416"},
	{417, "MAP417"},
	{418, "MAP418"},
	{419, "MAP419"},
	{420, "MAP420"},
	{421, "MAP421"},
	{422, "MAP422"},
	{423, "MAP423"},
	{424, "MAP424"},
	{425, "MAP425"},
	{426, "MAP426"},
	{427, "MAP427"},
	{428, "MAP428"},
	{429, "MAP429"},
	{430, "MAP430"},
	{431, "MAP431"},
	{432, "MAP432"},
	{433, "MAP433"},
	{434, "MAP434"},
	{435, "MAP435"},
	{436, "MAP436"},
	{437, "MAP437"},
	{438, "MAP438"},
	{439, "MAP439"},
	{440, "MAP440"},
	{441, "MAP441"},
	{442, "MAP442"},
	{443, "MAP443"},
	{444, "MAP444"},
	{445, "MAP445"},
	{446, "MAP446"},
	{447, "MAP447"},
	{448, "MAP448"},
	{449, "MAP449"},
	{450, "MAP450"},
	{451, "MAP451"},
	{452, "MAP452"},
	{453, "MAP453"},
	{454, "MAP454"},
	{455, "MAP455"},
	{456, "MAP456"},
	{457, "MAP457"},
	{458, "MAP458"},
	{459, "MAP459"},
	{460, "MAP460"},
	{461, "MAP461"},
	{462, "MAP462"},
	{463, "MAP463"},
	{464, "MAP464"},
	{465, "MAP465"},
	{466, "MAP466"},
	{467, "MAP467"},
	{468, "MAP468"},
	{469, "MAP469"},
	{470, "MAP470"},
	{471, "MAP471"},
	{472, "MAP472"},
	{473, "MAP473"},
	{474, "MAP474"},
	{475, "MAP475"},
	{476, "MAP476"},
	{477, "MAP477"},
	{478, "MAP478"},
	{479, "MAP479"},
	{480, "MAP480"},
	{481, "MAP481"},
	{482, "MAP482"},
	{483, "MAP483"},
	{484, "MAP484"},
	{485, "MAP485"},
	{486, "MAP486"},
	{487, "MAP487"},
	{488, "MAP488"},
	{489, "MAP489"},
	{490, "MAP490"},
	{491, "MAP491"},
	{492, "MAP492"},
	{493, "MAP493"},
	{494, "MAP494"},
	{495, "MAP495"},
	{496, "MAP496"},
	{497, "MAP497"},
	{498, "MAP498"},
	{499, "MAP499"},
	{500, "MAP500"},
	{501, "MAP501"},
	{502, "MAP502"},
	{503, "MAP503"},
	{504, "MAP504"},
	{505, "MAP505"},
	{506, "MAP506"},
	{507, "MAP507"},
	{508, "MAP508"},
	{509, "MAP509"},
	{510, "MAP510"},
	{511, "MAP511"},
	{512, "MAP512"},
	{513, "MAP513"},
	{514, "MAP514"},
	{515, "MAP515"},
	{516, "MAP516"},
	{517, "MAP517"},
	{518, "MAP518"},
	{519, "MAP519"},
	{520, "MAP520"},
	{521, "MAP521"},
	{522, "MAP522"},
	{523, "MAP523"},
	{524, "MAP524"},
	{525, "MAP525"},
	{526, "MAP526"},
	{527, "MAP527"},
	{528, "MAP528"},
	{529, "MAP529"},
	{530, "MAP530"},
	{531, "MAP531"},
	{532, "MAP532"},
	{533, "MAP533"},
	{534, "MAP534"},
	{535, "MAP535"},
	{536, "MAP536"},
	{537, "MAP537"},
	{538, "MAP538"},
	{539, "MAP539"},
	{540, "MAP540"},
	{541, "MAP541"},
	{542, "MAP542"},
	{543, "MAP543"},
	{544, "MAP544"},
	{545, "MAP545"},
	{546, "MAP546"},
	{547, "MAP547"},
	{548, "MAP548"},
	{549, "MAP549"},
	{550, "MAP550"},
	{551, "MAP551"},
	{552, "MAP552"},
	{553, "MAP553"},
	{554, "MAP554"},
	{555, "MAP555"},
	{556, "MAP556"},
	{557, "MAP557"},
	{558, "MAP558"},
	{559, "MAP559"},
	{560, "MAP560"},
	{561, "MAP561"},
	{562, "MAP562"},
	{563, "MAP563"},
	{564, "MAP564"},
	{565, "MAP565"},
	{566, "MAP566"},
	{567, "MAP567"},
	{568, "MAP568"},
	{569, "MAP569"},
	{570, "MAP570"},
	{571, "MAP571"},
	{572, "MAP572"},
	{573, "MAP573"},
	{574, "MAP574"},
	{575, "MAP575"},
	{576, "MAP576"},
	{577, "MAP577"},
	{578, "MAP578"},
	{579, "MAP579"},
	{580, "MAP580"},
	{581, "MAP581"},
	{582, "MAP582"},
	{583, "MAP583"},
	{584, "MAP584"},
	{585, "MAP585"},
	{586, "MAP586"},
	{587, "MAP587"},
	{588, "MAP588"},
	{589, "MAP589"},
	{590, "MAP590"},
	{591, "MAP591"},
	{592, "MAP592"},
	{593, "MAP593"},
	{594, "MAP594"},
	{595, "MAP595"},
	{596, "MAP596"},
	{597, "MAP597"},
	{598, "MAP598"},
	{599, "MAP599"},
	{600, "MAP600"},
	{601, "MAP601"},
	{602, "MAP602"},
	{603, "MAP603"},
	{604, "MAP604"},
	{605, "MAP605"},
	{606, "MAP606"},
	{607, "MAP607"},
	{608, "MAP608"},
	{609, "MAP609"},
	{610, "MAP610"},
	{611, "MAP611"},
	{612, "MAP612"},
	{613, "MAP613"},
	{614, "MAP614"},
	{615, "MAP615"},
	{616, "MAP616"},
	{617, "MAP617"},
	{618, "MAP618"},
	{619, "MAP619"},
	{620, "MAP620"},
	{621, "MAP621"},
	{622, "MAP622"},
	{623, "MAP623"},
	{624, "MAP624"},
	{625, "MAP625"},
	{626, "MAP626"},
	{627, "MAP627"},
	{628, "MAP628"},
	{629, "MAP629"},
	{630, "MAP630"},
	{631, "MAP631"},
	{632, "MAP632"},
	{633, "MAP633"},
	{634, "MAP634"},
	{635, "MAP635"},
	{636, "MAP636"},
	{637, "MAP637"},
	{638, "MAP638"},
	{639, "MAP639"},
	{640, "MAP640"},
	{641, "MAP641"},
	{642, "MAP642"},
	{643, "MAP643"},
	{644, "MAP644"},
	{645, "MAP645"},
	{646, "MAP646"},
	{647, "MAP647"},
	{648, "MAP648"},
	{649, "MAP649"},
	{650, "MAP650"},
	{651, "MAP651"},
	{652, "MAP652"},
	{653, "MAP653"},
	{654, "MAP654"},
	{655, "MAP655"},
	{656, "MAP656"},
	{657, "MAP657"},
	{658, "MAP658"},
	{659, "MAP659"},
	{660, "MAP660"},
	{661, "MAP661"},
	{662, "MAP662"},
	{663, "MAP663"},
	{664, "MAP664"},
	{665, "MAP665"},
	{666, "MAP666"},
	{667, "MAP667"},
	{668, "MAP668"},
	{669, "MAP669"},
	{670, "MAP670"},
	{671, "MAP671"},
	{672, "MAP672"},
	{673, "MAP673"},
	{674, "MAP674"},
	{675, "MAP675"},
	{676, "MAP676"},
	{677, "MAP677"},
	{678, "MAP678"},
	{679, "MAP679"},
	{680, "MAP680"},
	{681, "MAP681"},
	{682, "MAP682"},
	{683, "MAP683"},
	{684, "MAP684"},
	{685, "MAP685"},
	{686, "MAP686"},
	{687, "MAP687"},
	{688, "MAP688"},
	{689, "MAP689"},
	{690, "MAP690"},
	{691, "MAP691"},
	{692, "MAP692"},
	{693, "MAP693"},
	{694, "MAP694"},
	{695, "MAP695"},
	{696, "MAP696"},
	{697, "MAP697"},
	{698, "MAP698"},
	{699, "MAP699"},
	{700, "MAP700"},
	{701, "MAP701"},
	{702, "MAP702"},
	{703, "MAP703"},
	{704, "MAP704"},
	{705, "MAP705"},
	{706, "MAP706"},
	{707, "MAP707"},
	{708, "MAP708"},
	{709, "MAP709"},
	{710, "MAP710"},
	{711, "MAP711"},
	{712, "MAP712"},
	{713, "MAP713"},
	{714, "MAP714"},
	{715, "MAP715"},
	{716, "MAP716"},
	{717, "MAP717"},
	{718, "MAP718"},
	{719, "MAP719"},
	{720, "MAP720"},
	{721, "MAP721"},
	{722, "MAP722"},
	{723, "MAP723"},
	{724, "MAP724"},
	{725, "MAP725"},
	{726, "MAP726"},
	{727, "MAP727"},
	{728, "MAP728"},
	{729, "MAP729"},
	{730, "MAP730"},
	{731, "MAP731"},
	{732, "MAP732"},
	{733, "MAP733"},
	{734, "MAP734"},
	{735, "MAP735"},
	{736, "MAP736"},
	{737, "MAP737"},
	{738, "MAP738"},
	{739, "MAP739"},
	{740, "MAP740"},
	{741, "MAP741"},
	{742, "MAP742"},
	{743, "MAP743"},
	{744, "MAP744"},
	{745, "MAP745"},
	{746, "MAP746"},
	{747, "MAP747"},
	{748, "MAP748"},
	{749, "MAP749"},
	{750, "MAP750"},
	{751, "MAP751"},
	{752, "MAP752"},
	{753, "MAP753"},
	{754, "MAP754"},
	{755, "MAP755"},
	{756, "MAP756"},
	{757, "MAP757"},
	{758, "MAP758"},
	{759, "MAP759"},
	{760, "MAP760"},
	{761, "MAP761"},
	{762, "MAP762"},
	{763, "MAP763"},
	{764, "MAP764"},
	{765, "MAP765"},
	{766, "MAP766"},
	{767, "MAP767"},
	{768, "MAP768"},
	{769, "MAP769"},
	{770, "MAP770"},
	{771, "MAP771"},
	{772, "MAP772"},
	{773, "MAP773"},
	{774, "MAP774"},
	{775, "MAP775"},
	{776, "MAP776"},
	{777, "MAP777"},
	{778, "MAP778"},
	{779, "MAP779"},
	{780, "MAP780"},
	{781, "MAP781"},
	{782, "MAP782"},
	{783, "MAP783"},
	{784, "MAP784"},
	{785, "MAP785"},
	{786, "MAP786"},
	{787, "MAP787"},
	{788, "MAP788"},
	{789, "MAP789"},
	{790, "MAP790"},
	{791, "MAP791"},
	{792, "MAP792"},
	{793, "MAP793"},
	{794, "MAP794"},
	{795, "MAP795"},
	{796, "MAP796"},
	{797, "MAP797"},
	{798, "MAP798"},
	{799, "MAP799"},
	{800, "MAP800"},
	{801, "MAP801"},
	{802, "MAP802"},
	{803, "MAP803"},
	{804, "MAP804"},
	{805, "MAP805"},
	{806, "MAP806"},
	{807, "MAP807"},
	{808, "MAP808"},
	{809, "MAP809"},
	{810, "MAP810"},
	{811, "MAP811"},
	{812, "MAP812"},
	{813, "MAP813"},
	{814, "MAP814"},
	{815, "MAP815"},
	{816, "MAP816"},
	{817, "MAP817"},
	{818, "MAP818"},
	{819, "MAP819"},
	{820, "MAP820"},
	{821, "MAP821"},
	{822, "MAP822"},
	{823, "MAP823"},
	{824, "MAP824"},
	{825, "MAP825"},
	{826, "MAP826"},
	{827, "MAP827"},
	{828, "MAP828"},
	{829, "MAP829"},
	{830, "MAP830"},
	{831, "MAP831"},
	{832, "MAP832"},
	{833, "MAP833"},
	{834, "MAP834"},
	{835, "MAP835"},
	{836, "MAP836"},
	{837, "MAP837"},
	{838, "MAP838"},
	{839, "MAP839"},
	{840, "MAP840"},
	{841, "MAP841"},
	{842, "MAP842"},
	{843, "MAP843"},
	{844, "MAP844"},
	{845, "MAP845"},
	{846, "MAP846"},
	{847, "MAP847"},
	{848, "MAP848"},
	{849, "MAP849"},
	{850, "MAP850"},
	{851, "MAP851"},
	{852, "MAP852"},
	{853, "MAP853"},
	{854, "MAP854"},
	{855, "MAP855"},
	{856, "MAP856"},
	{857, "MAP857"},
	{858, "MAP858"},
	{859, "MAP859"},
	{860, "MAP860"},
	{861, "MAP861"},
	{862, "MAP862"},
	{863, "MAP863"},
	{864, "MAP864"},
	{865, "MAP865"},
	{866, "MAP866"},
	{867, "MAP867"},
	{868, "MAP868"},
	{869, "MAP869"},
	{870, "MAP870"},
	{871, "MAP871"},
	{872, "MAP872"},
	{873, "MAP873"},
	{874, "MAP874"},
	{875, "MAP875"},
	{876, "MAP876"},
	{877, "MAP877"},
	{878, "MAP878"},
	{879, "MAP879"},
	{880, "MAP880"},
	{881, "MAP881"},
	{882, "MAP882"},
	{883, "MAP883"},
	{884, "MAP884"},
	{885, "MAP885"},
	{886, "MAP886"},
	{887, "MAP887"},
	{888, "MAP888"},
	{889, "MAP889"},
	{890, "MAP890"},
	{891, "MAP891"},
	{892, "MAP892"},
	{893, "MAP893"},
	{894, "MAP894"},
	{895, "MAP895"},
	{896, "MAP896"},
	{897, "MAP897"},
	{898, "MAP898"},
	{899, "MAP899"},
	{900, "MAP900"},
	{901, "MAP901"},
	{902, "MAP902"},
	{903, "MAP903"},
	{904, "MAP904"},
	{905, "MAP905"},
	{906, "MAP906"},
	{907, "MAP907"},
	{908, "MAP908"},
	{909, "MAP909"},
	{910, "MAP910"},
	{911, "MAP911"},
	{912, "MAP912"},
	{913, "MAP913"},
	{914, "MAP914"},
	{915, "MAP915"},
	{916, "MAP916"},
	{917, "MAP917"},
	{918, "MAP918"},
	{919, "MAP919"},
	{920, "MAP920"},
	{921, "MAP921"},
	{922, "MAP922"},
	{923, "MAP923"},
	{924, "MAP924"},
	{925, "MAP925"},
	{926, "MAP926"},
	{927, "MAP927"},
	{928, "MAP928"},
	{929, "MAP929"},
	{930, "MAP930"},
	{931, "MAP931"},
	{932, "MAP932"},
	{933, "MAP933"},
	{934, "MAP934"},
	{935, "MAP935"},
	{936, "MAP936"},
	{937, "MAP937"},
	{938, "MAP938"},
	{939, "MAP939"},
	{940, "MAP940"},
	{941, "MAP941"},
	{942, "MAP942"},
	{943, "MAP943"},
	{944, "MAP944"},
	{945, "MAP945"},
	{946, "MAP946"},
	{947, "MAP947"},
	{948, "MAP948"},
	{949, "MAP949"},
	{950, "MAP950"},
	{951, "MAP951"},
	{952, "MAP952"},
	{953, "MAP953"},
	{954, "MAP954"},
	{955, "MAP955"},
	{956, "MAP956"},
	{957, "MAP957"},
	{958, "MAP958"},
	{959, "MAP959"},
	{960, "MAP960"},
	{961, "MAP961"},
	{962, "MAP962"},
	{963, "MAP963"},
	{964, "MAP964"},
	{965, "MAP965"},
	{966, "MAP966"},
	{967, "MAP967"},
	{968, "MAP968"},
	{969, "MAP969"},
	{970, "MAP970"},
	{971, "MAP971"},
	{972, "MAP972"},
	{973, "MAP973"},
	{974, "MAP974"},
	{975, "MAP975"},
	{976, "MAP976"},
	{977, "MAP977"},
	{978, "MAP978"},
	{979, "MAP979"},
	{980, "MAP980"},
	{981, "MAP981"},
	{982, "MAP982"},
	{983, "MAP983"},
	{984, "MAP984"},
	{985, "MAP985"},
	{986, "MAP986"},
	{987, "MAP987"},
	{988, "MAP988"},
	{989, "MAP989"},
	{990, "MAP990"},
	{991, "MAP991"},
	{992, "MAP992"},
	{993, "MAP993"},
	{994, "MAP994"},
	{995, "MAP995"},
	{996, "MAP996"},
	{997, "MAP997"},
	{998, "MAP998"},
	{999, "MAP999"},
	{1000, "MAP1000"},
	{1001, "MAP1001"},
	{1002, "MAP1002"},
	{1003, "MAP1003"},
	{1004, "MAP1004"},
	{1005, "MAP1005"},
	{1006, "MAP1006"},
	{1007, "MAP1007"},
	{1008, "MAP1008"},
	{1009, "MAP1009"},
	{1010, "MAP1010"},
	{1011, "MAP1011"},
	{1012, "MAP1012"},
	{1013, "MAP1013"},
	{1014, "MAP1014"},
	{1015, "MAP1015"},
	{1016, "MAP1016"},
	{1017, "MAP1017"},
	{1018, "MAP1018"},
	{1019, "MAP1019"},
	{1020, "MAP1020"},
	{1021, "MAP1021"},
	{1022, "MAP1022"},
	{1023, "MAP1023"},
	{1024, "MAP1024"},
	{1025, "MAP1025"},
	{1026, "MAP1026"},
	{1027, "MAP1027"},
	{1028, "MAP1028"},
	{1029, "MAP1029"},
	{1030, "MAP1030"},
	{1031, "MAP1031"},
	{1032, "MAP1032"},
	{1033, "MAP1033"},
	{1034, "MAP1034"},
	{1035, "MAP1035"},
	{INT16_MAX, NULL}
};

static void Newgametype_OnChange(void);
static void Nextmap_OnChange(void);

// GT_* defined in doomstat.h
consvar_t cv_nextmap = {"nextmap", "MAP01", CV_HIDEN|CV_CALL, map_cons_t, Nextmap_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t skins_cons_t[MAXSKINS+1] = {{1, DEFAULTSKIN}};
consvar_t cv_chooseskin = {"chooseskin", DEFAULTSKIN, CV_HIDEN|CV_CALL, skins_cons_t, Nextmap_OnChange, 0, NULL, NULL, 0, 0, NULL};

// When you add gametypes here, don't forget
// to update them in CV_AddValue!

CV_PossibleValue_t gametype_cons_t[] =
{
	{GT_COOP, "Coop"}, {GTF_CLASSICRACE, "Competition"},
	{GT_MATCH, "Match"}, {GTF_TEAMMATCH, "Team Match"},
	{GT_RACE, "Race"},
	{GT_TAG, "Tag"}, {GTF_HIDEANDSEEK, "Hide and Seek"},
	{GT_CTF, "CTF"},
#ifdef CHAOSISNOTDEADYET
	{GT_CHAOS, "Chaos"},
#endif
	{0, NULL}
};

//
// FindFirstMap
//
// Finds the first map of a particular gametype
// Defaults to 1 if nothing found.
//
static INT32 FindFirstMap(INT32 gtype)
{
	INT32 i;

	for (i = 0; i < NUMMAPS; i++)
	{
		if (mapheaderinfo[i] && (mapheaderinfo[i]->typeoflevel & gtype))
			return i + 1;
	}

	return 1;
}

consvar_t cv_newgametype = {"newgametype", "Coop", CV_HIDEN|CV_CALL, gametype_cons_t, Newgametype_OnChange, 0, NULL, NULL, 0, 0, NULL};
boolean StartSplitScreenGame;

static void Newgametype_OnChange(void)
{
	if (menuactive)
	{
		if(!mapheaderinfo[cv_nextmap.value-1])
			P_AllocMapHeader((INT16)(cv_nextmap.value-1));

		if ((cv_newgametype.value == GT_COOP && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_COOP)) ||
			((cv_newgametype.value == GT_RACE || cv_newgametype.value == GTF_CLASSICRACE) && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_RACE)) ||
			((cv_newgametype.value == GT_MATCH || cv_newgametype.value == GTF_TEAMMATCH) && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_MATCH)) ||
#ifdef CHAOSISNOTDEADYET
			(cv_newgametype.value == GT_CHAOS && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_CHAOS)) ||
#endif
			((cv_newgametype.value == GT_TAG || cv_newgametype.value == GTF_HIDEANDSEEK) && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_TAG)) ||
			(cv_newgametype.value == GT_CTF && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_CTF)))
		{
			INT32 value = 0;

			switch (cv_newgametype.value)
			{
				case GT_COOP:
					value = TOL_COOP;
					break;
				case GT_RACE:
					value = TOL_RACE;
					break;
				case GT_MATCH:
					value = TOL_MATCH;
					break;
				case GT_TAG:
					value = TOL_TAG;
					break;
				case GT_CTF:
					value = TOL_CTF;
					break;
			}

			CV_SetValue(&cv_nextmap, FindFirstMap(value));
			CV_AddValue(&cv_nextmap, -1);
			CV_AddValue(&cv_nextmap, 1);
		}
	}
}

static void M_ChangeLevel(INT32 choice)
{
	char mapname[6];
	(void)choice;

	strlcpy(mapname, G_BuildMapName(cv_nextmap.value), sizeof (mapname));
	strlwr(mapname);
	mapname[5] = '\0';

	M_ClearMenus(true);
	COM_BufAddText(va("map %s -gametype \"%s\"\n", mapname, cv_newgametype.string));
}

static void M_ConfirmSpectate(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);
	COM_ImmedExecute("changeteam spectator");
}

static void M_ConfirmTeamScramble(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);

	switch (cv_dummyscramble.value)
	{
		case 0:
			COM_ImmedExecute("teamscramble 1");
			break;
		case 1:
			COM_ImmedExecute("teamscramble 2");
			break;
	}
}

static void M_ConfirmTeamChange(INT32 choice)
{
	(void)choice;
	if (!cv_allowteamchange.value && cv_dummyteam.value)
	{
		M_StartMessage("The server is not allowing\n team changes at this time.\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	M_ClearMenus(true);

	switch (cv_dummyteam.value)
	{
		case 0:
			COM_ImmedExecute("changeteam spectator");
			break;
		case 1:
			COM_ImmedExecute("changeteam red");
			break;
		case 2:
			COM_ImmedExecute("changeteam blue");
			break;
	}
}

static void M_StartServer(INT32 choice)
{
	(void)choice;
	if (!StartSplitScreenGame)
		netgame = true;

	multiplayer = true;

	// Special Cases
	if (cv_newgametype.value == GTF_TEAMMATCH)
		CV_SetValue(&cv_matchtype, 1);
	else if (cv_newgametype.value == GTF_CLASSICRACE)
		CV_SetValue(&cv_racetype, 1);
	else if (cv_newgametype.value == GTF_HIDEANDSEEK)
		CV_SetValue(&cv_tagtype, 1);
	else if (cv_newgametype.value == GT_MATCH)
		CV_SetValue(&cv_matchtype, 0);
	else if (cv_newgametype.value == GT_RACE)
		CV_SetValue(&cv_racetype, 0);
	else if (cv_newgametype.value == GT_TAG)
		CV_SetValue(&cv_tagtype, 0);

	// Just in case you were in devmode before starting the server.
	if (!cv_cheats.value)
		CV_ResetCheatNetVars();

	if (!StartSplitScreenGame)
	{
		if (demoplayback)
			COM_BufAddText("stopdemo\n");
		D_MapChange(cv_nextmap.value, cv_newgametype.value, false, 1, 1, false, false);
		COM_BufAddText("dummyconsvar 1\n");
	}
	else // split screen
	{
		paused = false;
		if (demoplayback)
			COM_BufAddText("stopdemo\n");

		SV_StartSinglePlayerServer();
		if (!splitscreen)
		{
			splitscreen = true;
			SplitScreen_OnChange();
		}
		D_MapChange(cv_nextmap.value, cv_newgametype.value, false, 1, 1, false, false);
	}

	M_ClearMenus(true);
}

static void M_DrawServerMenu(void)
{
	lumpnum_t lumpnum;
	patch_t *PictureOfLevel;

	M_DrawGenericMenu();

	//  A 160x100 image of the level as entry MAPxxP
	lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(cv_nextmap.value)));

	if (lumpnum != LUMPERROR)
		PictureOfLevel = W_CachePatchName(va("%sP", G_BuildMapName(cv_nextmap.value)), PU_CACHE);
	else
		PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);

	V_DrawSmallScaledPatch((BASEVIDWIDTH*3/4)-(SHORT(PictureOfLevel->width)/4), ((BASEVIDHEIGHT*3/4)-(SHORT(PictureOfLevel->height)/4)+10), 0, PictureOfLevel);
}

static menuitem_t ServerMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Game Type",             &cv_newgametype,    10},
	{IT_STRING|IT_CVAR,              NULL, "Advertise on Internet", &cv_internetserver, 20},
#ifndef NONET
	{IT_STRING|IT_CVAR,              NULL, "Room",					&cv_chooseroom,		30},
	{IT_STRING|IT_CALL,              NULL, "Room Info",             M_RoomInfoMenu,     40},
#endif
	{IT_STRING|IT_CVAR|IT_CV_STRING, NULL, "Server Name",           &cv_servername,     50},

	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,        80},

	{IT_WHITESTRING|IT_CALL,         NULL, "Start",                 M_StartServer,     130},
};

menu_t Serverdef =
{
	0,
	"Start Server",
	sizeof (ServerMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	ServerMenu,
	M_DrawServerMenu,
	27,40,
	0,
	NULL
};

static menuitem_t ChangeLevelMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Game Type",             &cv_newgametype,    30},

	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,        60},


	{IT_WHITESTRING|IT_CALL,         NULL, "Change Level",                 M_ChangeLevel,     120},
};

menu_t ChangeLevelDef =
{
	0,
	"Change Level",
	sizeof (ChangeLevelMenu)/sizeof (menuitem_t),
	&MainDef,
	ChangeLevelMenu,
	M_DrawServerMenu,
	27,40,
	0,
	NULL
};

static menuitem_t ChangeTeamMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Select Team",             &cv_dummyteam,    30},

	{IT_WHITESTRING|IT_CALL,         NULL, "Confirm",           M_ConfirmTeamChange,      90},
};

menu_t ChangeTeamDef =
{
	0,
	"Change Team",
	sizeof (ChangeTeamMenu)/sizeof (menuitem_t),
	&MainDef,
	ChangeTeamMenu,
	M_DrawGenericMenu,
	27,40,
	0,
	NULL
};

static menuitem_t TeamScrambleMenu[] =
{
	{IT_STRING|IT_CVAR,      NULL, "Scramble Method", &cv_dummyscramble,     30},

	{IT_WHITESTRING|IT_CALL, NULL, "Confirm",         M_ConfirmTeamScramble, 90},
};

menu_t TeamScrambleDef =
{
	0,
	"Scramble Teams",
	sizeof (ChangeTeamMenu)/sizeof (menuitem_t),
	&MainDef,
	TeamScrambleMenu,
	M_DrawGenericMenu,
	27,40,
	0,
	NULL
};

//
// M_PatchLevelNameTable
//
// Populates the cv_nextmap variable
//
// Modes:
// 0 = Create Server Menu
// 1 = Level Select Menu
// 2 = Time Attack Menu
// 3 = SRB1 Level Select Menu
//
static boolean M_PatchLevelNameTable(INT32 mode)
{
	size_t i;
	INT32 j;
	INT32 currentmap;
	boolean foundone = false;

	for (j = 0; j < LEVELARRAYSIZE-2; j++)
	{
		i = 0;
		currentmap = map_cons_t[j].value-1;

		if (mapheaderinfo[currentmap] && mapheaderinfo[currentmap]->lvlttl[0] && ((mode == 0 && !mapheaderinfo[currentmap]->hideinmenu && !((mapheaderinfo[currentmap]->typeoflevel & TOL_SRB1) && !(grade & 2))) || (mode == 1 && mapheaderinfo[currentmap]->levelselect && !(mapheaderinfo[currentmap]->typeoflevel & TOL_SRB1)) || (mode == 2 && mapheaderinfo[currentmap]->timeattack && mapvisited[currentmap]) || (mode == 3 && mapheaderinfo[currentmap]->levelselect && (mapheaderinfo[currentmap]->typeoflevel & TOL_SRB1))))
		{
			strlcpy(lvltable[j], mapheaderinfo[currentmap]->lvlttl, sizeof (lvltable[j]));

			i += strlen(mapheaderinfo[currentmap]->lvlttl);

			if (!mapheaderinfo[currentmap]->nozone)
			{
				lvltable[j][i++] = ' ';
				lvltable[j][i++] = 'Z';
				lvltable[j][i++] = 'O';
				lvltable[j][i++] = 'N';
				lvltable[j][i++] = 'E';
			}

			if (mapheaderinfo[currentmap]->actnum)
			{
				char actnum[3];
				INT32 g;

				lvltable[j][i++] = ' ';

				sprintf(actnum, "%d", mapheaderinfo[currentmap]->actnum);

				for (g = 0; g < 3; g++)
				{
					if (actnum[g] == '\0')
						break;

					lvltable[j][i++] = actnum[g];
				}
			}

			lvltable[j][i++] = '\0';
			foundone = true;
		}
		else
			lvltable[j][0] = '\0';

		if (lvltable[j][0] == '\0')
			map_cons_t[j].strvalue = NULL;
		else
			map_cons_t[j].strvalue = lvltable[j];
	}

	if (!foundone)
		return false;

	CV_SetValue(&cv_nextmap, cv_nextmap.value); // This causes crash sometimes?!

	if (mode > 0)
	{
		INT32 value = 0;

		switch (cv_newgametype.value)
		{
			case GT_COOP:
				value = TOL_COOP;
				break;
			case GT_RACE:
				value = TOL_RACE;
				break;
			case GT_MATCH:
				value = TOL_MATCH;
				break;
			case GT_TAG:
				value = TOL_TAG;
				break;
			case GT_CTF:
				value = TOL_CTF;
				break;
		}

		CV_SetValue(&cv_nextmap, FindFirstMap(value));
		CV_AddValue(&cv_nextmap, -1);
		CV_AddValue(&cv_nextmap, 1);
	}
	else
		Newgametype_OnChange(); // Make sure to start on an appropriate map if wads have been added

	return true;
}

static void M_MapChange(INT32 choice)
{
	(void)choice;
	if (!(netgame || multiplayer) || !Playing())
	{
		M_StartMessage("You aren't in a game!\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	inlevelselect = 0;
	M_PatchLevelNameTable(0);

	// Special Cases
	if (gametype == GT_MATCH && cv_matchtype.value) // Team Match
		CV_SetValue(&cv_newgametype, GTF_TEAMMATCH);
	else if (gametype == GT_RACE && cv_racetype.value) // Time-Only Race
		CV_SetValue(&cv_newgametype, GTF_CLASSICRACE);
	else if (gametype == GT_TAG && cv_tagtype.value) // Hide and Seek Mode
		CV_SetValue(&cv_newgametype, GTF_HIDEANDSEEK);
	else
		CV_SetValue(&cv_newgametype, gametype);

	CV_SetValue(&cv_nextmap, gamemap);

	M_SetupNextMenu(&ChangeLevelDef);
}

static void M_TeamChange(INT32 choice)
{
	(void)choice;
	if (!(netgame || multiplayer) || !Playing())
	{
		M_StartMessage("You aren't in a game!\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&ChangeTeamDef);
}

static void M_TeamScramble(INT32 choice)
{
	(void)choice;
	if (!(netgame || multiplayer) || !Playing())
	{
		M_StartMessage("You aren't in a game!\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	if (!server && !adminplayer)
	{
		M_StartMessage("Only the server may use this command.\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&TeamScrambleDef);
}

//
// M_PatchSkinNameTable
//
// Like M_PatchLevelNameTable, but for cv_chooseskin
//
static void M_PatchSkinNameTable(void)
{
	INT32 j;

	memset(skins_cons_t, 0, sizeof (skins_cons_t));

	for (j = 0; j < MAXSKINS; j++)
	{
		if (skins[j].name[0] != '\0')
		{
			skins_cons_t[j].strvalue = skins[j].name;
			skins_cons_t[j].value = j+1;
		}
		else
		{
			skins_cons_t[j].strvalue = NULL;
			skins_cons_t[j].value = 0;
		}
	}

	CV_SetValue(&cv_chooseskin, cv_chooseskin.value); // This causes crash sometimes?!

	CV_SetValue(&cv_chooseskin, 1);
	CV_AddValue(&cv_chooseskin, -1);
	CV_AddValue(&cv_chooseskin, 1);

	return;
}

static inline void M_StartSplitServerMenu(void)
{
	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING, M_ExitGameResponse, MM_YESNO);
		return;
	}

	inlevelselect = 0;
	M_PatchLevelNameTable(0);
	StartSplitScreenGame = true;
	ServerMenu[1].status = IT_DISABLED; // No advertise on Internet option.
	ServerMenu[2].status = IT_DISABLED; // No room.
	ServerMenu[3].status = IT_DISABLED; // No room info.
	ServerMenu[4].status = IT_DISABLED; // No server name.
	M_SetupNextMenu(&Serverdef);
}

#ifndef NONET
static void M_StartServerMenu(INT32 choice)
{
	(void)choice;
	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING, M_ExitGameResponse, MM_YESNO);
		return;
	}

	inlevelselect = 0;
	M_PatchLevelNameTable(0);
	StartSplitScreenGame = false;
	ServerMenu[1].status = IT_STRING|IT_CVAR; // Make advertise on Internet option available.
	M_AlterRoomOptions();
	ServerMenu[4].status = IT_STRING|IT_CVAR|IT_CV_STRING; // Server name too.
	M_SetupNextMenu(&Serverdef);

}

void M_AlterRoomOptions(void)
{
	if (cv_internetserver.value)
	{
		ServerMenu[2].status = IT_STRING|IT_CVAR; // Make room option available.
		ServerMenu[3].status = IT_STRING|IT_CALL; // Make room info option available.
#ifdef UPDATE_ALERT
		if(M_CheckMODVersion())
		{
#endif
			if(!M_PatchRoomsTable(true))
			{
				ServerMenu[2].status = IT_DISABLED; // Make room option unavailable.
				ServerMenu[3].status = IT_DISABLED; // Same for Room info.
				CV_SetValue(&cv_internetserver, 0);
				return;
			}
#ifdef UPDATE_ALERT
		} else {
			ServerMenu[2].status = IT_DISABLED; // Make room option unavailable.
			ServerMenu[3].status = IT_DISABLED; // Same for Room info.
			CV_SetValue(&cv_internetserver, 0);
		}
#endif
	}
	else
	{
		ServerMenu[2].status = IT_DISABLED; // No room.
		ServerMenu[3].status = IT_DISABLED; // Same for Room info.
	}
}
#endif

//===========================================================================
//                            MULTI PLAYER MENU
//===========================================================================
static void M_SetupMultiPlayer(INT32 choice);
static void M_SetupMultiPlayerBis(INT32 choice);
static void M_Splitscreen(INT32 choice);

typedef enum
{
#ifdef NONET
	startsplitscreengame = 0,
#else
	startserver = 0,
	connectmultiplayermenu,
	connectlanmenu,
	connectip,
	startsplitscreengame,
#endif
	multiplayeroptions,
	setupplayer1,
	setupplayer2,
	end_game,
	multiplayer_end
} multiplayer_e;

static menuitem_t MultiPlayerMenu[] =
{
#ifndef NONET
	{IT_CALL | IT_STRING, NULL, "HOST GAME",              M_StartServerMenu,      10},
	{IT_CALL | IT_STRING, NULL, "JOIN GAME (Internet)",	  M_ConnectMenu,		  20},
	{IT_CALL | IT_STRING, NULL, "JOIN GAME (LAN)",		  M_ConnectLANMenu,       30},
	{IT_CALL | IT_STRING, NULL, "JOIN GAME (Specify IP)", M_ConnectIPMenu,        40},
#endif
	{IT_CALL | IT_STRING, NULL, "TWO PLAYER GAME",        M_Splitscreen,          60},
	{IT_CALL | IT_STRING, NULL, "NETWORK OPTIONS",        M_NetOption,            80},
	{IT_CALL | IT_STRING, NULL, "SETUP PLAYER",           M_SetupMultiPlayer,     90},
	{IT_CALL | IT_STRING | IT_DISABLED, NULL, "SETUP PLAYER 2",         M_SetupMultiPlayerBis, 100},
	{IT_CALL | IT_STRING, NULL, "END GAME",               M_EndGame,             120},
};

menu_t  MultiPlayerDef =
{
	"M_MULTI",
	"Multiplayer",
	multiplayer_end,
	&MainDef,
	MultiPlayerMenu,
	M_DrawGenericMenu,
	85,40,
	0,
	NULL
};

static void M_Splitscreen(INT32 choice)
{
	(void)choice;
	M_StartSplitServerMenu();
}

//===========================================================================
// Seconde mouse config for the splitscreen player
//===========================================================================

static menuitem_t  SecondMouseCfgMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Second Mouse Serial Port",
	                                                &cv_mouse2port,      0}, // Tails 01-18-2001

	{IT_STRING | IT_CVAR, NULL, "Use Mouse 2",      &cv_usemouse2,       0},

	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse2 Speed",     &cv_mousesens2,      0},

	{IT_STRING | IT_CVAR, NULL, "Always MouseLook", &cv_alwaysfreelook2, 0},
	{IT_STRING | IT_CVAR, NULL, "Mouse Move",       &cv_mousemove2,      0},
	{IT_STRING | IT_CVAR, NULL, "Invert Mouse2",    &cv_invertmouse2,    0},

	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mlook Speed",      &cv_mlooksens2,      0},
};

menu_t SecondMouseCfgdef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (SecondMouseCfgMenu)/sizeof (menuitem_t),
	&SetupMultiPlayerDef,
	SecondMouseCfgMenu,
	M_DrawGenericMenu,
	27, 40,
	0,
	NULL
};

//===========================================================================
//MULTI PLAYER SETUP MENU
//===========================================================================
static void M_DrawSetupMultiPlayerMenu(void);
static void M_HandleSetupMultiPlayer(INT32 choice);
static void M_Setup1PControlsMenu(INT32 choice);
static void M_Setup2PControlsMenu(INT32 choice);
static boolean M_QuitMultiPlayerMenu(void);

static menuitem_t SetupMultiPlayerMenu[] =
{
	{IT_KEYHANDLER | IT_STRING,   NULL, "Your name",   M_HandleSetupMultiPlayer,   0},

	{IT_KEYHANDLER | IT_STRING,   NULL, "Your color",  M_HandleSetupMultiPlayer,  16},

	{IT_KEYHANDLER | IT_STRING,   NULL, "Your player", M_HandleSetupMultiPlayer,  96}, // Tails 01-18-2001

	{IT_CALL | IT_WHITESTRING,    NULL, "Setup Controls...",
	                                                   M_Setup2PControlsMenu,    120},
	{IT_SUBMENU | IT_WHITESTRING, NULL, "Second Mouse config...",
	                                                   &SecondMouseCfgdef,       130},
};

enum
{
	setupmultiplayer_name = 0,
	setupmultiplayer_color,
	setupmultiplayer_skin,
	setupmultiplayer_controls,
	setupmultiplayer_mouse2,
	setupmulti_end
};

menu_t SetupMultiPlayerDef =
{
	"M_PICKP",
	"Multiplayer",
	sizeof (SetupMultiPlayerMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	SetupMultiPlayerMenu,
	M_DrawSetupMultiPlayerMenu,
	27, 40,
	0,
	M_QuitMultiPlayerMenu
};

// Tails 03-02-2002
static void M_DrawSetupChoosePlayerMenu(void);
static boolean M_QuitChoosePlayerMenu(void);
static void M_ChoosePlayer(INT32 choice);
INT32 ultmode;
typedef enum
{
	Player1,
	Player2,
	Player3,
	Player4,
	Player5,
	Player6,
	Player7,
	Player8,
	Player9,
	Player10,
	Player11,
	Player12,
	Player13,
	Player14,
	Player15,
	player_end
} players_e;

menuitem_t PlayerMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "SONIC", M_ChoosePlayer,  0},
	{IT_CALL | IT_STRING, NULL, "TAILS", M_ChoosePlayer,  0},
	{IT_CALL | IT_STRING, NULL, "KNUCKLES", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER4", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER5", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER6", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER7", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER8", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER9", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER10", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER11", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER12", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER13", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER14", M_ChoosePlayer,  0},
	{IT_DISABLED,         NULL, "PLAYER15", M_ChoosePlayer,  0},
};

menu_t PlayerDef =
{
	"M_PICKP",
	"Choose Your Character",
	sizeof (PlayerMenu)/sizeof (menuitem_t),//player_end,
	&MainDef,
	PlayerMenu,
	M_DrawSetupChoosePlayerMenu,
	24, 16,
	0,
	M_QuitChoosePlayerMenu
};
// Tails 03-02-2002

#define PLBOXW    8
#define PLBOXH    9

static INT32       multi_tics;
static state_t * multi_state;

// this is set before entering the MultiPlayer setup menu,
// for either player 1 or 2
static char       setupm_name[MAXPLAYERNAME+1];
static player_t * setupm_player;
static consvar_t *setupm_cvskin;
static consvar_t *setupm_cvcolor;
static consvar_t *setupm_cvname;
static INT32 setupm_skin;
static INT32 setupm_color;

static void M_SetupMultiPlayer(INT32 choice)
{
	(void)choice;
	if (!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		M_StartMessage("You have to be in a game\nto do this.\n\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy(setupm_name, cv_playername.string);

	SetupMultiPlayerDef.numitems = setupmultiplayer_skin +1;      //remove player2 setup controls and mouse2

	// set for player 1
	setupm_player = &players[consoleplayer];
	setupm_cvskin = &cv_skin;
	setupm_cvcolor = &cv_playercolor;
	setupm_cvname = &cv_playername;
	setupm_skin = cv_skin.value;
	setupm_color = cv_playercolor.value;
	M_SetupNextMenu (&SetupMultiPlayerDef);
}

// start the multiplayer setup menu, for secondary player (splitscreen mode)
static void M_SetupMultiPlayerBis(INT32 choice)
{
	(void)choice;
	if (!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		M_StartMessage("You have to be in a game\nto do this.\n\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy (setupm_name, cv_playername2.string);
	SetupMultiPlayerDef.numitems = setupmulti_end;          //activate the setup controls for player 2

	// set for splitscreen secondary player
	setupm_player = &players[secondarydisplayplayer];
	setupm_cvskin = &cv_skin2;
	setupm_cvcolor = &cv_playercolor2;
	setupm_cvname = &cv_playername2;
	setupm_skin = cv_skin2.value;
	setupm_color = cv_playercolor2.value;
	M_SetupNextMenu (&SetupMultiPlayerDef);
}

#ifndef NONET
// Draw the funky Connect IP menu. Tails 11-19-2002
// So much work for such a little thing!
static void M_DrawConnectIPMenu(void)
{
	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

	// draw name string
//	M_DrawTextBox(82,8,MAXPLAYERNAME,1);
	V_DrawString (128,40,0,setupm_ip);

	// draw text cursor for name
	if (itemOn == 0 &&
	    skullAnimCounter < 4)   //blink cursor
		V_DrawCharacter(128+V_StringWidth(setupm_ip),40,'_',false);
}
#endif

// called at splitscreen changes
void M_SwitchSplitscreen(void)
{
// activate setup for player 2
	if (splitscreen)
		MultiPlayerMenu[setupplayer2].status = IT_CALL | IT_STRING;
	else
		MultiPlayerMenu[setupplayer2].status = IT_DISABLED;

	if (MultiPlayerDef.lastOn == setupplayer2)
		MultiPlayerDef.lastOn = setupplayer1;
}


//
//  Draw the multi player setup menu, had some fun with player anim
//
static void M_DrawSetupMultiPlayerMenu(void)
{
	INT32 mx, my, st;
	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	patch_t *patch;

	mx = SetupMultiPlayerDef.x;
	my = SetupMultiPlayerDef.y;

	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

	// draw name string
	M_DrawTextBox(mx + 90, my - 8, MAXPLAYERNAME, 1);
	V_DrawString(mx + 98, my, V_ALLOWLOWERCASE, setupm_name);

	// draw skin string
	V_DrawString(mx + 90, my + 96, 0, (char *)&skins[setupm_skin].name);

	// draw the name of the color you have chosen
	// Just so people don't go thinking that "Default" is Green.
	V_DrawString(208, 72, 0, Color_Names[setupm_color]);

	// draw text cursor for name
	if (!itemOn && skullAnimCounter < 4) // blink cursor
		V_DrawCharacter(mx + 98 + V_StringWidth(setupm_name), my, '_',false);

	// anim the player in the box
	if (--multi_tics <= 0)
	{
		st = multi_state->nextstate;
		if (st != S_NULL)
			multi_state = &states[st];
		multi_tics = multi_state->tics;
		if (multi_tics == -1)
			multi_tics = 15;
	}

	// skin 0 is default player sprite
	if (R_SkinAvailable((char *)&skins[setupm_skin].name) != -1)
		sprdef = &skins[R_SkinAvailable((char *)&skins[setupm_skin].name)].spritedef;
	else
		sprdef = &skins[0].spritedef;

	sprframe = &sprdef->spriteframes[multi_state->frame & FF_FRAMEMASK];
	patch = W_CachePatchNum(sprframe->lumppat[0], PU_CACHE);

	// draw box around guy
	M_DrawTextBox(mx + 90, my + 8, PLBOXW, PLBOXH);

	// draw player sprite
	if (!setupm_color)
	{
		if (atoi(skins[setupm_skin].highres))
			V_DrawScaledPatch(mx + 98 + (PLBOXW*8/2), my + 16 + (PLBOXH*8) - 8, 0, patch);
		else
			V_DrawScaledPatch(mx + 98 + (PLBOXW*8/2), my + 16 + (PLBOXH*8) - 8, 0, patch);
	}
	else
	{
		UINT8 *colormap = R_GetTranslationColormap(setupm_skin, setupm_color, 0);

		if (atoi(skins[setupm_skin].highres))
			V_DrawSmallMappedPatch(mx + 98 + (PLBOXW*8/2), my + 16 + (PLBOXH*8) - 8, 0, patch, colormap);
		else
			V_DrawMappedPatch(mx + 98 + (PLBOXW*8/2), my + 16 + (PLBOXH*8) - 8, 0, patch, colormap);

		Z_Free(colormap);
	}
}

#ifndef NONET
// Tails 11-19-2002
static void M_HandleConnectIP(INT32 choice)
{
	size_t   l;
	boolean  exitmenu = false;  // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_ENTER:
			S_StartSound(NULL,sfx_menu1); // Tails
			M_ClearMenus(true);
			M_ConnectIP(1);
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			if ((l = strlen(setupm_ip))!=0 && itemOn == 0)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l-1] =0;
			}
			break;

		default:
#define ALLOW_NUMPAD
			l = strlen(setupm_ip);
			if (l < 16-1 && (choice == 46 || (choice >= 48 && choice <= 57))) // Rudimentary number and period enforcing
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l] =(char)choice;
				setupm_ip[l+1] =0;
			}
#ifdef ALLOW_NUMPAD
			else if (l < 16-1 && choice >= 199 && choice <= 211 && choice != 202 && choice != 206) //numpad too!
			{
				XBOXSTATIC char keypad_translation[] = {'7','8','9','-','4','5','6','+','1','2','3','0','.'};
				choice = keypad_translation[choice - 199];
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l] =(char)choice;
				setupm_ip[l+1] =0;
			}
#endif
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}
#endif

//
// Handle Setup MultiPlayer Menu
//
static void M_HandleSetupMultiPlayer(INT32 choice)
{
	size_t   l;
	boolean  exitmenu = false;  // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL,sfx_menu1); // Tails
			if (itemOn+1 >= SetupMultiPlayerDef.numitems)
				itemOn = 0;
			else itemOn++;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL,sfx_menu1); // Tails
			if (!itemOn)
				itemOn = (INT16)(SetupMultiPlayerDef.numitems-1);
			else itemOn--;
			break;

		case KEY_LEFTARROW:
			if (itemOn == 1)       //player color
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_color--;
			}
			else if (itemOn == 2)       //player skin
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_skin--;
			}
			break;

		case KEY_RIGHTARROW:
			if (itemOn == 1)       //player color
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_color++;
			}
			else if (itemOn == 2)       //player skin
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_skin++;
			}
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			if ((l = strlen(setupm_name))!=0 && itemOn == 0)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_name[l-1] =0;
			}
			break;

		default:
			if (choice < 32 || choice > 127 || itemOn != 0)
				break;
			l = strlen(setupm_name);
			if (l < MAXPLAYERNAME-1)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_name[l] =(char)choice;
				setupm_name[l+1] =0;
			}
			break;
	}

	// check skin
	if (setupm_skin < 0)
		setupm_skin = numskins-1;
	if (setupm_skin > numskins-1)
		setupm_skin = 0;

	// check color, we don't like yellow in some instances
	if (gametype == GT_MATCH || gametype == GT_CTF)
	{
		if (setupm_color < 1)
			setupm_color = MAXSKINCOLORS-2; // avoid yellow
		if (setupm_color > MAXSKINCOLORS-2) // avoid yellow
			setupm_color = 1; // don't allow zero
	}
	else
	{
		if (setupm_color < 1)
			setupm_color = MAXSKINCOLORS-1;
		if (setupm_color > MAXSKINCOLORS-1)
			setupm_color = 1; // don't allow zero
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

static boolean M_QuitMultiPlayerMenu(void)
{
	size_t l;
	// send name if changed
	if (strcmp(setupm_name, setupm_cvname->string))
	{
		// remove trailing whitespaces
		for (l= strlen(setupm_name)-1;
		    (signed)l >= 0 && setupm_name[l] ==' '; l--)
			setupm_name[l] =0;
		COM_ImmedExecute (va("%s \"%s\"\n",setupm_cvname->name,setupm_name));
	}
	// check skin change
	if (setupm_skin != setupm_player->skin)
		COM_ImmedExecute (va("%s \"%s\"",setupm_cvskin->name, skins[setupm_skin].name));
	// check color change
	if (setupm_color != setupm_player->skincolor)
		COM_ImmedExecute (va("%s \"%s\"",setupm_cvcolor->name, Color_Names[setupm_color]));

	return true;
}


////////////////////////////////////////////////////////////////
//                   CHARACTER SELECT SCREEN                  //
////////////////////////////////////////////////////////////////

static inline void M_SetupChoosePlayer(INT32 choice)
{
	(void)choice;
	if (Playing() == false)
	{
		S_StopMusic();
		S_ChangeMusic(mus_chrsel, true);
	}

	M_SetupNextMenu (&PlayerDef);
}

//
//  Draw the choose player setup menu, had some fun with player anim
//
static void M_DrawSetupChoosePlayerMenu(void)
{
	INT32      mx = PlayerDef.x, my = PlayerDef.y;
	patch_t *patch;

	// Black BG
	V_DrawFill(0, 0, vid.width, vid.height, 31);

	{
		// Compact the menu
		INT32 i;
		UINT8 alpha = 0;
		for (i = 0; i < currentMenu->numitems; i++)
		{
			if (currentMenu->menuitems[i].status == 0
			|| currentMenu->menuitems[i].status == IT_DISABLED)
				continue;

			currentMenu->menuitems[i].alphaKey = alpha;
			alpha += 8;
		}
	}

	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

	// TEXT BOX!
	// For the character
	M_DrawTextBox(mx+152,my, 16, 16);

	// For description
	M_DrawTextBox(mx-24, my+72, 20, 10);

	patch = W_CachePatchName(description[itemOn].picname, PU_CACHE);

	V_DrawString(mx-16, my+80, V_YELLOWMAP, "Speed:\nAbility:\nNotes:");

	V_DrawScaledPatch(mx+160,my+8,0,patch);
	V_DrawString(mx-16, my+80, 0, description[itemOn].info);
}

//
// Handle Setup Choose Player Menu
//
#if 0
static void M_HandleSetupChoosePlayer(INT32 choice)
{
	boolean  exitmenu = false;  // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL,sfx_menu1); // Tails
			if (itemOn+1 >= SetupMultiPlayerDef.numitems)
				itemOn = 0;
			else itemOn++;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL,sfx_menu1); // Tails
			if (!itemOn)
				itemOn = (INT16)(SetupMultiPlayerDef.numitems-1);
			else itemOn--;
			break;

		case KEY_ENTER:
			S_StartSound(NULL,sfx_menu1); // Tails
			exitmenu = true;
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		default:
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}
#endif

static boolean M_QuitChoosePlayerMenu(void)
{
	// Stop music
	S_StopMusic();
	return true;
}


//===========================================================================
//                           NEW GAME FOR SINGLE PLAYER
//===========================================================================
static void M_Statistics(INT32 choice)
{
	(void)choice;
	if (modifiedgame && !savemoddata)
	{
		M_StartMessage("Statistics not available\nin modified games.", NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&StatsDef);
}

static void M_Stats2(INT32 choice)
{
	(void)choice;
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats2Def);
}

static void M_Stats3(INT32 choice)
{
	(void)choice;
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats3Def);
}

static void M_Stats4(INT32 choice)
{
	(void)choice;
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats4Def);
}

//
// M_GetLevelEmblem
//
// Returns pointer to an emblem if an emblem exists
// for that level, and exists for that player.
// NULL if not found.
//
static emblem_t *M_GetLevelEmblem(INT32 mapnum, INT32 player)
{
	INT32 i;

	for (i = 0; i < numemblems; i++)
	{
		if (emblemlocations[i].level == mapnum
			&& emblemlocations[i].player == player)
			return &emblemlocations[i];
	}
	return NULL;
}

static void M_DrawStats(void)
{
	INT32 found = 0;
	INT32 i;
	char hours[4];
	char minutes[4];
	char seconds[4];
	tic_t besttime = 0;
	boolean displaytimeattack = true;

	for (i = 0; i < MAXEMBLEMS; i++)
	{
		if (emblemlocations[i].collected)
			found++;
	}

	V_DrawString(64, 32, 0, va("x %d/%d", found, numemblems));
	V_DrawScaledPatch(32, 32-4, 0, W_CachePatchName("EMBLICON", PU_STATIC));

	if (G_TicsToHours(totalplaytime) < 10)
		sprintf(hours, "0%i", G_TicsToHours(totalplaytime));
	else
		sprintf(hours, "%i:", G_TicsToHours(totalplaytime));

	if (G_TicsToMinutes(totalplaytime, false) < 10)
		sprintf(minutes, "0%i", G_TicsToMinutes(totalplaytime, false));
	else
		sprintf(minutes, "%i", G_TicsToMinutes(totalplaytime, false));

	if (G_TicsToSeconds(totalplaytime) < 10)
		sprintf(seconds, "0%i", G_TicsToSeconds(totalplaytime));
	else
		sprintf(seconds, "%i", G_TicsToSeconds(totalplaytime));

	V_DrawCenteredString(224, 8, 0, "Total Play Time:");
	V_DrawCenteredString(224, 20, 0, va("%s:%s:%s", hours, minutes, seconds));

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->timeattack))
			continue;

		if (timedata[i].time > 0)
			besttime += timedata[i].time;
		else
			displaytimeattack = false;
	}

	if (displaytimeattack)
	{
		if (G_TicsToHours(besttime) < 10)
			sprintf(hours, "0%i", G_TicsToHours(besttime));
		else
			sprintf(hours, "%i", G_TicsToHours(besttime));

		if (G_TicsToMinutes(besttime, false) < 10)
			sprintf(minutes, "0%i", G_TicsToMinutes(besttime, false));
		else
			sprintf(minutes, "%i", G_TicsToMinutes(besttime, false));

		if (G_TicsToSeconds(besttime) < 10)
			sprintf(seconds, "0%i", G_TicsToSeconds(besttime));
		else
			sprintf(seconds, "%i", G_TicsToSeconds(besttime));

		V_DrawCenteredString(224, 36, 0, "Best Time Attack:");
		V_DrawCenteredString(224, 48, 0, va("%s:%s:%s", hours, minutes, seconds));
	}

	{
		INT32 y = 80;
		char names[8];
		emblem_t *emblem;

		V_DrawString(32+36, y-16, 0, "LEVEL NAME");
		V_DrawString(224+28, y-16, 0, "BEST TIME");

		lastmapnum = 0;
		oldlastmapnum = 0;

		sprintf(names, "%c %c %c", skins[0].name[0], skins[1].name[0], skins[2].name[0]);
		V_DrawString(32, y-16, 0, names);

		for (i = oldlastmapnum; i < NUMMAPS; i++)
		{

			if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
				continue;

			if (!(mapheaderinfo[i]->typeoflevel & TOL_SP))
				continue;

			if (!mapvisited[i])
				continue;

			lastmapnum = i;

			emblem = M_GetLevelEmblem(i+1, 0);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 1);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 2);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			if (mapheaderinfo[i]->actnum != 0)
				V_DrawString(32+36, y, V_YELLOWMAP, va("%s %d", mapheaderinfo[i]->lvlttl, mapheaderinfo[i]->actnum));
			else
				V_DrawString(32+36, y, V_YELLOWMAP, mapheaderinfo[i]->lvlttl);

			if (timedata[i].time)
			{
				if (G_TicsToMinutes(timedata[i].time, true) < 10)
					sprintf(minutes, "0%i", G_TicsToMinutes(timedata[i].time, true));
				else
					sprintf(minutes, "%i", G_TicsToMinutes(timedata[i].time, true));

				if (G_TicsToSeconds(timedata[i].time) < 10)
					sprintf(seconds, "0%i", G_TicsToSeconds(timedata[i].time));
				else
					sprintf(seconds, "%i", G_TicsToSeconds(timedata[i].time));

				if (G_TicsToCentiseconds(timedata[i].time) < 10)
					sprintf(hours, "0%i", G_TicsToCentiseconds(timedata[i].time));
				else
					sprintf(hours, "%i", G_TicsToCentiseconds(timedata[i].time));

				V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));
			}

			y += 8;

			if (y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

static void M_DrawStats2(void)
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		INT32 i;
		INT32 y = 16;
		emblem_t *emblem;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_YELLOWMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 2");

		for (i = oldlastmapnum+1; i < NUMMAPS; i++)
		{
			if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
				continue;

			if (!(mapheaderinfo[i]->typeoflevel & TOL_SP))
				continue;

			if (!mapvisited[i])
				continue;

			lastmapnum = i;

			emblem = M_GetLevelEmblem(i+1, 0);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 1);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 2);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			if (mapheaderinfo[i]->actnum != 0)
				V_DrawString(32+36, y, V_YELLOWMAP, va("%s %d", mapheaderinfo[i]->lvlttl, mapheaderinfo[i]->actnum));
			else
				V_DrawString(32+36, y, V_YELLOWMAP, mapheaderinfo[i]->lvlttl);

			if (timedata[i].time)
			{
				if (G_TicsToMinutes(timedata[i].time, true) < 10)
					sprintf(minutes, "0%i", G_TicsToMinutes(timedata[i].time, true));
				else
					sprintf(minutes, "%i", G_TicsToMinutes(timedata[i].time, true));

				if (G_TicsToSeconds(timedata[i].time) < 10)
					sprintf(seconds, "0%i", G_TicsToSeconds(timedata[i].time));
				else
					sprintf(seconds, "%i", G_TicsToSeconds(timedata[i].time));

				if (G_TicsToCentiseconds(timedata[i].time) < 10)
					sprintf(hours, "0%i", G_TicsToCentiseconds(timedata[i].time));
				else
					sprintf(hours, "%i", G_TicsToCentiseconds(timedata[i].time));

				V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));
			}

			y += 8;

			if (y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

static void M_DrawStats3(void)
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		INT32 i;
		INT32 y = 16;
		emblem_t *emblem;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_YELLOWMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 3");

		for (i = oldlastmapnum+1; i < NUMMAPS; i++)
		{
			if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
				continue;

			if (!(mapheaderinfo[i]->typeoflevel & TOL_SP))
				continue;

			if (!mapvisited[i])
				continue;

			lastmapnum = i;

			emblem = M_GetLevelEmblem(i+1, 0);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 1);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 2);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			if (mapheaderinfo[i]->actnum != 0)
				V_DrawString(32+36, y, V_YELLOWMAP, va("%s %d", mapheaderinfo[i]->lvlttl, mapheaderinfo[i]->actnum));
			else
				V_DrawString(32+36, y, V_YELLOWMAP, mapheaderinfo[i]->lvlttl);

			if (timedata[i].time)
			{
				if (G_TicsToMinutes(timedata[i].time, true) < 10)
					sprintf(minutes, "0%i", G_TicsToMinutes(timedata[i].time, true));
				else
					sprintf(minutes, "%i", G_TicsToMinutes(timedata[i].time, true));

				if (G_TicsToSeconds(timedata[i].time) < 10)
					sprintf(seconds, "0%i", G_TicsToSeconds(timedata[i].time));
				else
					sprintf(seconds, "%i", G_TicsToSeconds(timedata[i].time));

				if (G_TicsToCentiseconds(timedata[i].time) < 10)
					sprintf(hours, "0%i", G_TicsToCentiseconds(timedata[i].time));
				else
					sprintf(hours, "%i", G_TicsToCentiseconds(timedata[i].time));

				V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));
			}

			y += 8;

			if (y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

static void M_DrawStats4(void)
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		INT32 i;
		INT32 y = 16;
		emblem_t *emblem;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_YELLOWMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 4");

		for (i = oldlastmapnum+1; i < NUMMAPS; i++)
		{
			if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
				continue;

			if (!(mapheaderinfo[i]->typeoflevel & TOL_SP))
				continue;

			if (!mapvisited[i])
				continue;

			lastmapnum = i;

			emblem = M_GetLevelEmblem(i+1, 0);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 1);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 2);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			if (mapheaderinfo[i]->actnum != 0)
				V_DrawString(32+36, y, V_YELLOWMAP, va("%s %d", mapheaderinfo[i]->lvlttl, mapheaderinfo[i]->actnum));
			else
				V_DrawString(32+36, y, V_YELLOWMAP, mapheaderinfo[i]->lvlttl);

			if (timedata[i].time)
			{
				if (G_TicsToMinutes(timedata[i].time, true) < 10)
					sprintf(minutes, "0%i", G_TicsToMinutes(timedata[i].time, true));
				else
					sprintf(minutes, "%i", G_TicsToMinutes(timedata[i].time, true));

				if (G_TicsToSeconds(timedata[i].time) < 10)
					sprintf(seconds, "0%i", G_TicsToSeconds(timedata[i].time));
				else
					sprintf(seconds, "%i", G_TicsToSeconds(timedata[i].time));

				if (G_TicsToCentiseconds(timedata[i].time) < 10)
					sprintf(hours, "0%i", G_TicsToCentiseconds(timedata[i].time));
				else
					sprintf(hours, "%i", G_TicsToCentiseconds(timedata[i].time));

				V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));
			}

			y += 8;

			if (y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

static void M_DrawStats5(void)
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		INT32 i;
		INT32 y = 16;
		emblem_t *emblem;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_YELLOWMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 5");

		for (i = oldlastmapnum+1; i < NUMMAPS; i++)
		{
			if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
				continue;

			if (!(mapheaderinfo[i]->typeoflevel & TOL_SP))
				continue;

			if (!mapvisited[i])
				continue;

			lastmapnum = i;

			emblem = M_GetLevelEmblem(i+1, 0);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(30, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 1);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(42, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			emblem = M_GetLevelEmblem(i+1, 2);

			if (emblem)
			{
				if (emblem->collected)
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
				else
					V_DrawScaledPatch(54, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));
			}

			if (mapheaderinfo[i]->actnum != 0)
				V_DrawString(32+36, y, V_YELLOWMAP, va("%s %d", mapheaderinfo[i]->lvlttl, mapheaderinfo[i]->actnum));
			else
				V_DrawString(32+36, y, V_YELLOWMAP, mapheaderinfo[i]->lvlttl);

			if (timedata[i].time)
			{
				if (G_TicsToMinutes(timedata[i].time, true) < 10)
					sprintf(minutes, "0%i", G_TicsToMinutes(timedata[i].time, true));
				else
					sprintf(minutes, "%i", G_TicsToMinutes(timedata[i].time, true));

				if (G_TicsToSeconds(timedata[i].time) < 10)
					sprintf(seconds, "0%i", G_TicsToSeconds(timedata[i].time));
				else
					sprintf(seconds, "%i", G_TicsToSeconds(timedata[i].time));

				if (G_TicsToCentiseconds(timedata[i].time) < 10)
					sprintf(hours, "0%i", G_TicsToCentiseconds(timedata[i].time));
				else
					sprintf(hours, "%i", G_TicsToCentiseconds(timedata[i].time));

				V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));
			}

			y += 8;

			if (y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

static void M_NewGame(void)
{
	fromlevelselect = false;
	pandoralevelselect = false;
	ultmode = false;

	startmap = spstage_start;
	CV_SetValue(&cv_newgametype, GT_COOP); // Graue 09-08-2004

	PlayerDef.prevMenu = currentMenu;
	M_SetupChoosePlayer(0);

	StartSplitScreenGame = false;
}

static void M_SRB1Remake(INT32 choice)
{
	(void)choice;
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	startmap = 101;

	PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&PlayerDef);

	StartSplitScreenGame = false;
}

static void M_NightsGame(INT32 choice)
{
	(void)choice;
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	startmap = 29;

	PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&PlayerDef);

	StartSplitScreenGame = false;
}

static void M_MarioGame(INT32 choice)
{
	(void)choice;
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	startmap = 30;

	PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&PlayerDef);

	StartSplitScreenGame = false;
}

static void M_NAGZGame(INT32 choice)
{
	(void)choice;
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	startmap = 40;

	PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&PlayerDef);

	StartSplitScreenGame = false;
}

static void M_CustomWarp(INT32 choice)
{
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	startmap = (INT16)(customsecretinfo[choice-1].variable);

	PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&PlayerDef);

	StartSplitScreenGame = false;
}

// Chose the player you want to use Tails 03-02-2002
static void M_ChoosePlayer(INT32 choice)
{
	INT32 skinnum;

	M_ClearMenus(true);

	strlwr(description[choice].skinname);

	skinnum = R_SkinAvailable(description[choice].skinname);

	if (startmap != spstage_start)
		cursaveslot = -1;

	lastmapsaved = 0;
	gamecomplete = false;

	G_DeferedInitNew(ultmode, G_BuildMapName(startmap), skinnum, StartSplitScreenGame, fromlevelselect);
	COM_BufAddText("dummyconsvar 1\n"); // G_DeferedInitNew doesn't do this
}

static void M_ReplayTimeAttack(INT32 choice);
static void M_ChooseTimeAttackNoRecord(INT32 choice);
static void M_ChooseTimeAttack(INT32 choice);
static void M_DrawTimeAttackMenu(void);

typedef enum
{
	taplayer = 0,
	talevel,
	tareplay,
	tastart,
	timeattack_end
} timeattack_e;

static menuitem_t TimeAttackMenu[] =
{
	{IT_STRING|IT_CVAR,      NULL, "Player",     &cv_chooseskin,   50},
	{IT_STRING|IT_CVAR,      NULL, "Level",      &cv_nextmap,      65},

	{IT_WHITESTRING|IT_CALL, NULL, "Replay",   M_ReplayTimeAttack,        100},
	{IT_WHITESTRING|IT_CALL, NULL, "Start (No Record)",  M_ChooseTimeAttackNoRecord,115},
	{IT_WHITESTRING|IT_CALL, NULL, "Start (Record)",     M_ChooseTimeAttack,        130},
};

menu_t TimeAttackDef =
{
	0,
	"Time Attack",
	sizeof (TimeAttackMenu)/sizeof (menuitem_t),
	&SinglePlayerDef,
	TimeAttackMenu,
	M_DrawTimeAttackMenu,
	40, 40,
	0,
	NULL
};

// Used only for time attack menu
static void Nextmap_OnChange(void)
{
	if (currentMenu != &TimeAttackDef)
		return;

	TimeAttackMenu[tareplay].status = IT_DISABLED;

	// Check if file exists, if not, disable REPLAY option
	if (FIL_FileExists(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%02d.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value), cv_chooseskin.value-1)))
		TimeAttackMenu[tareplay].status = IT_WHITESTRING|IT_CALL;
}

//
// M_TimeAttack
//
static void M_TimeAttack(INT32 choice)
{
	(void)choice;

	if (modifiedgame && !savemoddata)
	{
		M_StartMessage("This cannot be done in a modified game.\n",NULL,MM_NOTHING);
		return;
	}

	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}

	memset(skins_cons_t, 0, sizeof (skins_cons_t));

	if (!(M_PatchLevelNameTable(2)))
	{
		M_StartMessage("No time-attackable levels found.\n",NULL,MM_NOTHING);
		return;
	}

	inlevelselect = 2; // Don't be dependent on cv_newgametype

	M_PatchSkinNameTable();

	M_SetupNextMenu(&TimeAttackDef);

	CV_AddValue(&cv_nextmap, 1);
	CV_AddValue(&cv_nextmap, -1);

	G_SetGamestate(GS_TIMEATTACK);
	S_ChangeMusic(mus_racent, true);
}

//
// M_DrawTimeAttackMenu
//
void M_DrawTimeAttackMenu(void)
{
	patch_t *PictureOfLevel;
	lumpnum_t lumpnum;
	char hours[4];
	char minutes[4];
	char seconds[4];
	char tics[4];
	tic_t besttime = 0;
	INT32 i;

	S_ChangeMusic(mus_racent, true); // Eww, but needed for when user hits escape during demo playback

	V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	if (W_CheckNumForName(description[cv_chooseskin.value-1].picname) != LUMPERROR)
		V_DrawSmallScaledPatch(224, 16, 0, W_CachePatchName(description[cv_chooseskin.value-1].picname, PU_CACHE));

	//  A 160x100 image of the level as entry MAPxxP
	lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(cv_nextmap.value)));

	if (lumpnum != LUMPERROR)
		PictureOfLevel = W_CachePatchName(va("%sP", G_BuildMapName(cv_nextmap.value)), PU_CACHE);
	else
		PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);

	V_DrawSmallScaledPatch(208, 128, 0, PictureOfLevel);

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->timeattack))
			continue;

		if (timedata[i].time > 0)
			besttime += timedata[i].time;
	}

	sprintf(hours,   "%02i", G_TicsToHours(besttime));
	sprintf(minutes, "%02i", G_TicsToMinutes(besttime, false));
	sprintf(seconds, "%02i", G_TicsToSeconds(besttime));
	sprintf(tics,    "%02i", G_TicsToCentiseconds(besttime));

	V_DrawCenteredString(128, 36, 0, "Best Time Attack:");
	V_DrawCenteredString(128, 48, 0, va("%s:%s:%s.%s", hours, minutes, seconds, tics));

	if (cv_nextmap.value)
	{
		if (timedata[cv_nextmap.value-1].time > 0)
			V_DrawCenteredString(BASEVIDWIDTH/2, 116, 0, va("Best Time: %i:%02i.%02i", G_TicsToMinutes(timedata[cv_nextmap.value-1].time, true),
				G_TicsToSeconds(timedata[cv_nextmap.value-1].time), G_TicsToCentiseconds(timedata[cv_nextmap.value-1].time)));
	}

	M_DrawGenericMenu();
}

//
// M_ChooseTimeAttackNoRecord
//
// Like M_ChooseTimeAttack, but doesn't record a demo.
static void M_ChooseTimeAttackNoRecord(INT32 choice)
{
	(void)choice;
	emeralds = 0;
	M_ClearMenus(true);
	timeattacking = true;
	remove(va("%s"PATHSEP"temp.lmp", srb2home));
	G_DeferedInitNew(false, G_BuildMapName(cv_nextmap.value), cv_chooseskin.value-1, false, false);
	timeattacking = true;
}

//
// M_ChooseTimeAttack
//
// Player has selected the "START" from the time attack screen
static void M_ChooseTimeAttack(INT32 choice)
{
	(void)choice;
	emeralds = 0;
	M_ClearMenus(true);
	timeattacking = true;
	G_RecordDemo("temp");
	G_BeginRecording();
	G_DeferedInitNew(false, G_BuildMapName(cv_nextmap.value), cv_chooseskin.value-1, false, false);
	timeattacking = true;
}

//
// M_ReplayTimeAttack
//
// Player has selected the "REPLAY" from the time attack screen
static void M_ReplayTimeAttack(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);
	G_DoPlayDemo(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%02d", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value), cv_chooseskin.value-1));

	timeattacking = true;
}

static void M_EraseData(INT32 choice);

// Tails 08-11-2002
//===========================================================================
//                        Data OPTIONS MENU
//===========================================================================

static menuitem_t DataOptionsMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "Erase Time Attack Data", M_EraseData, 0},
	{IT_STRING | IT_CALL, NULL, "Erase Secrets Data", M_EraseData, 0},
};

menu_t DataOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (DataOptionsMenu)/sizeof (menuitem_t),
	&GameOptionDef,
	DataOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

static void M_TimeDataResponse(INT32 ch)
{
	INT32 i;
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// Delete the data
	for (i = 0; i < NUMMAPS; i++)
		timedata[i].time = 0;

	M_SetupNextMenu(&DataOptionsDef);
}

static void M_SecretsDataResponse(INT32 ch)
{
	INT32 i;
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// Delete the data
	for (i = 0; i < MAXEMBLEMS; i++)
		emblemlocations[i].collected = false;

	grade = 0;
	timesbeaten = 0;

	M_ClearMenus(true);
}

static void M_EraseData(INT32 choice)
{
	if (Playing())
	{
		M_StartMessage("A game cannot be running.\nEnd it first.",NULL,MM_NOTHING);
		return;
	}

	else if (choice == 0)
		M_StartMessage("Are you sure you want to delete\nthe time attack data?\n(Y/N)\n",M_TimeDataResponse,MM_YESNO);
	else // 1
		M_StartMessage("Are you sure you want to delete\nthe secrets data?\n(Y/N)\n",M_SecretsDataResponse,MM_YESNO);
}

void M_OnePControlsMenu(INT32 choice);
void M_TwoPControlsMenu(INT32 choice);

static menuitem_t ControlsMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "Player 1 Controls...", M_OnePControlsMenu,  20},
	{IT_CALL | IT_STRING, NULL, "Player 2 Controls...", M_TwoPControlsMenu,  30},

	{IT_SUBMENU | IT_STRING, NULL, "Joystick Options...", &JoystickDef  ,  60},
	{IT_SUBMENU | IT_STRING, NULL, "Mouse Options...", &MouseOptionsDef, 70},

	{IT_STRING  | IT_CVAR, NULL, "Control per key", &cv_controlperkey, 100}, // Changed all to normal string Tails 11-30-2000
};

menu_t ControlsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (ControlsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	ControlsMenu,
	M_DrawGenericMenu,
	60, 24,
	0,
	NULL
};

static menuitem_t OnePControlsMenu[] =
{
	{IT_CALL    | IT_STRING, NULL, "Control Configuration...", M_Setup1PControlsMenu,   20},

	{IT_STRING  | IT_CVAR, NULL, "Camera"  , &cv_chasecam  ,  40}, // Changed all to normal string Tails 11-30-2000

	{IT_STRING  | IT_CVAR, NULL, "Analog Control", &cv_useranalog,  60}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING  | IT_CVAR, NULL, "Autoaim" , &cv_autoaim   ,  80}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING  | IT_CVAR, NULL, "Crosshair", &cv_crosshair , 100}, // Changed all to normal string Tails 11-30-2000
};

menu_t OnePControlsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OnePControlsMenu)/sizeof (menuitem_t),
	&ControlsDef,
	OnePControlsMenu,
	M_DrawGenericMenu,
	60, 24,
	0,
	NULL
};

static menuitem_t TwoPControlsMenu[] =
{
	{IT_CALL    | IT_STRING, NULL, "Control Configuration...", M_Setup2PControlsMenu,   20},

	{IT_STRING  | IT_CVAR, NULL, "Camera"  , &cv_chasecam2 , 40}, // Changed all to normal string Tails 11-30-2000

	{IT_STRING  | IT_CVAR, NULL, "Analog Control", &cv_useranalog2,  60}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING  | IT_CVAR, NULL, "Autoaim" , &cv_autoaim2  , 80}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING  | IT_CVAR, NULL, "Crosshair", &cv_crosshair2, 100}, // Changed all to normal string Tails 11-30-2000
};

menu_t TwoPControlsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OnePControlsMenu)/sizeof (menuitem_t),
	&ControlsDef,
	TwoPControlsMenu,
	M_DrawGenericMenu,
	60, 24,
	0,
	NULL
};

void M_OnePControlsMenu(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu(&OnePControlsDef);
}

void M_TwoPControlsMenu(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu(&TwoPControlsDef);
}

//===========================================================================
//                             OPTIONS MENU
//===========================================================================
//
// M_Options
//

//added : 10-02-98: note: alphaKey member is the y offset
static menuitem_t OptionsMenu[] =
{
	{IT_SUBMENU | IT_STRING, NULL, "Setup Controls...",     &ControlsDef,      10},
	{IT_CALL    | IT_STRING, NULL, "Game Options...",       M_GameOption,      30},
	{IT_CALL    | IT_STRING, NULL, "Gametype Options...",   M_GametypeOptions, 40},
	{IT_SUBMENU | IT_STRING, NULL, "Server Options...",     &ServerOptionsDef, 50},
	{IT_SUBMENU | IT_STRING, NULL, "Sound Options...",      &SoundDef,         70},
	{IT_SUBMENU | IT_STRING, NULL, "Video Options...",      &VideoOptionsDef,  80},
};

menu_t OptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OptionsMenu)/sizeof (menuitem_t),
	&MainDef,
	OptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

// Tails 08-18-2002
static void M_OptionsMenu(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu (&OptionsDef);
}

FUNCNORETURN static ATTRNORETURN void M_UltimateCheat(INT32 choice)
{
	(void)choice;
	I_Quit ();
}

static void M_GetAllEmeralds(INT32 choice)
{
	(void)choice;

	if (!(Playing() && gamestate == GS_LEVEL))
	{
		M_StartMessage("You need to be playing and in\na level to do this!",NULL,MM_NOTHING);
		return;
	}

	if (multiplayer || netgame)
	{
		M_StartMessage("You can't do this in\na network game!",NULL,MM_NOTHING);
		return;
	}

	emeralds = ((EMERALD7)*2)-1;
	M_StartMessage("You now have all 7 emeralds.",NULL,MM_NOTHING);
}

static void M_DestroyRobotsResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// Destroy all robots
	P_DestroyRobots();

	M_ClearMenus(true);
}

static void M_DestroyRobots(INT32 choice)
{
	(void)choice;
	if (!(Playing() && gamestate == GS_LEVEL))
	{
		M_StartMessage("You need to be playing and in\na level to do this!",NULL,MM_NOTHING);
		return;
	}

	if (multiplayer || netgame)
	{
		M_StartMessage("You can't do this in\na network game!",NULL,MM_NOTHING);
		return;
	}

	M_StartMessage("Do you want to destroy all\nrobots in the current level?\n(Y/N)\n",M_DestroyRobotsResponse,MM_YESNO);
}

static void M_LevelSelectWarp(INT32 choice)
{
	(void)choice;
	if (netgame && Playing())
	{
		M_StartMessage(M_GetText("You are in a network game.\n""End it?\n(Y/N).\n"), M_ExitGameResponse, MM_YESNO);
		return;
	}

	if (W_CheckNumForName(G_BuildMapName(cv_nextmap.value)) == LUMPERROR)
	{
//		DEBPRINT(va("\2Internal game map '%s' not found\n", G_BuildMapName(cv_nextmap.value)));
		return;
	}

	// Allow character select when level warping from Pandora's Box,
	// even if you are playing a fully completed save.
	if (pandoralevelselect)
	{
		//disassociate our save game since we're using the general level select.
		fromloadgame = 0;
		cursaveslot = -1;
	}

	if (!fromloadgame)
	{
		PlayerDef.prevMenu = currentMenu;
		M_SetupNextMenu(&PlayerDef);
	}

	startmap = (INT16)(cv_nextmap.value);

	fromlevelselect = true;

	StartSplitScreenGame = false;

	if (fromloadgame)
	{
		G_LoadGame((UINT32)fromloadgame - 1, startmap);
		M_ClearMenus(true);
	}
}

/** Checklist of unlockable bonuses.
  */
typedef struct
{
	const char *name;        ///< What you get.
	const char *requirement; ///< What you have to do.
	boolean unlocked;        ///< Whether you've done it.
} checklist_t;

// Tails 12-19-2003
static void M_DrawUnlockChecklist(void)
{
#define NUMCHECKLIST 9
	checklist_t checklist[NUMCHECKLIST];
	INT32 i = 0;
	INT32 y = 8;

	checklist[i].name = "Level Select";
	checklist[i].requirement = "Find All Emblems";
	checklist[i].unlocked = (grade & 8);
	i++;

	checklist[i].name = "SRB1 Remake";
	checklist[i].requirement = "Finish 1P\nw/ Emeralds";
	checklist[i].unlocked = (grade & 2);
	i++;

	checklist[i].name = "Sonic Into Dreams";
	checklist[i].requirement = "Find 10 Emblems";
	checklist[i].unlocked = (grade & 16);
	i++;

	checklist[i].name = "Mario Koopa Blast";
	checklist[i].requirement = "Find 20 Emblems";
	checklist[i].unlocked = (grade & 4);
	i++;

	checklist[i].name = "Pandora's Box";
	checklist[i].requirement = "Find All Emblems";
	checklist[i].unlocked = (grade & 8);
	i++;

	checklist[i].name = "Extra Emblem #1";
	checklist[i].requirement = "Finish 1P";
	checklist[i].unlocked = (emblemlocations[MAXEMBLEMS-2].collected);
	i++;

	checklist[i].name = "Extra Emblem #2";
	checklist[i].requirement = "Finish 1P\nw/ Emeralds";
	checklist[i].unlocked = (emblemlocations[MAXEMBLEMS-1].collected);
	i++;

	checklist[i].name = "Extra Emblem #3";
	checklist[i].requirement = "Finish 1P in\n23 minutes";
	checklist[i].unlocked = (emblemlocations[MAXEMBLEMS-3].collected);
	i++;

	checklist[i].name = "Extra Emblem #4";
	checklist[i].requirement = "Perfect Bonus on\nany stage";
	checklist[i].unlocked = (emblemlocations[MAXEMBLEMS-4].collected);
	i++;

	for (i = 0; i < NUMCHECKLIST; i++)
	{
		V_DrawString(8, y, V_RETURN8, checklist[i].name);
		V_DrawString(160, y, V_RETURN8, checklist[i].requirement);

		if (checklist[i].unlocked)
			V_DrawString(308, y, V_YELLOWMAP, "Y");
		else
			V_DrawString(308, y, V_YELLOWMAP, "N");

		y += 20;
	}
}

boolean M_GotEnoughEmblems(INT32 number)
{
	INT32 i;
	INT32 gottenemblems = 0;

	for (i = 0; i < MAXEMBLEMS; i++)
	{
		if (emblemlocations[i].collected)
			gottenemblems++;
	}

	if (gottenemblems >= number)
		return true;

	return false;
}

boolean M_GotLowEnoughTime(INT32 ptime)
{
	INT32 seconds = 0;
	INT32 i;

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->timeattack))
			continue;

		if (timedata[i].time > 0)
			seconds += timedata[i].time;
		else
			seconds += 800*TICRATE;
	}

	seconds /= TICRATE;

	if (seconds <= ptime)
		return true;

	return false;
}

static void M_DrawCustomChecklist(void)
{
	INT32 numcustom = 0;
	INT32 i;
	INT32 totalnum = 0;
	checklist_t checklist[15];

	memset(checklist, 0, sizeof (checklist));

	for (i = 0; i < 15; i++)
	{
		if (customsecretinfo[i].neededemblems)
		{
			checklist[i].unlocked = M_GotEnoughEmblems(customsecretinfo[i].neededemblems);

			if (checklist[i].unlocked && customsecretinfo[i].neededtime)
				checklist[i].unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);

			if (checklist[i].unlocked && customsecretinfo[i].neededgrade)
				checklist[i].unlocked = (grade & customsecretinfo[i].neededgrade);
		}
		else if (customsecretinfo[i].neededtime)
		{
			checklist[i].unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);

			if (checklist[i].unlocked && customsecretinfo[i].neededgrade)
				checklist[i].unlocked = (grade & customsecretinfo[i].neededgrade);
		}
		else
			checklist[i].unlocked = (grade & customsecretinfo[i].neededgrade);

		if (checklist[i].unlocked)
			totalnum++;
	}

	for (i = 0; i < 15; i++)
	{
		if (checklist[i].unlocked && totalnum > 7)
			continue;

		if (customsecretinfo[i].name[0] == 0)
			continue;

		V_DrawString(8, 8+(24*numcustom), V_RETURN8, customsecretinfo[i].name);
		V_DrawString(160, 8+(24*numcustom), V_RETURN8|V_WORDWRAP, customsecretinfo[i].objective);

		if (checklist[i].unlocked)
			V_DrawString(308, 8+(24*numcustom), V_YELLOWMAP, "Y");
		else
			V_DrawString(308, 8+(24*numcustom), V_YELLOWMAP, "N");

		numcustom++;

		if (numcustom > 6)
			break;
	}
}

// Empty thingy for checklist menu
typedef enum
{
	unlockchecklistempty1,
	unlockchecklist_end
} unlockchecklist_e;

static menuitem_t UnlockChecklistMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "NEXT", M_SecretsMenu, 192},
};

menu_t UnlockChecklistDef =
{
	NULL,
	NULL,
	unlockchecklist_end,
	&SecretsDef,
	UnlockChecklistMenu,
	M_DrawUnlockChecklist,
	280, 185,
	0,
	NULL
};

// Empty thingy for custom checklist menu
typedef enum
{
	customchecklistempty1,
	customchecklist_end
} customchecklist_e;

static menuitem_t CustomChecklistMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "NEXT", M_CustomSecretsMenu, 192},
};

menu_t CustomChecklistDef =
{
	NULL,
	NULL,
	customchecklist_end,
	&CustomSecretsDef,
	CustomChecklistMenu,
	M_DrawCustomChecklist,
	280, 185,
	0,
	NULL
};

static void M_UnlockChecklist(INT32 choice)
{
	(void)choice;
	if (savemoddata)
	{
		M_StartMessage("Checklist does not apply\nfor this mod.\nUse statistics screen instead.", NULL, MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&UnlockChecklistDef);
}

static void M_CustomChecklist(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu(&CustomChecklistDef);
}

static void M_BetaShowcase(INT32 choice)
{
	(void)choice;
}

//===========================================================================
//                             ??? MENU
//===========================================================================
//
// M_Options
//
static void M_Reward(INT32 choice);
static void M_LevelSelect(INT32 choice);
static void M_SRB1LevelSelect(INT32 choice);

typedef enum
{
	unlockchecklist = 0,
	ultimatecheat,
	soundtest,
	secretsgravity,
	ringslinger,
	getemeralds,
	levelselect,
	norobots,
	betashowcase,
	reward,
	secrets_end
} secrets_e;

//added : 10-02-98: note: alphaKey member is the y offset
static menuitem_t SecretsMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "Secrets Checklist",  M_UnlockChecklist,    0},
	{IT_STRING | IT_CALL, NULL, "Ultimate Cheat",     M_UltimateCheat,     20},
	{IT_STRING | IT_CVAR, NULL, "Sound Test",         &cv_soundtest,       30},
	{IT_STRING | IT_CVAR, NULL, "Gravity",            &cv_gravity,         50},
	{IT_STRING | IT_CVAR, NULL, "Throw Rings",        &cv_ringslinger,     60},
	{IT_STRING | IT_CALL, NULL, "Get All Emeralds",   M_GetAllEmeralds,    70},
	{IT_STRING | IT_CALL, NULL, "Level Select",       M_LevelSelect,       90},
	{IT_STRING | IT_CALL, NULL, "Destroy All Robots", M_DestroyRobots,    110},
	{IT_STRING | IT_CALL, NULL, "Beta Showcase",      M_BetaShowcase,     120},
	{IT_STRING | IT_CALL, NULL, "Bonus Levels",       M_Reward,           130},
};

menu_t SecretsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	secrets_end,
	&MainDef,
	SecretsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                             Custom Secrets MENU
//===========================================================================
//
//
//

typedef enum
{
	customchecklist = 0,
	custom1,
	custom2,
	custom3,
	custom4,
	custom5,
	custom6,
	custom7,
	custom8,
	custom9,
	custom10,
	custom11,
	custom12,
	custom13,
	custom14,
	custom15,
	customsecrets_end
} customsecrets_e;

//added : 10-02-98: note: alphaKey member is the y offset
static menuitem_t CustomSecretsMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "Secrets Checklist",  M_CustomChecklist,    0},
	{IT_STRING | IT_CALL, NULL, "Custom1",   M_CustomLevelSelect,       10},
	{IT_STRING | IT_CALL, NULL, "Custom2",   M_CustomWarp,    20},
	{IT_STRING | IT_CALL, NULL, "Custom3",   M_CustomWarp,    30},
	{IT_STRING | IT_CALL, NULL, "Custom4",   M_CustomWarp,    40},
	{IT_STRING | IT_CALL, NULL, "Custom5",   M_CustomWarp,    50},
	{IT_STRING | IT_CALL, NULL, "Custom6",   M_CustomWarp,    60},
	{IT_STRING | IT_CALL, NULL, "Custom7",   M_CustomWarp,    70},
	{IT_STRING | IT_CALL, NULL, "Custom8",   M_CustomWarp,    80},
	{IT_STRING | IT_CALL, NULL, "Custom9",   M_CustomWarp,    90},
	{IT_STRING | IT_CALL, NULL, "Custom10",   M_CustomWarp,    100},
	{IT_STRING | IT_CALL, NULL, "Custom11",   M_CustomWarp,    110},
	{IT_STRING | IT_CALL, NULL, "Custom12",   M_CustomWarp,    120},
	{IT_STRING | IT_CALL, NULL, "Custom13",   M_CustomWarp,    130},
	{IT_STRING | IT_CALL, NULL, "Custom14",   M_CustomWarp,    140},
	{IT_STRING | IT_CALL, NULL, "Custom15",   M_CustomWarp,    150},
};

menu_t CustomSecretsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	customsecrets_end,
	&MainDef,
	CustomSecretsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                             Reward MENU
//===========================================================================
//
// M_Reward
//
typedef enum
{
	nights,
	mario,
	srb1_remake,
	srb1_levelselect,
	nagz,
	reward_end
} reward_e;

//added : 10-02-98: note: alphaKey member is the y offset
static menuitem_t RewardMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "Sonic Into Dreams", M_NightsGame,    30},
	{IT_STRING | IT_CALL, NULL, "Mario Koopa Blast", M_MarioGame,     50},
	{IT_STRING | IT_CALL, NULL,       "SRB1 Remake", M_SRB1Remake,    70},
	{IT_STRING | IT_CALL, NULL, "SRB1 Level Select", M_SRB1LevelSelect, 80},
	{IT_STRING | IT_CALL, NULL, "Neo Aerial Garden", M_NAGZGame,     100},
};

menu_t RewardDef =
{
	"M_OPTTTL",
	"OPTIONS",
	reward_end,
	&SecretsDef,
	RewardMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

static void M_Reward(INT32 choice)
{
	(void)choice;
	if (grade & 2)
		RewardMenu[srb1_remake].status = IT_STRING | IT_CALL;
	else
		RewardMenu[srb1_remake].status |= IT_DISABLED;

	if (grade & 4)
		RewardMenu[mario].status = IT_STRING | IT_CALL;
	else
		RewardMenu[mario].status |= IT_DISABLED;

	if (grade & 16)
		RewardMenu[nights].status = IT_STRING | IT_CALL;
	else
		RewardMenu[nights].status |= IT_DISABLED;

	if (grade & 1024)
		RewardMenu[srb1_levelselect].status = IT_STRING | IT_CALL;
	else
		RewardMenu[srb1_levelselect].status |= IT_DISABLED;

	if (grade & 2048)
		RewardMenu[nagz].status = IT_STRING | IT_CALL;
	else
		RewardMenu[nagz].status |= IT_DISABLED;

	M_SetupNextMenu (&RewardDef);
}

//===========================================================================
//                             Level Select Menu
//===========================================================================
//
// M_LevelSelect
//

static void M_DrawLevelSelectMenu(void)
{
	M_DrawGenericMenu();

	if (cv_nextmap.value)
	{
		lumpnum_t lumpnum;
		patch_t *PictureOfLevel;

		//  A 160x100 image of the level as entry MAPxxP
		lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(cv_nextmap.value)));

		if (lumpnum != LUMPERROR)
			PictureOfLevel = W_CachePatchName(va("%sP", G_BuildMapName(cv_nextmap.value)), PU_CACHE);
		else
			PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);

		V_DrawSmallScaledPatch(200, 110, 0, PictureOfLevel);
	}
}

static menuitem_t LevelSelectMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,        60},

	{IT_WHITESTRING|IT_CALL,         NULL, "Start",                 M_LevelSelectWarp,     120},
};

menu_t LevelSelectDef =
{
	0,
	"Level Select",
	sizeof (LevelSelectMenu)/sizeof (menuitem_t),
	&SecretsDef,
	LevelSelectMenu,
	M_DrawLevelSelectMenu,
	40, 40,
	0,
	NULL
};

static void M_SRB1LevelSelect(INT32 choice)
{
	(void)choice;
	LevelSelectDef.prevMenu = &SecretsDef;
	inlevelselect = 3;
	pandoralevelselect = true;

	if (!(M_PatchLevelNameTable(3)))
	{
		M_StartMessage("No selectable levels found.\n",NULL,MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&LevelSelectDef);
}

static void M_LevelSelect(INT32 choice)
{
	(void)choice;
	LevelSelectDef.prevMenu = &SecretsDef;
	inlevelselect = 1;
	pandoralevelselect = true;

	if (!(M_PatchLevelNameTable(1)))
	{
		M_StartMessage("No selectable levels found.\n",NULL,MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&LevelSelectDef);
}

static void M_CustomLevelSelect(INT32 choice)
{
	(void)choice;
	LevelSelectDef.prevMenu = &CustomSecretsDef;
	inlevelselect = 1;
	pandoralevelselect = true;

	if (!(M_PatchLevelNameTable(1)))
	{
		M_StartMessage("No selectable levels found.\n",NULL,MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&LevelSelectDef);
}

static void M_SecretsMenu(INT32 choice)
{
	INT32 i;

	// Disable all the menu choices
	(void)choice;
	for (i = ultimatecheat;i < secrets_end;i++)
		SecretsMenu[i].status = IT_DISABLED;

	// Check grade and enable options as appropriate
	if (grade & 8)
	{
		SecretsMenu[norobots].status = IT_STRING | IT_CALL;
		SecretsMenu[ringslinger].status = IT_STRING | IT_CVAR;
		SecretsMenu[secretsgravity].status = IT_STRING | IT_CVAR;
		SecretsMenu[ultimatecheat].status = IT_STRING | IT_CALL;
		SecretsMenu[levelselect].status = IT_STRING | IT_CALL;
		SecretsMenu[getemeralds].status = IT_STRING | IT_CALL;
	}
	else
	{
		SecretsMenu[norobots].status = IT_DISABLED;
		SecretsMenu[ringslinger].status = IT_DISABLED;
		SecretsMenu[secretsgravity].status = IT_DISABLED;
		SecretsMenu[ultimatecheat].status = IT_DISABLED;
		SecretsMenu[levelselect].status = IT_DISABLED;
		SecretsMenu[getemeralds].status = IT_DISABLED;
	}

	if ((grade & 2) ||
	(grade & 4) ||
	(grade & 16))
		SecretsMenu[reward].status = IT_STRING | IT_CALL;
	else
		SecretsMenu[reward].status = IT_DISABLED;

	if (grade & 1)
		SecretsMenu[soundtest].status = IT_STRING | IT_CVAR;
	else
		SecretsMenu[soundtest].status = IT_DISABLED;

//	if (grade & 256)
//		Insert reward for beating Ultimate here!

	M_SetupNextMenu(&SecretsDef);
}

static void M_CustomSecretsMenu(INT32 choice)
{
	INT32 i;
	boolean unlocked;

	// Disable all the menu choices
	(void)choice;
	for (i = custom1;i < customsecrets_end;i++)
		CustomSecretsMenu[i].status = IT_DISABLED;

	for (i = 0; i < 15; i++)
	{
		unlocked = false;

		if (customsecretinfo[i].neededemblems)
		{
			unlocked = M_GotEnoughEmblems(customsecretinfo[i].neededemblems);

			if (unlocked && customsecretinfo[i].neededtime)
				unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);

			if (unlocked && customsecretinfo[i].neededgrade)
				unlocked = (grade & customsecretinfo[i].neededgrade);
		}
		else if (customsecretinfo[i].neededtime)
		{
			unlocked = M_GotLowEnoughTime(customsecretinfo[i].neededtime);

			if (unlocked && customsecretinfo[i].neededgrade)
				unlocked = (grade & customsecretinfo[i].neededgrade);
		}
		else
			unlocked = (grade & customsecretinfo[i].neededgrade);

		if (unlocked)
		{
			CustomSecretsMenu[custom1+i].status = IT_STRING|IT_CALL;

			switch (customsecretinfo[i].type)
			{
				case 0:
					CustomSecretsMenu[custom1+i].itemaction = M_CustomLevelSelect;
					break;
				case 1:
					CustomSecretsMenu[custom1+i].itemaction = M_CustomWarp;
				default:
					break;
			}

			CustomSecretsMenu[custom1+i].text = customsecretinfo[i].name;
		}
	}

	M_SetupNextMenu(&CustomSecretsDef);
}

//
//  A smaller 'Thermo', with range given as percents (0-100)
//
static void M_DrawSlider(INT32 x, INT32 y, const consvar_t *cv)
{
	INT32 i;
	INT32 range;
	patch_t *p;

	for (i = 0; cv->PossibleValue[i+1].strvalue; i++);

	range = ((cv->value - cv->PossibleValue[0].value) * 100 /
	 (cv->PossibleValue[i].value - cv->PossibleValue[0].value));

	if (range < 0)
		range = 0;
	if (range > 100)
		range = 100;

	x = BASEVIDWIDTH - x - SLIDER_WIDTH;

	V_DrawScaledPatch(x - 8, y, 0, W_CachePatchName("M_SLIDEL", PU_CACHE));

	p =  W_CachePatchName("M_SLIDEM", PU_CACHE);
	for (i = 0; i < SLIDER_RANGE; i++)
		V_DrawScaledPatch (x+i*8, y, 0,p);

	p = W_CachePatchName("M_SLIDER", PU_CACHE);
	V_DrawScaledPatch(x+SLIDER_RANGE*8, y, 0, p);

	// draw the slider cursor
	p = W_CachePatchName("M_SLIDEC", PU_CACHE);
	V_DrawMappedPatch(x + ((SLIDER_RANGE-1)*8*range)/100, y, 0, p, yellowmap);
}

//===========================================================================
//                        Video OPTIONS MENU
//===========================================================================

//added : 10-02-98: note: alphaKey member is the y offset
static menuitem_t VideoOptionsMenu[] =
{
	// Tails
	{IT_STRING | IT_SUBMENU, NULL, "Video Modes...",      &VidModeDef,        0},
#if (defined (__unix__) && !defined (MSDOS)) || defined (UNIXCOMMON) || defined (SDL)
	{IT_STRING|IT_CVAR,      NULL, "Fullscreen",          &cv_fullscreen,    10},
#endif
#if defined (HWRENDER) && defined (SHUFFLE)
	//17/10/99: added by Hurdler
	{IT_CALL|IT_WHITESTRING, NULL, "3D Card Options...",  M_OpenGLOption,    20},
#endif
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                         NULL, "Brightness",          &cv_usegamma,      40},

	{IT_STRING | IT_CVAR,    NULL, "V-SYNC",              &cv_vidwait,       50},

	{IT_STRING | IT_CVAR,    NULL, "Rain/Snow Density",   &cv_precipdensity, 80}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,    NULL, "Rain/Snow Draw Dist", &cv_precipdist,    90}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,    NULL, "FPS Meter",           &cv_ticrate,       100},
};

menu_t VideoOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (VideoOptionsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	VideoOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Mouse OPTIONS MENU
//===========================================================================

//added : 24-03-00: note: alphaKey member is the y offset
static menuitem_t MouseOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Use Mouse",        &cv_usemouse,         0},
	{IT_STRING | IT_CVAR, NULL, "Always MouseLook", &cv_alwaysfreelook,   0},
	{IT_STRING | IT_CVAR, NULL, "Mouse Move",       &cv_mousemove,        0},
	{IT_STRING | IT_CVAR, NULL, "Invert Mouse",     &cv_invertmouse,      0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse Speed",      &cv_mousesens,        0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mlook Speed",      &cv_mlooksens,        0},
};

menu_t MouseOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (MouseOptionsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	MouseOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Game OPTIONS MENU
//===========================================================================

static menuitem_t GameOptionsMenu[] =
{
	// Tails
	{IT_STRING | IT_CVAR, NULL, "Show HUD",    &cv_showhud,       20},
#ifdef SEENAMES
	{IT_STRING | IT_CVAR, NULL, "HUD Player Names",    &cv_seenames,       30},
#endif
	{IT_STRING | IT_CVAR, NULL, "High Resolution Timer",    &cv_timetic,       40},

	{IT_STRING | IT_CVAR, NULL, "Console Color", &cons_backcolor, 60},
	{IT_STRING | IT_CVAR, NULL, "Uppercase Console", &cv_allcaps, 70},

	{IT_STRING | IT_SUBMENU, NULL, "Data Options...", &DataOptionsDef, 90},
};

menu_t GameOptionDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (GameOptionsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	GameOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

static void M_GameOption(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu(&GameOptionDef);
}

static void M_MonitorToggles(INT32 choice)
{
	(void)choice;
	if (!(server || (adminplayer == consoleplayer)))
	{
		M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&MonitorToggleDef);
}

//===========================================================================
//                        Network OPTIONS MENU
//===========================================================================

static menuitem_t NetOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Time Limit",            &cv_timelimit,        10},
	{IT_STRING | IT_CVAR, NULL, "Point Limit",           &cv_pointlimit,       20},

	{IT_STRING | IT_CVAR, NULL, "Special Ring Weapons",  &cv_specialrings,     40},
	{IT_STRING | IT_CVAR, NULL, "Emeralds",              &cv_powerstones,      50},
	{IT_STRING | IT_CVAR, NULL, "Item Boxes",            &cv_matchboxes,       60},
	{IT_STRING | IT_CVAR, NULL, "Item Respawn",          &cv_itemrespawn,      70},
	{IT_STRING | IT_CVAR, NULL, "Item Respawn time",     &cv_itemrespawntime,  80},

	{IT_STRING | IT_CVAR, NULL, "Server controls skin #", &cv_forceskin,      100},
	{IT_STRING | IT_CVAR, NULL, "Sudden Death",          &cv_suddendeath,     110},
	{IT_STRING | IT_CVAR, NULL, "Intermission Timer",    &cv_inttime,         120},
	{IT_STRING | IT_CVAR, NULL, "Advance to next map",   &cv_advancemap,      130},

	{IT_STRING | IT_CALL, NULL, "Random Monitor Toggles...", M_MonitorToggles,   150},
};

menu_t NetOptionDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (NetOptionsMenu)/sizeof (menuitem_t),
	&MultiPlayerDef,
	NetOptionsMenu,
	M_DrawGenericMenu,
	60, 30,
	0,
	NULL
};

//===========================================================================
//                        GAMETYPE OPTIONS MENU
//===========================================================================

static menuitem_t GametypeOptionsMenu[] =
{
	{IT_STRING | IT_SUBMENU, NULL, "Coop options...",          &CoopOptionsDef,     20},
	{IT_STRING | IT_SUBMENU, NULL, "Race options...",          &RaceOptionsDef,     30},
	{IT_STRING | IT_SUBMENU, NULL, "Match options...",          &MatchOptionsDef,   40},
	{IT_STRING | IT_SUBMENU, NULL, "Tag options...",          &TagOptionsDef,       50},
	{IT_STRING | IT_SUBMENU, NULL, "CTF options...",          &CTFOptionsDef,       60},
};

menu_t GametypeOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (GametypeOptionsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	GametypeOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Coop Mode OPTIONS MENU
//===========================================================================

static menuitem_t CoopOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Players for exit",      &cv_playersforexit,  10},
};

menu_t CoopOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (CoopOptionsMenu)/sizeof (menuitem_t),
	&GametypeOptionsDef,
	CoopOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Race Mode OPTIONS MENU
//===========================================================================

static menuitem_t RaceOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Item Boxes",      &cv_raceitemboxes, 10},
	{IT_STRING | IT_CVAR, NULL, "Number of Laps",  &cv_numlaps,       20},
	{IT_STRING | IT_CVAR, NULL, "Countdown Time",  &cv_countdowntime, 30},
};

menu_t RaceOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (RaceOptionsMenu)/sizeof (menuitem_t),
	&GametypeOptionsDef,
	RaceOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Match Mode OPTIONS MENU
//===========================================================================

static menuitem_t MatchOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Scoring Type",          &cv_match_scoring,   10},
	{IT_STRING | IT_CVAR, NULL, "Team Match Type",       &cv_matchtype,       20},
	{IT_STRING | IT_CVAR, NULL, "Overtime Tie-Breaker",  &cv_overtime,        30},
};

menu_t MatchOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (MatchOptionsMenu)/sizeof (menuitem_t),
	&GametypeOptionsDef,
	MatchOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Tag Mode OPTIONS MENU
//===========================================================================

static menuitem_t TagOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Hide Time",     &cv_hidetime,        10},
};

menu_t TagOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (TagOptionsMenu)/sizeof (menuitem_t),
	&GametypeOptionsDef,
	TagOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        CTF Mode OPTIONS MENU
//===========================================================================

static menuitem_t CTFOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Flag Respawn Time",           &cv_flagtime,         10},
	{IT_STRING | IT_CVAR, NULL, "Autobalance",                 &cv_autobalance,      20},
	{IT_STRING | IT_CVAR, NULL, "Team Scrambler",              &cv_scrambleonchange, 30},
	{IT_STRING | IT_CVAR, NULL, "Overtime Tie-Breaker",        &cv_overtime,         40},
};

menu_t CTFOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (CTFOptionsMenu)/sizeof (menuitem_t),
	&GametypeOptionsDef,
	CTFOptionsMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                        Monitor Toggle MENU
//===========================================================================

static menuitem_t MonitorToggleMenu[] =
{
	{IT_STRING|IT_CVAR, NULL, "Recycler",          &cv_recycler,      10},
	{IT_STRING|IT_CVAR, NULL, "Teleporters",       &cv_teleporters,   20},
	{IT_STRING|IT_CVAR, NULL, "Super Ring",        &cv_superring,     30},
	{IT_STRING|IT_CVAR, NULL, "Super Sneakers",    &cv_supersneakers, 40},
	{IT_STRING|IT_CVAR, NULL, "Invincibility",     &cv_invincibility, 50},
	{IT_STRING|IT_CVAR, NULL, "Jump Shield",       &cv_jumpshield,    60},
	{IT_STRING|IT_CVAR, NULL, "Elemental Shield",  &cv_watershield,   70},
	{IT_STRING|IT_CVAR, NULL, "Attraction Shield", &cv_ringshield,    80},
	{IT_STRING|IT_CVAR, NULL, "Force Shield",      &cv_forceshield,   90},
	{IT_STRING|IT_CVAR, NULL, "Armageddon Shield", &cv_bombshield,   100},
	{IT_STRING|IT_CVAR, NULL, "1 Up",              &cv_1up,          110},
	{IT_STRING|IT_CVAR, NULL, "Eggman Box",        &cv_eggmanbox,    120},
};

menu_t MonitorToggleDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (MonitorToggleMenu)/sizeof (menuitem_t),
	&NetOptionDef,
	MonitorToggleMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

static void M_NetOption(INT32 choice)
{
	(void)choice;
	if (!(server || (adminplayer == consoleplayer)))
	{
		M_StartMessage("You are not the server\nYou can't change the options\n", NULL, MM_NOTHING);
		return;
	}

	if (!(netgame || multiplayer) || !Playing())
	{
		M_StartMessage("You aren't in a game!\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&NetOptionDef);
}

static void M_GametypeOptions(INT32 choice)
{
	(void)choice;
	if (!(server || (adminplayer == consoleplayer)))
	{
		M_StartMessage("You are not the server\nYou can't change the options\n", NULL, MM_NOTHING);
		return;
	}

	if (!(netgame || multiplayer) || !Playing())
	{
		M_StartMessage("You aren't in a game!\nPress a key.", NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&GametypeOptionsDef);
}

//===========================================================================
//                        Server OPTIONS MENU
//===========================================================================
static menuitem_t ServerOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Internet server",        &cv_internetserver,   10},
	{IT_STRING | IT_CVAR | IT_CV_STRING,
	                      NULL, "Master server",          &cv_masterserver,     30},
	{IT_STRING | IT_CVAR | IT_CV_STRING,
	                      NULL, "Server name",            &cv_servername,       60},

	{IT_STRING | IT_CVAR, NULL, "Allow join player",      &cv_allownewplayer , 100},
	{IT_STRING | IT_CVAR, NULL, "Allow WAD Downloading",  &cv_downloading,     110},
	{IT_STRING | IT_CVAR, NULL, "Max Players",            &cv_maxplayers,      120},
	{IT_STRING | IT_CVAR, NULL, "Consistency Protection", &cv_consfailprotect, 130},
};

menu_t ServerOptionsDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (ServerOptionsMenu)/sizeof (menuitem_t),
	&OptionsDef,
	ServerOptionsMenu,
	M_DrawGenericMenu,
	28, 40,
	0,
	NULL
};

//===========================================================================
//                          Read This! MENU 1
//===========================================================================

static void M_DrawReadThis1(void);
static void M_DrawReadThis2(void);

typedef enum
{
	rdthsempty1,
	read1_end
} read_e;

static menuitem_t ReadMenu1[] =
{
	{IT_SUBMENU | IT_NOTHING, NULL, "", &MainDef, 0},
};

menu_t ReadDef1 =
{
	NULL,
	NULL,
	read1_end,
	NULL,
	ReadMenu1,
	M_DrawReadThis1,
	330, 165,
	0,
	NULL
};

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
static void M_DrawReadThis1(void)
{
	V_DrawScaledPatch (0,0,0,W_CachePatchName("HELP",PU_CACHE));
	return;
}

//===========================================================================
//                          *B^D 'Menu'
//===========================================================================

typedef enum
{
	rdthsempty2,
	read2_end
} read_e2;

static menuitem_t ReadMenu2[] =
{
	{IT_SUBMENU | IT_NOTHING, NULL, "", &MainDef, 0},
};

menu_t ReadDef2 =
{
	NULL,
	NULL,
	read2_end,
	NULL,
	ReadMenu2,
	M_DrawReadThis2,
	330, 175,
	0,
	NULL
};


//
// Read This Menus - optional second page.
//
static void M_DrawReadThis2(void)
{
	V_DrawScaledPic (0,0,0,W_GetNumForName ("BULMER"));
	HU_Drawer();
	return;
}

// M_ToggleSFX
// M_ToggleDigital
// M_ToggleMIDI
//
// Toggles sound systems in-game.
//
static void M_ToggleSFX(void)
{
	if (nosound)
	{
		nosound = false;
		I_StartupSound();
		if (nosound) return;
		S_Init(cv_soundvolume.value, cv_digmusicvolume.value, cv_midimusicvolume.value);
		M_StartMessage("SFX Enabled\n", NULL, MM_NOTHING);
	}
	else
	{
		if (sound_disabled)
		{
			sound_disabled = false;
			M_StartMessage("SFX Enabled\n", NULL, MM_NOTHING);
		}
		else
		{
			sound_disabled = true;
			S_StopSounds();
			M_StartMessage("SFX Disabled\n", NULL, MM_NOTHING);
		}
	}
}

static void M_ToggleDigital(void)
{
	if (nodigimusic)
	{
		nodigimusic = false;
		I_InitDigMusic();
		if (nodigimusic) return;
		S_Init(cv_soundvolume.value, cv_digmusicvolume.value, cv_midimusicvolume.value);
		S_StopMusic();
		S_ChangeMusic(mus_lclear, false);
		M_StartMessage("Digital Music Enabled\n", NULL, MM_NOTHING);
	}
	else
	{
		if (digital_disabled)
		{
			digital_disabled = false;
			M_StartMessage("Digital Music Enabled\n", NULL, MM_NOTHING);
		}
		else
		{
			digital_disabled = true;
			S_StopMusic();
			M_StartMessage("Digital Music Disabled\n", NULL, MM_NOTHING);
		}
	}
}

static void M_ToggleMIDI(void)
{
	if (nomidimusic)
	{
		nomidimusic = false;
		I_InitMIDIMusic();
		if (nomidimusic) return;
		S_Init(cv_soundvolume.value, cv_digmusicvolume.value, cv_midimusicvolume.value);
		S_ChangeMusic(mus_lclear, false);
		M_StartMessage("MIDI Music Enabled\n", NULL, MM_NOTHING);
	}
	else
	{
		if (music_disabled)
		{
			music_disabled = false;
			M_StartMessage("MIDI Music Enabled\n", NULL, MM_NOTHING);
		}
		else
		{
			music_disabled = true;
			S_StopMusic();
			M_StartMessage("MIDI Music Disabled\n", NULL, MM_NOTHING);
		}
	}
}

//===========================================================================
//                        SOUND VOLUME MENU
//===========================================================================

typedef enum
{
	sfx_vol,
	sfx_empty1,
	digmusic_vol,
	sfx_empty2,
	midimusic_vol,
	sfx_empty3,
#ifdef PC_DOS
	cdaudio_vol,
	sfx_empty4,
#endif
	tog_sfx,
	tog_dig,
	tog_midi,
	sound_end
} sound_e;

static menuitem_t SoundMenu[] =
{
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
                              NULL, "Sound Volume" , &cv_soundvolume,     0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
                              NULL, "Music Volume" , &cv_digmusicvolume,  10},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
                              NULL, "MIDI Volume"  , &cv_midimusicvolume, 20},
#ifdef PC_DOS
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
                              NULL, "CD Volume"    , &cd_volume,          30},
#endif
	{IT_STRING    | IT_CALL,  NULL,  "Toggle SFX"   , M_ToggleSFX,         40},
	{IT_STRING    | IT_CALL,  NULL,  "Toggle Digital Music", M_ToggleDigital,     50},
	{IT_STRING    | IT_CALL,  NULL,  "Toggle MIDI Music", M_ToggleMIDI,        60},
};

menu_t SoundDef =
{
	"M_SVOL",
	"Sound Volume",
	sizeof (SoundMenu)/sizeof (menuitem_t),
	&OptionsDef,
	SoundMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};

//===========================================================================
//                          JOYSTICK MENU
//===========================================================================
static void M_Setup1PJoystickMenu(INT32 choice);
static void M_Setup2PJoystickMenu(INT32 choice);

typedef enum
{
	p1joy,
	p1set,
	p1turn,
	p1move,
	p1side,
	p1look,
	p1fire,
	p1nfire,
	p2joy,
	p2set,
	p2turn,
	p2move,
	p2side,
	p2look,
	p2fire,
	p2nfire,
	joystick_end
} joy_e;


static menuitem_t JoystickMenu[] =
{
	{IT_WHITESTRING | IT_SPACE, NULL, "Player 1 Joystick" , NULL                 ,  10},
	{IT_STRING      | IT_CALL,  NULL, "Select Joystick...", M_Setup1PJoystickMenu,  20},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Turning"  , &cv_turnaxis         ,  30},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Moving"   , &cv_moveaxis         ,  40},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Strafe"   , &cv_sideaxis         ,  50},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Looking"  , &cv_lookaxis         ,  60},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Firing"   , &cv_fireaxis         ,  70},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For NFiring"  , &cv_firenaxis        ,  80},
	{IT_WHITESTRING | IT_SPACE, NULL, "Player 2 Joystick" , NULL                 ,  90},
	{IT_STRING      | IT_CALL,  NULL, "Select Joystick...", M_Setup2PJoystickMenu, 100},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Turning"  , &cv_turnaxis2        , 110},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Moving"   , &cv_moveaxis2        , 120},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Strafe"   , &cv_sideaxis2        , 130},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Looking"  , &cv_lookaxis2        , 140},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For Firing"   , &cv_fireaxis2        , 150},
	{IT_STRING      | IT_CVAR,  NULL, "Axis For NFiring"  , &cv_firenaxis2       , 160},

};

menu_t JoystickDef =
{
	"M_CONTRO",
	"Setup Joystick",
	joystick_end,
	&ControlsDef,
	JoystickMenu,
	M_DrawGenericMenu,
	50, 20,
	1,
	NULL
};

static void M_DrawJoystick(void);
static void M_AssignJoystick(INT32 choice);

typedef enum
{
	joy0 = 0,
	joy1,
	joy2,
	joy3,
	joy4,
	joy5,
	joy6,
	joystickset_end
} joyset_e;

static menuitem_t JoystickSetMenu[] =
{
	{IT_CALL | IT_NOTHING, "None", NULL, M_AssignJoystick, '0'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '1'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '2'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '3'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '4'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '5'},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, '6'},
};

static menu_t JoystickSetDef =
{
	"M_CONTRO",
	"Select Joystick",
	sizeof (JoystickSetMenu)/sizeof (menuitem_t),
	&JoystickDef,
	JoystickSetMenu,
	M_DrawJoystick,
	50, 40,
	0,
	NULL
};

//===========================================================================
//                          CONTROLS MENU
//===========================================================================
static void M_DrawControl(void);               // added 3-1-98
static void M_ChangeControl(INT32 choice);
static void M_ControlDef2(void);

//
// this is the same for all control pages
//
static menuitem_t ControlMenu[] =
{
	// Player Actions
	{IT_CALL | IT_STRING2, NULL, "Forward",          M_ChangeControl, gc_forward    },
	{IT_CALL | IT_STRING2, NULL, "Reverse",          M_ChangeControl, gc_backward   },
	{IT_CALL | IT_STRING2, NULL, "Turn Left",        M_ChangeControl, gc_turnleft   },
	{IT_CALL | IT_STRING2, NULL, "Turn Right",       M_ChangeControl, gc_turnright  },
	{IT_CALL | IT_STRING2, NULL, "Jump",             M_ChangeControl, gc_jump       },
	{IT_CALL | IT_STRING2, NULL, "Spin",             M_ChangeControl, gc_use        },
	{IT_CALL | IT_STRING2, NULL, "Taunt",            M_ChangeControl, gc_taunt      },
	{IT_CALL | IT_STRING2, NULL, "Toss Flag",        M_ChangeControl, gc_tossflag   },
	{IT_CALL | IT_STRING2, NULL, "Ring Toss",        M_ChangeControl, gc_fire       },
	{IT_CALL | IT_STRING2, NULL, "Ring Toss Normal",
	                                                 M_ChangeControl, gc_firenormal },
	// First person specific
	{IT_CALL | IT_STRING2, NULL, "Strafe On",        M_ChangeControl, gc_strafe     },
	{IT_CALL | IT_STRING2, NULL, "Strafe Left",      M_ChangeControl, gc_strafeleft },
	{IT_CALL | IT_STRING2, NULL, "Strafe Right",     M_ChangeControl, gc_straferight},
	{IT_CALL | IT_STRING2, NULL, "Look Up",          M_ChangeControl, gc_lookup     },
	{IT_CALL | IT_STRING2, NULL, "Look Down",        M_ChangeControl, gc_lookdown   },
	{IT_CALL | IT_STRING2, NULL, "Center View",      M_ChangeControl, gc_centerview },
	{IT_CALL | IT_STRING2, NULL, "Mouselook",        M_ChangeControl, gc_mouseaiming},
	// Misc
	{IT_CALL | IT_STRING2, NULL, "Pause",            M_ChangeControl, gc_pause      },

	{IT_CALL | IT_WHITESTRING,
	                       NULL, "next",             M_ControlDef2,   144           },
};

menu_t ControlDef =
{
	"M_CONTRO",
	"Setup Controls",
	sizeof (ControlMenu)/sizeof (menuitem_t),
	&ControlsDef,
	ControlMenu,
	M_DrawControl,
	24, 40,
	0,
	NULL
};

//
//  Controls page 2
//
// WARNING!: IF YOU MODIFY THIS CHECK "UGLY HACK"
// COMMENTS BELOW TO MAINTAIN CONSISTENCY!!!
//
static menuitem_t ControlMenu2[] =
{
	// Chat
	{IT_CALL | IT_STRING2, NULL, "Talk key",         M_ChangeControl, gc_talkkey      },
	{IT_CALL | IT_STRING2, NULL, "Team-Talk key",    M_ChangeControl, gc_teamkey      },
	// Weapons
	{IT_CALL | IT_STRING2, NULL, "Next Weapon",      M_ChangeControl, gc_weaponnext   },
	{IT_CALL | IT_STRING2, NULL, "Prev Weapon",      M_ChangeControl, gc_weaponprev   },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 1",    M_ChangeControl, gc_normalring   },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 2",    M_ChangeControl, gc_autoring     },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 3",    M_ChangeControl, gc_bouncering   },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 4",    M_ChangeControl, gc_scatterring  },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 5",    M_ChangeControl, gc_grenadering  },
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 6",    M_ChangeControl, gc_explosionring},
	{IT_CALL | IT_STRING2, NULL, "Weapon Slot 7",    M_ChangeControl, gc_railring     },
	// Camera controls
	{IT_CALL | IT_STRING2, NULL, "Rotate Camera L",  M_ChangeControl, gc_camleft      },
	{IT_CALL | IT_STRING2, NULL, "Rotate Camera R",  M_ChangeControl, gc_camright     },
	{IT_CALL | IT_STRING2, NULL, "Reset Camera",     M_ChangeControl, gc_camreset     },
	// Misc
	{IT_CALL | IT_STRING2, NULL, "Rankings/Scores",  M_ChangeControl, gc_scores       },
	{IT_CALL | IT_STRING2, NULL, "Console",          M_ChangeControl, gc_console      },

	{IT_SUBMENU | IT_WHITESTRING,
	                       NULL, "next",             &ControlDef,     140             },
};

menu_t ControlDef2 =
{
	"M_CONTRO",
	"Setup Controls",
	sizeof (ControlMenu2)/sizeof (menuitem_t),
	&ControlsDef,
	ControlMenu2,
	M_DrawControl,
	24, 40,
	0,
	NULL
};


//
// Start the controls menu, setting it up for either the console player,
// or the secondary splitscreen player
//
static  boolean setupcontrols_secondaryplayer;
static  INT32   (*setupcontrols)[2];  // pointer to the gamecontrols of the player being edited

static void M_ControlDef2(void)
{
	M_SetupNextMenu(&ControlDef2);
}

static void M_DrawJoystick(void)
{
	INT32 i;

	M_DrawGenericMenu();

	for (i = joy0;i < joystickset_end; i++)
	{
		M_DrawSaveLoadBorder(JoystickSetDef.x,JoystickSetDef.y+LINEHEIGHT*i);

		if ((setupcontrols_secondaryplayer && (i == cv_usejoystick2.value))
			|| (!setupcontrols_secondaryplayer && (i == cv_usejoystick.value)))
			V_DrawString(JoystickSetDef.x,JoystickSetDef.y+LINEHEIGHT*i,V_YELLOWMAP,joystickInfo[i]);
		else
			V_DrawString(JoystickSetDef.x,JoystickSetDef.y+LINEHEIGHT*i,0,joystickInfo[i]);
	}
}

static void M_SetupJoystickMenu(INT32 choice)
{
	INT32 i = 0;
	const char *joyname = "None";
	const char *joyNA = "Unavailable";
	INT32 n = I_NumJoys();
	(void)choice;

	strcpy(joystickInfo[i], joyname);

	for (i = joy1; i < joystickset_end; i++)
	{
		if (i <= n && (joyname = I_GetJoyName(i)) != NULL)
		{
			strncpy(joystickInfo[i], joyname, 24);
			joystickInfo[i][24] = '\0';
		}
		else
			strcpy(joystickInfo[i], joyNA);
	}

	M_SetupNextMenu(&JoystickSetDef);
}

static void M_Setup1PJoystickMenu(INT32 choice)
{
	setupcontrols_secondaryplayer = false;
	M_SetupJoystickMenu(choice);
}

static void M_Setup2PJoystickMenu(INT32 choice)
{
	setupcontrols_secondaryplayer = true;
	M_SetupJoystickMenu(choice);
}

static void M_AssignJoystick(INT32 choice)
{
	if (setupcontrols_secondaryplayer)
		CV_SetValue(&cv_usejoystick2, choice);
	else
		CV_SetValue(&cv_usejoystick, choice);
}

static void M_Setup1PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrols_secondaryplayer = false;
	setupcontrols = gamecontrol;        // was called from main Options (for console player, then)
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&ControlDef);
}

static void M_Setup2PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrols_secondaryplayer = true;
	setupcontrols = gamecontrolbis;
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&ControlDef);
}

static void M_DrawControlsGenerics(void)
{
	INT32 x, y, i, cursory = 0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();

	// UGLY HACK!
	if (setupcontrols_secondaryplayer
		&& currentMenu == &ControlDef2)
	{
		for (i = 0; i < 0; i++) //vertical adjustable lines
		{
			if (currentMenu->menuitems[i].alphaKey)
				y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
		}
		if (itemOn < 0) //will stop and not display the item above.
			itemOn = 0;
	}

	for (i = 0; i < currentMenu->numitems; i++)
	{
		// UGLY HACK!
		if (setupcontrols_secondaryplayer
			&& currentMenu == &ControlDef2
			&& i < 0) //vertical adjusted lines.
			continue;

		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
				{
					V_DrawScaledPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
				}
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y += LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;

				if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
					V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
				else
					V_DrawString(x, y, V_YELLOWMAP, currentMenu->menuitems[i].text);

				// Cvar specific handling
				switch (currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
						switch (currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider(x, y, cv);
							case IT_CV_NOPRINT: // color use this
								break;
							case IT_CV_STRING:
								M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
								V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, cv->string);
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter(x + 8 + V_StringWidth(cv->string), y + 12,
										'_' | 0x80,false);
								y += 16;
								break;
							default:
								V_DrawString(BASEVIDWIDTH - x - V_StringWidth(cv->string), y,
									V_YELLOWMAP, cv->string);
								break;
						}
						break;
					}
					y += STRINGHEIGHT;
					break;
			case IT_STRING2:
				V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
			case IT_DYLITLSPACE:
				y += SMALLLINEHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawMappedPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch,PU_CACHE), graymap);
				y += LINEHEIGHT;
				break;
		}
	}

	// DRAW THE SKULL CURSOR
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
		|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
	{
		V_DrawScaledPatch(currentMenu->x + SKULLXOFF, cursory - 5, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
	}
	else
	{
		V_DrawScaledPatch(currentMenu->x - 22, cursory, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawString(currentMenu->x, cursory, V_YELLOWMAP, currentMenu->menuitems[itemOn].text);
	}
}
//
//  Draws the Customise Controls menu
//
static void M_DrawControl(void)
{
	char     tmp[50];
	INT32    i;
	INT32    keys[2];

	// draw title, strings and submenu
	M_DrawControlsGenerics();

	M_CentreText (ControlDef.y-12,
		 (setupcontrols_secondaryplayer ? "SET CONTROLS FOR SECONDARY PLAYER" :
		                                  "PRESS ENTER TO CHANGE, BACKSPACE TO CLEAR"));

	for (i = 0;i < currentMenu->numitems;i++)
	{
		if (currentMenu->menuitems[i].status != IT_CONTROL)
			continue;

		if (setupcontrols_secondaryplayer
			&& currentMenu == &ControlDef2
			&& i < 3)
			continue;

		keys[0] = setupcontrols[currentMenu->menuitems[i].alphaKey][0];
		keys[1] = setupcontrols[currentMenu->menuitems[i].alphaKey][1];

		tmp[0] ='\0';
		if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
		{
			strcpy(tmp, "---");
		}
		else
		{
			if (keys[0] != KEY_NULL)
				strcat (tmp, G_KeynumToString (keys[0]));

			if (keys[0] != KEY_NULL && keys[1] != KEY_NULL)
				strcat(tmp," or ");

			if (keys[1] != KEY_NULL)
				strcat (tmp, G_KeynumToString (keys[1]));


		}
		V_DrawString(BASEVIDWIDTH-ControlDef.x-V_StringWidth(tmp), ControlDef.y + i*8,V_YELLOWMAP, tmp);
	}

}

static INT32 controltochange;

static void M_ChangecontrolResponse(event_t *ev)
{
	INT32        control;
	INT32        found;
	INT32        ch = ev->data1;

	// ESCAPE cancels
	if (ch != KEY_ESCAPE)
	{

		switch (ev->type)
		{
			// ignore mouse/joy movements, just get buttons
			case ev_mouse:
			case ev_mouse2:
			case ev_joystick:
			case ev_joystick2:
				ch = KEY_NULL;      // no key
			break;

			// keypad arrows are converted for the menu in cursor arrows
			// so use the event instead of ch
			case ev_keydown:
				ch = ev->data1;
			break;

			default:
			break;
		}

		control = controltochange;

		// check if we already entered this key
		found = -1;
		if (setupcontrols[control][0] ==ch)
			found = 0;
		else if (setupcontrols[control][1] ==ch)
			found = 1;
		if (found >= 0)
		{
			// replace mouse and joy clicks by double clicks
			if (ch >= KEY_MOUSE1 && ch <= KEY_MOUSE1+MOUSEBUTTONS)
				setupcontrols[control][found] = ch-KEY_MOUSE1+KEY_DBLMOUSE1;
			else if (ch >= KEY_JOY1 && ch <= KEY_JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_JOY1+KEY_DBLJOY1;
			else if (ch >= KEY_2MOUSE1 && ch <= KEY_2MOUSE1+MOUSEBUTTONS)
				setupcontrols[control][found] = ch-KEY_2MOUSE1+KEY_DBL2MOUSE1;
			else if (ch >= KEY_2JOY1 && ch <= KEY_2JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_2JOY1+KEY_DBL2JOY1;
		}
		else
		{
			// check if change key1 or key2, or replace the two by the new
			found = 0;
			if (setupcontrols[control][0] == KEY_NULL)
				found++;
			if (setupcontrols[control][1] == KEY_NULL)
				found++;
			if (found == 2)
			{
				found = 0;
				setupcontrols[control][1] = KEY_NULL;  //replace key 1,clear key2
			}
			G_CheckDoubleUsage(ch);
			setupcontrols[control][found] = ch;
		}

	}

	M_StopMessage(0);
}

static void M_ChangeControl(INT32 choice)
{
	static char tmp[55];

	controltochange = currentMenu->menuitems[choice].alphaKey;
	sprintf(tmp, "Hit the new key for\n%s\nESC for Cancel",
		currentMenu->menuitems[choice].text);

	M_StartMessage(tmp, M_ChangecontrolResponse, MM_EVENTHANDLER);
}

//===========================================================================
//                        VIDEO MODE MENU
//===========================================================================
static void M_DrawVideoMode(void);             //added : 30-01-98:

static void M_HandleVideoMode(INT32 ch);

static menuitem_t VideoModeMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "", M_HandleVideoMode, '\0'},     // dummy menuitem for the control func
};

menu_t VidModeDef =
{
	"M_VIDEO",
	"Video Mode",
	1,                  // # of menu items
	//sizeof (VideoModeMenu)/sizeof (menuitem_t),
	&VideoOptionsDef,   // previous menu
	VideoModeMenu,      // menuitem_t ->
	M_DrawVideoMode,    // drawing routine ->
	48, 36,             // x,y
	0,                  // lastOn
	NULL
};

//added : 30-01-98:
#define MAXCOLUMNMODES   10     //max modes displayed in one column
#define MAXMODEDESCS     (MAXCOLUMNMODES*3)

// shhh... what am I doing... nooooo!
static INT32 vidm_testingmode = 0;
static INT32 vidm_previousmode;
static INT32 vidm_current = 0;
static INT32 vidm_nummodes;
static INT32 vidm_column_size;

typedef struct
{
	INT32 modenum; // video mode number in the vidmodes list
	const char *desc;  // XXXxYYY
	INT32 iscur;   // 1 if it is the current active mode
} modedesc_t;

static modedesc_t modedescs[MAXMODEDESCS];

//
// Draw the video modes list, a-la-Quake
//
static void M_DrawVideoMode(void)
{
	INT32 i, j, vdup, row, col, nummodes;
	const char *desc;
	char temp[80];
	INT32 width, height;

	// draw title
	M_DrawMenuTitle();

#if (defined (__unix__) && !defined (MSDOS)) || defined (UNIXCOMMON) || defined (SDL)
	VID_PrepareModeList(); // FIXME: hack
#endif
	vidm_nummodes = 0;
	nummodes = VID_NumModes();

#ifdef _WINDOWS
	// clean that later: skip windowed mode 0, video modes menu only shows FULL SCREEN modes
	if (nummodes <= NUMSPECIALMODES)
	{
		// put the windowed mode so that there is at least one mode
		modedescs[0].modenum = vid.modenum;
		modedescs[0].desc = VID_GetModeName(vid.modenum);
		modedescs[0].iscur = 1;
		vidm_nummodes = 1;
	}
	for (i = NUMSPECIALMODES; i < nummodes && vidm_nummodes < MAXMODEDESCS; i++)
#else
	// DOS does not skip mode 0, because mode 0 is ALWAYS present
	for (i = 0; i < nummodes && vidm_nummodes < MAXMODEDESCS; i++)
#endif
	{
		desc = VID_GetModeName(i);
		if (desc)
		{
			vdup = 0;

			// when a resolution exists both under VGA and VESA, keep the
			// VESA mode, which is always a higher modenum
			for (j = 0; j < vidm_nummodes; j++)
			{
				if (!strcmp(modedescs[j].desc, desc))
				{
					// mode(0): 320x200 is always standard VGA, not vesa
					if (modedescs[j].modenum)
					{
						modedescs[j].modenum = i;
						vdup = 1;

						if (i == vid.modenum)
							modedescs[j].iscur = 1;
					}
					else
						vdup = 1;

					break;
				}
			}

			if (!vdup)
			{
				modedescs[vidm_nummodes].modenum = i;
				modedescs[vidm_nummodes].desc = desc;
				modedescs[vidm_nummodes].iscur = 0;

				if (i == vid.modenum)
					modedescs[vidm_nummodes].iscur = 1;

				vidm_nummodes++;
			}
		}
	}

	vidm_column_size = (vidm_nummodes+2) / 3;

	row = 41;
	col = VidModeDef.y;
	for (i = 0; i < vidm_nummodes; i++)
	{
		// Pull out the width and height
		sscanf(modedescs[i].desc, "%u%*c%u", &width, &height);

		// Show multiples of 320x200 as green.
		if ((width % BASEVIDWIDTH == 0 && height % BASEVIDHEIGHT == 0) &&
			(width / BASEVIDWIDTH == height / BASEVIDHEIGHT))
			V_DrawString(row, col, modedescs[i].iscur ? V_YELLOWMAP : V_GREENMAP, modedescs[i].desc);
		else
			V_DrawString(row, col, modedescs[i].iscur ? V_YELLOWMAP : 0, modedescs[i].desc);

		col += 8;
		if ((i % vidm_column_size) == (vidm_column_size-1))
		{
			row += 7*13;
			col = 36;
		}
	}

	V_DrawCenteredString(BASEVIDWIDTH/2, 168, V_GREENMAP, "Green modes are recommended.");
	V_DrawCenteredString(BASEVIDWIDTH/2, 176, V_GREENMAP, "Non-green modes are known to cause");
	V_DrawCenteredString(BASEVIDWIDTH/2, 184, V_GREENMAP, "random crashes. Use at own risk.");

	if (vidm_testingmode > 0)
	{
		sprintf(temp, "TESTING MODE %s", modedescs[vidm_current].desc);
		M_CentreText(VidModeDef.y + 80 + 16, temp);
		M_CentreText(VidModeDef.y + 90 + 16, "Please wait 5 seconds...");
	}
	else
	{
		M_CentreText(VidModeDef.y + 60 + 16, "Press ENTER to set mode");
		M_CentreText(VidModeDef.y + 70 + 16, "T to test mode for 5 seconds");

		sprintf(temp, "D to make %s the default", VID_GetModeName(vid.modenum));
		M_CentreText(VidModeDef.y + 80 + 16,temp);

		sprintf(temp, "Current default is %dx%d (%d bits)", cv_scr_width.value,
			cv_scr_height.value, cv_scr_depth.value);
		M_CentreText(VidModeDef.y + 90 + 16,temp);

		M_CentreText(VidModeDef.y + 100 + 16,"Press ESC to exit");
	}

	// Draw the cursor for the VidMode menu
	if (skullAnimCounter < 4) // use the Skull anim counter to blink the cursor
	{
		i = 41 - 10 + ((vidm_current / vidm_column_size)*7*13);
		j = VidModeDef.y + ((vidm_current % vidm_column_size)*8);
		V_DrawCharacter(i - 8, j, '*',false);
	}
}

// special menuitem key handler for video mode list
static void M_HandleVideoMode(INT32 ch)
{
	if (vidm_testingmode > 0)
	{
		// change back to the previous mode quickly
		if (ch == KEY_ESCAPE)
		{
			setmodeneeded = vidm_previousmode + 1;
			vidm_testingmode = 0;
		}
		return;
	}

	switch (ch)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_current++;
			if (vidm_current >= vidm_nummodes)
				vidm_current = 0;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_current--;
			if (vidm_current < 0)
				vidm_current = vidm_nummodes - 1;
			break;

		case KEY_LEFTARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_current -= vidm_column_size;
			if (vidm_current < 0)
				vidm_current = (vidm_column_size*3) + vidm_current;
			if (vidm_current >= vidm_nummodes)
				vidm_current = vidm_nummodes - 1;
			break;

		case KEY_RIGHTARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_current += vidm_column_size;
			if (vidm_current >= (vidm_column_size*3))
				vidm_current %= vidm_column_size;
			if (vidm_current >= vidm_nummodes)
				vidm_current = vidm_nummodes - 1;
			break;

		case KEY_ENTER:
			S_StartSound(NULL, sfx_menu1);
			if (!setmodeneeded) // in case the previous setmode was not finished
				setmodeneeded = modedescs[vidm_current].modenum + 1;
			break;

		case KEY_ESCAPE: // this one same as M_Responder
			if (currentMenu->prevMenu)
				M_SetupNextMenu(currentMenu->prevMenu);
			else
				M_ClearMenus(true);
			break;

		case 'T':
		case 't':
			vidm_testingmode = TICRATE*5;
			vidm_previousmode = vid.modenum;
			if (!setmodeneeded) // in case the previous setmode was not finished
				setmodeneeded = modedescs[vidm_current].modenum + 1;
			break;

		case 'D':
		case 'd':
			// current active mode becomes the default mode.
			SCR_SetDefaultMode();
			break;

		default:
			break;
	}
}

//===========================================================================
//LOAD GAME MENU
//===========================================================================
static void M_DrawLoad(void);

static void M_LoadSelect(INT32 choice);
static void M_PlayWithNoSave(void);

typedef enum
{
	load1,
	load2,
	load3,
	load4,
	load5,
	nosave,
	load_end
} load_e;

static menuitem_t LoadGameMenu[] =
{
	{IT_CALL | IT_NOTHING, "", NULL, M_LoadSelect, '1'},
	{IT_CALL | IT_NOTHING, "", NULL, M_LoadSelect, '2'},
	{IT_CALL | IT_NOTHING, "", NULL, M_LoadSelect, '3'},
	{IT_CALL | IT_NOTHING, "", NULL, M_LoadSelect, '4'},
	{IT_CALL | IT_NOTHING, "", NULL, M_LoadSelect, '5'},
	{IT_CALL | IT_NOTHING, "", NULL, M_PlayWithNoSave, '6'},
};

menu_t LoadDef =
{
	"M_PICKG",
	"Load Game",
	load_end,
	&SinglePlayerDef,
	LoadGameMenu,
	M_DrawLoad,
	80, 54,
	0,
	NULL
};

static void M_DrawGameStats(void)
{
	INT32 ecks;
	saveSlotSelected = itemOn;

	ecks = LoadDef.x + 24;
	M_DrawTextBox(LoadDef.x-8,144, 23, 4);

	if (savegameinfo[saveSlotSelected].lives == -42) // Empty
	{
		V_DrawString(ecks + 16, 152, 0, "EMPTY");
		return;
	}
	else if (saveSlotSelected == 5) //No save option
	{
		V_DrawString(ecks + 16, 152, 0, "NO SAVE");
		return;
	}

	if (savegameinfo[saveSlotSelected].skincolor == 0)
		V_DrawScaledPatch ((INT32)((LoadDef.x+4)*vid.fdupx),(INT32)((144+8)*vid.fdupy), V_NOSCALESTART,W_CachePatchName(skins[savegameinfo[saveSlotSelected].skinnum].faceprefix, PU_CACHE));
	else
	{
		UINT8 *colormap = R_GetTranslationColormap(savegameinfo[saveSlotSelected].skinnum, savegameinfo[saveSlotSelected].skincolor, 0);
		V_DrawMappedPatch ((INT32)((LoadDef.x+4)*vid.fdupx),(INT32)((144+8)*vid.fdupy), V_NOSCALESTART,W_CachePatchName(skins[savegameinfo[saveSlotSelected].skinnum].faceprefix, PU_CACHE), colormap);
	}

	V_DrawString(ecks + 16, 152, 0, savegameinfo[saveSlotSelected].playername);

	if (savegameinfo[saveSlotSelected].gamemap == spstage_end)
		V_DrawString(ecks + 16, 160, 0, "COMPLETED!");
	else
	{
// Don't show the act so people know it saves per-zone.
//	if (savegameinfo[saveSlotSelected].actnum == 0)
		V_DrawString(ecks + 16, 160, 0, va("%s", savegameinfo[saveSlotSelected].levelname));
//	else
//		V_DrawString(ecks + 16, 160, 0, va("%s %d", savegameinfo[saveSlotSelected].levelname, savegameinfo[saveSlotSelected].actnum));
	}

	V_DrawScaledPatch(ecks + 16, 168, 0, W_CachePatchName("CHAOS1", PU_CACHE));
	V_DrawString(ecks + 36, 172, 0, va("x %d", savegameinfo[saveSlotSelected].numemeralds));

	V_DrawScaledPatch(ecks + 64, 169, 0, W_CachePatchName("ONEUP", PU_CACHE));
	V_DrawString(ecks + 84, 172, 0, va("x %d", savegameinfo[saveSlotSelected].lives));

	V_DrawScaledPatch(ecks + 120, 168, 0, W_CachePatchName("CONTINS", PU_CACHE));
	V_DrawString(ecks + 140, 172, 0, va("x %d", savegameinfo[saveSlotSelected].continues));
}

//
// M_LoadGame & Cie.
//
static void M_DrawLoad(void)
{
	INT32 i;

	M_DrawGenericMenu();

	V_DrawCenteredString(BASEVIDWIDTH/2, 40, 0, "Hit backspace to delete a save.");

	for (i = 0; i < load_end - 1; i++) //nosave is the last one.
	{
		M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
		V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,0,va("Save Slot %d", i+1));
	}

	// Option to play with no save.
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,0,"Play Without Saving");

	M_DrawGameStats();
}

//
// User wants to load this game
//
static void M_LoadSelect(INT32 choice)
{
	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}
	else if (modifiedgame && !savemoddata)
	{
		M_DrawTextBox(24,64-4,32,3);

		V_DrawCenteredString(160, 64+4, 0, "Note: Game must be reset to record");
		V_DrawCenteredString(160, 64+16, 0, "statistics or unlock secrets.");
	}

	if (!FIL_ReadFileOK(va(savegamename, choice)))
	{
		// This slot is empty, so start a new game here.
		M_NewGame();
	}
	else if (savegameinfo[saveSlotSelected].gamemap == spstage_end) // Completed
	{
		fromloadgame = saveSlotSelected + 1;
		M_LevelSelect(0);
		pandoralevelselect = false; //this is set to true in the above function.
	}
	else
	{
		G_LoadGame((UINT32)choice, 0);
		M_ClearMenus(true);
	}

	cursaveslot = choice;
}

//
// User wants to play without saving
//
static void M_PlayWithNoSave(void)
{
	if (Playing())
	{
		M_StartMessage(ALREADYPLAYING,M_ExitGameResponse,MM_YESNO);
		return;
	}
	else if (modifiedgame && !savemoddata)
	{
		M_DrawTextBox(24,64-4,32,3);

		V_DrawCenteredString(160, 64+4, 0, "Note: Game must be reset to record");
		V_DrawCenteredString(160, 64+16, 0, "statistics or unlock secrets.");
	}

	// Start a new game here.
	M_NewGame();
	cursaveslot = -1;
}

#define VERSIONSIZE             16
// Reads the save file to list lives, level, player, etc.
// Tails 05-29-2003
static void M_ReadSavegameInfo(UINT32 slot)
{
#define BADSAVE I_Error("Bad savegame in slot %u", slot);
#define CHECKPOS if (save_p >= end_p) BADSAVE
	size_t length;
	char savename[255];
	UINT8 *savebuffer;
	UINT8 *end_p; // buffer end point, don't read past here
	UINT8 *save_p;
	INT32 fake; // Dummy variable
	char temp[sizeof(timeattackfolder)];

	sprintf(savename, savegamename, slot);

	length = FIL_ReadFile(savename, &savebuffer);
	if (length == 0)
	{
		CONS_Printf("%s %s", M_GetText("[Message unsent]\n"), savename);
		savegameinfo[slot].lives = -42;
		return;
	}

	end_p = savebuffer + length;

	// skip the description field
	save_p = savebuffer;

	save_p += VERSIONSIZE;

	// dearchive all the modifications
	// P_UnArchiveMisc()

	CHECKPOS
	fake = READINT16(save_p);
	if (fake-1 >= NUMMAPS) BADSAVE

	if(!mapheaderinfo[fake-1])
		P_AllocMapHeader((INT16)(fake-1));

	strcpy(savegameinfo[slot].levelname, mapheaderinfo[fake-1]->lvlttl);
	savegameinfo[slot].gamemap = fake;

	savegameinfo[slot].actnum = mapheaderinfo[fake-1]->actnum;

	CHECKPOS
	fake = READUINT16(save_p)-357; // emeralds

	savegameinfo[slot].numemeralds = 0;

	if (fake & EMERALD1)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD2)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD3)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD4)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD5)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD6)
		savegameinfo[slot].numemeralds++;
	if (fake & EMERALD7)
		savegameinfo[slot].numemeralds++;

	CHECKPOS
	READSTRINGN(save_p, temp, sizeof(temp)); // mod it belongs to

	// P_UnArchivePlayer()
	CHECKPOS
	savegameinfo[slot].skincolor = READUINT8(save_p);
	CHECKPOS
	savegameinfo[slot].skinnum = READUINT8(save_p);
	strcpy(savegameinfo[slot].playername,
		skins[savegameinfo[slot].skinnum].name);

	CHECKPOS
	(void)READINT32(save_p); // Score

	CHECKPOS
	savegameinfo[slot].lives = READINT32(save_p); // lives
	CHECKPOS
	savegameinfo[slot].continues = READINT32(save_p); // continues

	// done
	Z_Free(savebuffer);
#undef CHECKPOS
#undef BADSAVE
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//  and put it in savegamestrings global variable
//
static void M_ReadSaveStrings(void)
{
	FILE *handle;
	UINT32 i;
	char name[256];

	for (i = 0; i < load_end - 1; i++) //nosave is the last one.
	{
		snprintf(name, sizeof name, savegamename, i);
		name[sizeof name - 1] = '\0';

		handle = fopen(name, "rb");
		if (handle == NULL)
		{
			LoadGameMenu[i].status = 0;
			savegameinfo[i].lives = -42;
			continue;
		}
		fclose(handle);
		LoadGameMenu[i].status = 1;
		M_ReadSavegameInfo(i);
	}
}

static INT32 curSaveSelected;

//
// User wants to delete this game
//
static void M_SaveGameDeleteResponse(INT32 ch)
{
	char name[256];

	if (ch != 'y')
		return;

	// delete savegame
	snprintf(name, sizeof name, savegamename, curSaveSelected);
	name[sizeof name - 1] = '\0';
	remove(name);

	// Refresh savegame menu info
	M_ReadSaveStrings();
}

//
// Selected from SRB2 menu
//
static void M_LoadGame(INT32 choice)
{
	(void)choice;
	// change can't load message to can't load in server mode
	if (netgame && !server)
	{
		M_StartMessage(M_GetText("Only the server can do a load net game!\n\npress a key."), NULL, MM_NOTHING);
		return;
	}

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}

//
// Draw border for the savegame description
//
static void M_DrawSaveLoadBorder(INT32 x,INT32 y)
{
	INT32 i;

	V_DrawScaledPatch (x-8,y+7,0,W_CachePatchName("M_LSLEFT",PU_CACHE));

	for (i = 0;i < 24;i++)
	{
		V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSCNTR",PU_CACHE));
		x += 8;
	}

	V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSRGHT",PU_CACHE));
}

//===========================================================================
//                                 END GAME
//===========================================================================

//
// M_EndGame
//
static void M_EndGameResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	currentMenu->lastOn = itemOn;
	M_ClearMenus(true);
	//Command_ExitGame_f();
	G_SetExitGameFlag();
}

void M_EndGame(INT32 choice)
{
	(void)choice;
	if (demoplayback || demorecording)
		return;

	if (!Playing())
		return;

	M_StartMessage(M_GetText("Are you sure you want to end the game?\n\npress Y or N."), M_EndGameResponse, MM_YESNO);
}

//===========================================================================
//                                 Quit Game
//===========================================================================

//
// M_QuitSRB2
//
static INT32 quitsounds2[8] =
{
	sfx_spring, // Tails 11-09-99
	sfx_itemup, // Tails 11-09-99
	sfx_jump, // Tails 11-09-99
	sfx_pop,
	sfx_gloop, // Tails 11-09-99
	sfx_splash, // Tails 11-09-99
	sfx_floush, // Tails 11-09-99
	sfx_chchng // Tails 11-09-99
};

void M_ExitGameResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	//Command_ExitGame_f();
	G_SetExitGameFlag();
}

void M_QuitResponse(INT32 ch)
{
	tic_t ptime;
	if (ch != 'y' && ch != KEY_ENTER)
		return;
	if (!(netgame || cv_debug))
	{
		if (quitsounds2[(gametic>>2)&7]) S_StartSound(NULL, quitsounds2[(gametic>>2)&7]); // Use quitsounds2, not quitsounds Tails 11-09-99

		//added : 12-02-98: do that instead of I_WaitVbl which does not work
		ptime = I_GetTime() + TICRATE*3; // Shortened the quit time, used to be 2 seconds Tails 03-26-2001
		while (ptime > I_GetTime())
		{
			V_DrawScaledPatch(0, 0, 0, W_CachePatchName("GAMEQUIT", PU_CACHE)); // Demo 3 Quit Screen Tails 06-16-2001
			I_FinishUpdate(); // Update the screen with the image Tails 06-19-2001
			I_Sleep();
		}
	}
	I_Quit();
}

static void M_QuitSRB2(INT32 choice)
{
	// We pick index 0 which is language sensitive, or one at random,
	// between 1 and maximum number.
	static char s[200];
	(void)choice;
	sprintf(s, M_GetText("%s\n\n(Press 'Y' to quit)"), quitmsg[QUITMSG + (gametic % NUM_QUITMESSAGES)]);
	M_StartMessage(s, M_QuitResponse, MM_YESNO);
}

//===========================================================================
//                              Some Draw routine
//===========================================================================

//
// Menu Functions
//
static void M_DrawThermo(INT32 x, INT32 y, consvar_t *cv)
{
	INT32 xx = x, i;
	lumpnum_t leftlump, rightlump, centerlump[2], cursorlump;
	patch_t *p;

	leftlump = W_GetNumForName("M_THERML");
	rightlump = W_GetNumForName("M_THERMR");
	centerlump[0] = W_GetNumForName("M_THERMM");
	centerlump[1] = W_GetNumForName("M_THERMM");
	cursorlump = W_GetNumForName("M_THERMO");

	V_DrawScaledPatch(xx, y, 0, p = W_CachePatchNum(leftlump,PU_CACHE));
	xx += SHORT(p->width) - SHORT(p->leftoffset);
	for (i = 0; i < 16; i++)
	{
		V_DrawScaledPatch(xx, y, V_WRAPX, W_CachePatchNum(centerlump[i & 1], PU_CACHE));
		xx += 8;
	}
	V_DrawScaledPatch(xx, y, 0, W_CachePatchNum(rightlump, PU_CACHE));

	xx = (cv->value - cv->PossibleValue[0].value) * (15*8) /
		(cv->PossibleValue[1].value - cv->PossibleValue[0].value);

	V_DrawScaledPatch((x + 8) + xx, y, 0, W_CachePatchNum(cursorlump, PU_CACHE));
}

//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
//added : 06-02-98:
void M_DrawTextBox(INT32 x, INT32 y, INT32 width, INT32 boxlines)
{
	patch_t *p;
	INT32 cx = x, cy = y, n;
	INT32 step = 8, boff = 8;

	// draw left side
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TL], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_L], PU_CACHE);
	for (n = 0; n < boxlines; n++)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPY, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BL], PU_CACHE));

	// draw middle
	V_DrawFlatFill(x + boff, y + boff, width*step, boxlines*step, st_borderpatchnum);

	cx += boff;
	cy = y;
	while (width > 0)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPX, W_CachePatchNum(viewborderlump[BRDR_T], PU_CACHE));
		V_DrawScaledPatch(cx, y + boff + boxlines*step, V_WRAPX, W_CachePatchNum(viewborderlump[BRDR_B], PU_CACHE));
		width--;
		cx += step;
	}

	// draw right side
	cy = y;
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TR], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_R], PU_CACHE);
	for (n = 0; n < boxlines; n++)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPY, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BR], PU_CACHE));
}

//==========================================================================
//                        Message is now a (hackable) Menu
//==========================================================================
static void M_DrawMessageMenu(void);

static menuitem_t MessageMenu[] =
{
	// TO HACK
	{0,NULL, NULL, NULL,0}
};

menu_t MessageDef =
{
	NULL,               // title
	NULL,
	1,                  // # of menu items
	NULL,               // previous menu       (TO HACK)
	MessageMenu,        // menuitem_t ->
	M_DrawMessageMenu,  // drawing routine ->
	0, 0,               // x, y                (TO HACK)
	0,                  // lastOn, flags       (TO HACK)
	NULL
};


void M_StartMessage(const char *string, void *routine,
	menumessagetype_t itemtype)
{
	size_t max = 0, start = 0, i, strlines;
	static char *message = NULL;
	Z_Free(message);
	message = Z_StrDup(string);
	DEBFILE(message);

	M_StartControlPanel(); // can't put menuactive to true

	if (currentMenu == &MessageDef) // Prevent recursion
		MessageDef.prevMenu = &MainDef;
	else
		MessageDef.prevMenu = currentMenu;

	MessageDef.menuitems[0].text     = message;
	MessageDef.menuitems[0].alphaKey = (UINT8)itemtype;
	if (!routine && itemtype != MM_NOTHING) itemtype = MM_NOTHING;
	switch (itemtype)
	{
		case MM_NOTHING:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = M_StopMessage;
			break;
		case MM_YESNO:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
		case MM_EVENTHANDLER:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
	}
	//added : 06-02-98: now draw a textbox around the message
	// compute lenght max and the numbers of lines
	for (strlines = 0; *(message+start); strlines++)
	{
		for (i = 0;i < strlen(message+start);i++)
		{
			if (*(message+start+i) == '\n')
			{
				if (i > max)
					max = i;
				start += i;
				i = (size_t)-1; //added : 07-02-98 : damned!
				start++;
				break;
			}
		}

		if (i == strlen(message+start))
			start += i;
	}

	MessageDef.x = (INT16)((BASEVIDWIDTH  - 8*max-16)/2);
	MessageDef.y = (INT16)((BASEVIDHEIGHT - M_StringHeight(message))/2);

	MessageDef.lastOn = (INT16)((strlines<<8)+max);

	//M_SetupNextMenu();
	currentMenu = &MessageDef;
	itemOn = 0;
}

#define MAXMSGLINELEN 256

static void M_DrawMessageMenu(void)
{
	INT32 y = currentMenu->y;
	size_t i, start = 0;
	INT16 max;
	char string[MAXMSGLINELEN];
	INT32 mlines;
	const char *msg = currentMenu->menuitems[0].text;

	mlines = currentMenu->lastOn>>8;
	max = (INT16)((UINT8)(currentMenu->lastOn & 0xFF)*8);
	M_DrawTextBox(currentMenu->x, y - 8, (max+7)>>3, mlines);

	while (*(msg+start))
	{
		size_t len = strlen(msg+start);

		for (i = 0; i < len; i++)
		{
			if (*(msg+start+i) == '\n')
			{
				memset(string, 0, MAXMSGLINELEN);
				if (i >= MAXMSGLINELEN)
				{
					DEBPRINT(va("M_DrawMessageMenu: too long segment in %s\n", msg));
					return;
				}
				else
				{
					strncpy(string,msg+start, i);
					string[i] = '\0';
					start += i;
					i = (size_t)-1; //added : 07-02-98 : damned!
					start++;
				}
				break;
			}
		}

		if (i == strlen(msg+start))
		{
			if (i >= MAXMSGLINELEN)
			{
				DEBPRINT(va("M_DrawMessageMenu: too long segment in %s\n", msg));
				return;
			}
			else
			{
				strcpy(string, msg + start);
				start += i;
			}
		}

		V_DrawString((BASEVIDWIDTH - V_StringWidth(string))/2,y,0,string);
		y += 8; //SHORT(hu_font[0]->height);
	}
}

// default message handler
static void M_StopMessage(INT32 choice)
{
	(void)choice;
	M_SetupNextMenu(MessageDef.prevMenu);
}

//==========================================================================
//                        Menu stuffs
//==========================================================================

//added : 30-01-98:
//
//  Write a string centered using the hu_font
//
static void M_CentreText(INT32 y, const char *string)
{
	INT32 x;
	//added : 02-02-98 : centre on 320, because V_DrawString centers on vid.width...
	x = (BASEVIDWIDTH - V_StringWidth(string))>>1;
	V_DrawString(x,y,0,string);
}


//
// CONTROL PANEL
//

static void M_ChangeCvar(INT32 choice)
{
	consvar_t *cv = (consvar_t *)currentMenu->menuitems[itemOn].itemaction;

	if (((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER)
	    ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD))
	{
		CV_SetValue(cv,cv->value+choice*2-1);
	}
	else if (cv->flags & CV_FLOAT)
	{
		char s[20];
		sprintf(s,"%f",FIXED_TO_FLOAT(cv->value)+(choice*2-1)*(1.0f/16.0f));
		CV_Set(cv,s);
	}
	else
		CV_AddValue(cv,choice*2-1);
}

static boolean M_ChangeStringCvar(INT32 choice)
{
	consvar_t *cv = (consvar_t *)currentMenu->menuitems[itemOn].itemaction;
	char buf[255];
	size_t len;

	switch (choice)
	{
		case KEY_BACKSPACE:
			len = strlen(cv->string);
			if (len > 0)
			{
				M_Memcpy(buf, cv->string, len);
				buf[len-1] = 0;
				CV_Set(cv, buf);
			}
			return true;
		default:
			if (choice >= 32 && choice <= 127)
			{
				len = strlen(cv->string);
				if (len < MAXSTRINGLENGTH - 1)
				{
					M_Memcpy(buf, cv->string, len);
					buf[len++] = (char)choice;
					buf[len] = 0;
					CV_Set(cv, buf);
				}
				return true;
			}
			break;
	}
	return false;
}

//
// M_Responder
//
boolean M_Responder(event_t *ev)
{
	INT32 ch = -1;
//	INT32 i;
	static tic_t joywait = 0, mousewait = 0;
	static boolean shiftdown = false;
	static INT32 pmousex = 0, pmousey = 0;
	static INT32 lastx = 0, lasty = 0;
	void (*routine)(INT32 choice); // for some casting problem

	if (dedicated || gamestate == GS_INTRO || gamestate == GS_INTRO2 || gamestate == GS_CUTSCENE)
		return false;

	if (ev->type == ev_keyup && (ev->data1 == KEY_LSHIFT || ev->data1 == KEY_RSHIFT))
	{
		shiftdown = false;
		return false;
	}
	else if (ev->type == ev_keydown)
	{
		ch = ev->data1;

		// added 5-2-98 remap virtual keys (mouse & joystick buttons)
		switch (ch)
		{
			case KEY_LSHIFT:
			case KEY_RSHIFT:
				shiftdown = true;
				break; //return false;
			case KEY_MOUSE1:
			case KEY_JOY1:
			case KEY_JOY1 + 2:
				ch = KEY_ENTER;
				break;
			case KEY_JOY1 + 3:
				ch = 'n';
				break;
			case KEY_MOUSE1 + 1:
			case KEY_JOY1 + 1:
				ch = KEY_BACKSPACE;
				break;
			case KEY_HAT1:
				ch = KEY_UPARROW;
				break;
			case KEY_HAT1 + 1:
				ch = KEY_DOWNARROW;
				break;
			case KEY_HAT1 + 2:
				ch = KEY_LEFTARROW;
				break;
			case KEY_HAT1 + 3:
				ch = KEY_RIGHTARROW;
				break;
		}
	}
	else if (menuactive)
	{
		if (ev->type == ev_joystick  && ev->data1 == 0 && joywait < I_GetTime())
		{
			if (ev->data3 == -1)
			{
				ch = KEY_UPARROW;
				joywait = I_GetTime() + TICRATE/7;
			}
			else if (ev->data3 == 1)
			{
				ch = KEY_DOWNARROW;
				joywait = I_GetTime() + TICRATE/7;
			}

			if (ev->data2 == -1)
			{
				ch = KEY_LEFTARROW;
				joywait = I_GetTime() + TICRATE/17;
			}
			else if (ev->data2 == 1)
			{
				ch = KEY_RIGHTARROW;
				joywait = I_GetTime() + TICRATE/17;
			}
		}
		else if (ev->type == ev_mouse && mousewait < I_GetTime())
		{
			pmousey += ev->data3;
			if (pmousey < lasty-30)
			{
				ch = KEY_DOWNARROW;
				mousewait = I_GetTime() + TICRATE/7;
				pmousey = lasty -= 30;
			}
			else if (pmousey > lasty + 30)
			{
				ch = KEY_UPARROW;
				mousewait = I_GetTime() + TICRATE/7;
				pmousey = lasty += 30;
			}

			pmousex += ev->data2;
			if (pmousex < lastx - 30)
			{
				ch = KEY_LEFTARROW;
				mousewait = I_GetTime() + TICRATE/7;
				pmousex = lastx -= 30;
			}
			else if (pmousex > lastx+30)
			{
				ch = KEY_RIGHTARROW;
				mousewait = I_GetTime() + TICRATE/7;
				pmousex = lastx += 30;
			}
		}
	}

	if (ch == -1)
		return false;

	// F-Keys
	if (!menuactive || ch == KEY_F8) //allow screenshots
	{
		switch (ch)
		{
			case KEY_F1: // Help key
				M_StartControlPanel();
				currentMenu = &ReadDef1;
				itemOn = 0;
				return true;

			case KEY_F2: // Empty
				return true;

			case KEY_F3: // Empty
				return true;

			case KEY_F4: // Sound Volume
				M_StartControlPanel();
				currentMenu = &SoundDef;
				itemOn = sfx_vol;
				return true;

#ifndef DC
			case KEY_F5: // Video Mode
				M_StartControlPanel();
				M_SetupNextMenu(&VidModeDef);
				return true;
#endif

			case KEY_F6: // Empty
				return true;

			case KEY_F7: // Options
				M_StartControlPanel();
				M_OptionsMenu(0);
				return true;

			case KEY_F8: // Screenshot
				COM_ImmedExecute("screenshot\n");
				return true;

			case KEY_F9: // Empty
				return true;

			case KEY_F10: // Quit SRB2
				M_QuitSRB2(0);
				return true;

			case KEY_F11: // Gamma Level
				CV_AddValue(&cv_usegamma, 1);
				return true;

			case KEY_ESCAPE: // Pop up menu
				if (chat_on)
				{
					HU_clearChatChars();
					chat_on = false;
				}
				else
					M_StartControlPanel();
				return true;
		}
		return false;
	}

	routine = currentMenu->menuitems[itemOn].itemaction;

	// Handle menuitems which need a specific key handling
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER)
	{
		if (shiftdown && ch >= 32 && ch <= 127)
			ch = shiftxform[ch];
		routine(ch);
		return true;
	}

	if (currentMenu->menuitems[itemOn].status == IT_MSGHANDLER)
	{
		if (currentMenu->menuitems[itemOn].alphaKey != MM_EVENTHANDLER)
		{
			if (ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE || ch == KEY_ENTER)
			{
				if (routine)
					routine(ch);
				M_StopMessage(0);
				return true;
			}
			return true;
		}
		else
		{
			// dirty hack: for customising controls, I want only buttons/keys, not moves
			if (ev->type == ev_mouse || ev->type == ev_mouse2 || ev->type == ev_joystick
				|| ev->type == ev_joystick2)
				return true;
			if (routine)
			{
				void (*otherroutine)(event_t *sev) = currentMenu->menuitems[itemOn].itemaction;
				otherroutine(ev); //Alam: what a hack
			}
			return true;
		}
	}

	// BP: one of the more big hack i have never made
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR)
	{
		if ((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_STRING)
		{
			if (shiftdown && ch >= 32 && ch <= 127)
				ch = shiftxform[ch];
			if (M_ChangeStringCvar(ch))
				return true;
			else
				routine = NULL;
		}
		else
			routine = M_ChangeCvar;
	}

	// Keys usable within menu
	switch (ch)
	{
		case KEY_DOWNARROW:
			do
			{
				if (itemOn + 1 > currentMenu->numitems - 1)
					itemOn = 0;
				else
					itemOn++;
			} while ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE);

			S_StartSound(NULL, sfx_menu1);
			return true;

		case KEY_UPARROW:
			do
			{
				if (!itemOn)
					itemOn = (INT16)(currentMenu->numitems - 1);
				else
					itemOn--;
			} while ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE);

			S_StartSound(NULL, sfx_menu1);
			return true;

		case KEY_LEFTARROW:
			if (routine && ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				|| (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				if (currentMenu != &SoundDef)
					S_StartSound(NULL, sfx_menu1);
				routine(0);
			}
			return true;

		case KEY_RIGHTARROW:
			if (routine && ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				|| (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				if (currentMenu != &SoundDef)
					S_StartSound(NULL, sfx_menu1);
				routine(1);
			}
			return true;

		case KEY_ENTER:
			currentMenu->lastOn = itemOn;
			if (routine)
			{
				switch (currentMenu->menuitems[itemOn].status & IT_TYPE)
				{
					case IT_CVAR:
					case IT_ARROWS:
						routine(1); // right arrow
						S_StartSound(NULL, sfx_menu1);
						break;
					case IT_CALL:
						routine(itemOn);
						S_StartSound(NULL, sfx_menu1);
						break;
					case IT_SUBMENU:
						currentMenu->lastOn = itemOn;
						M_SetupNextMenu((menu_t *)currentMenu->menuitems[itemOn].itemaction);
						S_StartSound(NULL, sfx_menu1);
						break;
				}
			}
			return true;

		case KEY_ESCAPE:
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				//If we entered the game search menu, but didn't enter a game,
				//make sure the game doesn't still think we're in a netgame.
				if (!Playing() && netgame && multiplayer)
				{
					MSCloseUDPSocket();		// Clean up so we can re-open the connection later.
					netgame = false;
					multiplayer = false;
				}
				// Catch Switch Map option in case we quit a game using the menu somewhere...
				if (!(netgame || multiplayer) || !Playing()
					|| !(server || adminplayer == consoleplayer))
				{
					MainMenu[switchmap].status = IT_DISABLED;
					MainMenu[scramble].status = IT_DISABLED;
				}
				else
				{
					MainMenu[switchmap].status = IT_STRING | IT_CALL;

					if((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
						MainMenu[scramble].status = IT_STRING | IT_CALL;
				}

				// Make sure the Switch Team / Spectate option only shows up in gametypes that apply.
				if (!(gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF)
					|| splitscreen || !(netgame || multiplayer) || !Playing())
				{
					MainMenu[spectate].status = IT_DISABLED;
					MainMenu[switchteam].status = IT_DISABLED;
					MainMenu[scramble].status = IT_DISABLED;
				}
				else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
				{
					MainMenu[spectate].status = IT_DISABLED;
					MainMenu[switchteam].status = IT_STRING | IT_CALL;

					if(server || adminplayer == consoleplayer)
						MainMenu[scramble].status = IT_STRING | IT_CALL;
				}
				else
				{
					MainMenu[spectate].status = IT_STRING | IT_CALL;
					MainMenu[switchteam].status = IT_DISABLED;
					MainMenu[scramble].status = IT_DISABLED;
				}

				if (currentMenu == &TimeAttackDef)
				{
					// Fade to black first
					if (rendermode != render_none)
					{
						V_DrawFill(0, 0, vid.width, vid.height, 31);
						F_WipeEndScreen(0, 0, vid.width, vid.height);

						F_RunWipe(2*TICRATE, false);
					}
					menuactive = false;
					D_StartTitle();
				}
				else if (currentMenu == &LevelSelectDef)
				{
					// Don't let people backdoor their way into Pandora's Box if they havn't earned it.
					if (pandoralevelselect)
					{
						currentMenu = &SecretsDef;
						itemOn = currentMenu->lastOn;
					}
					else
					{
						currentMenu = &LoadDef;
						itemOn = currentMenu->lastOn;
					}
				}
				else
				{
					currentMenu = currentMenu->prevMenu;
					itemOn = currentMenu->lastOn;
				}
			}
			else
				M_ClearMenus(true);

			return true;

		case KEY_BACKSPACE:
			if ((currentMenu->menuitems[itemOn].status) == IT_CONTROL)
			{
				// detach any keys associated with the game control
				G_ClearControlKeys(setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
				return true;
			}
			else if (currentMenu == &LoadDef)
			{
				if (curSaveSelected != 5) //Don't delete the "No Save" option.
				{
					curSaveSelected = itemOn; // Eww eww!
					M_StartMessage("Are you sure you want to delete\nthis save game?\n(Y/N)\n",M_SaveGameDeleteResponse,MM_YESNO);
					return true;
				}
			}
			else if (currentMenu == &LevelSelectDef)
			{
				// Don't let people backdoor their way into Pandora's Box if they havn't earned it.
				if (pandoralevelselect)
				{
					currentMenu = &SecretsDef;
					itemOn = currentMenu->lastOn;
				}
				else
				{
					currentMenu = &LoadDef;
					itemOn = currentMenu->lastOn;
				}
				return true;
			}
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				currentMenu = currentMenu->prevMenu;
				itemOn = currentMenu->lastOn;
			}
			return true;

		default:
/*			for (i = itemOn + 1; i < currentMenu->numitems; i++)
				if (currentMenu->menuitems[i].alphaKey == ch && !(currentMenu->menuitems[i].status & IT_DISABLED))
				{
					itemOn = (INT16)i;
					S_StartSound(NULL, sfx_menu1);
					return true;
				}
			for (i = 0; i <= itemOn; i++)
				if (currentMenu->menuitems[i].alphaKey == ch && !(currentMenu->menuitems[i].status & IT_DISABLED))
				{
					itemOn = (INT16)i;
					S_StartSound(NULL, sfx_menu1);
					return true;
				}*/
			break;
	}

	return true;
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
	if (currentMenu == &MessageDef)
		menuactive = true;

	if (!menuactive)
		return;

	// now that's more readable with a faded background (yeah like Quake...)
	if (!WipeInAction)
		V_DrawFadeScreen();

	if (currentMenu->drawroutine)
		currentMenu->drawroutine(); // call current menu Draw routine

	// Draw version down in corner
	if (customversionstring[0] != '\0')
		V_DrawString(0, BASEVIDHEIGHT - 8, V_TRANSLUCENT, customversionstring);
	else
		V_DrawString(0, BASEVIDHEIGHT - 8, V_TRANSLUCENT, VERSIONSTRING);
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
	// intro might call this repeatedly
	if (menuactive)
	{
		CON_ToggleOff(); // move away console
		return;
	}

	menuactive = true;

	if (!(netgame || multiplayer) || !Playing()
		|| !(server || adminplayer == consoleplayer))
	{
		MainMenu[switchmap].status = IT_DISABLED;
		MainMenu[scramble].status = IT_DISABLED;
	}
	else
	{
		MainMenu[switchmap].status = IT_STRING | IT_CALL;

		if((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
			MainMenu[scramble].status = IT_STRING | IT_CALL;
	}

	if (!(gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF) || splitscreen)
	{
		MainMenu[spectate].status = IT_DISABLED;
		MainMenu[switchteam].status = IT_DISABLED;
		MainMenu[scramble].status = IT_DISABLED;
	}
	else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
	{
		MainMenu[spectate].status = IT_DISABLED;
		MainMenu[switchteam].status = IT_STRING | IT_CALL;
	}
	else
	{
		MainMenu[spectate].status = IT_STRING | IT_CALL;
		MainMenu[switchteam].status = IT_DISABLED;
	}

	MainMenu[secrets].status = IT_DISABLED;

	// Check for the ??? menu
	if (grade > 0)
		MainMenu[secrets].status = IT_STRING | IT_CALL;

	if (savemoddata)
		MainMenu[secrets].itemaction = M_CustomSecretsMenu;
	else
		MainMenu[secrets].itemaction = M_SecretsMenu;

	currentMenu = &MainDef;
	itemOn = singleplr;

	CON_ToggleOff(); // move away console

	if (timeattacking) // Cancel recording
	{
		G_CheckDemoStatus();

		if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
			Command_ExitGame_f();

		currentMenu = &TimeAttackDef;
		itemOn = currentMenu->lastOn;
		timeattacking = false;
		G_SetGamestate(GS_TIMEATTACK);
		S_ChangeMusic(mus_racent, true);
		CV_AddValue(&cv_nextmap, 1);
		CV_AddValue(&cv_nextmap, -1);
		return;
	}
}

//
// M_ClearMenus
//
void M_ClearMenus(boolean callexitmenufunc)
{
	if (!menuactive)
		return;

	if (currentMenu->quitroutine && callexitmenufunc && !currentMenu->quitroutine())
		return; // we can't quit this menu (also used to set parameter from the menu)

#ifndef DC // Save the config file. I'm sick of crashing the game later and losing all my changes!
	COM_BufAddText(va("saveconfig \"%s\" -silent\n", configfile));
#endif //Alam: But not on the Dreamcast's VMUs

	if (currentMenu != &MessageDef)
		menuactive = false;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
	INT32 i;

	if (currentMenu->quitroutine)
	{
		if (!currentMenu->quitroutine())
			return; // we can't quit this menu (also used to set parameter from the menu)
	}
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;

	// in case of...
	if (itemOn >= currentMenu->numitems)
		itemOn = (INT16)(currentMenu->numitems - 1);

	// the curent item can be disabled,
	// this code go up until an enabled item found
	if (currentMenu->menuitems[itemOn].status == IT_DISABLED)
	{
		for (i = 0; i < currentMenu->numitems; i++)
		{
			if ((currentMenu->menuitems[i].status != IT_DISABLED))
			{
				itemOn = (INT16)i;
				break;
			}
		}
	}
}

//
// M_Ticker
//
void M_Ticker(void)
{
	if (dedicated)
		return;

	if (--skullAnimCounter <= 0)
		skullAnimCounter = 8 * NEWTICRATERATIO;

	//added : 30-01-98 : test mode for five seconds
	if (vidm_testingmode > 0)
	{
		// restore the previous video mode
		if (--vidm_testingmode == 0)
			setmodeneeded = vidm_previousmode + 1;
	}
}

//
// M_Init
//
void M_Init(void)
{
	CV_RegisterVar(&cv_nextmap);
	CV_RegisterVar(&cv_newgametype);
	CV_RegisterVar(&cv_chooseskin);

	if (dedicated)
		return;

	quitmsg[QUITMSG] = M_GetText("Eggman's tied explosives\nto your girlfriend, and\nwill activate them if\nyou press the 'Y' key!\nPress 'N' to save her!");
	quitmsg[QUITMSG1] = M_GetText("What would Tails say if\nhe saw you quitting the game?");
	quitmsg[QUITMSG2] = M_GetText("Hey!\nWhere do ya think you're goin'?");
	quitmsg[QUITMSG3] = M_GetText("Forget your studies!\nPlay some more!");
	quitmsg[QUITMSG4] = M_GetText("You're trying to say you\nlike Sonic 2K6 better than\nthis, right?");
	quitmsg[QUITMSG5] = M_GetText("Don't leave yet -- there's a\nsuper emerald around that corner!");
	quitmsg[QUITMSG6] = M_GetText("You'd rather work than play?");
	quitmsg[QUITMSG7] = M_GetText("Go ahead and leave. See if I care...\n*sniffle*");

	quitmsg[QUIT2MSG] = M_GetText("If you leave now,\nEggman will take over the world!");
	quitmsg[QUIT2MSG1] = M_GetText("Don't quit!\nThere are animals\nto save!");
	quitmsg[QUIT2MSG2] = M_GetText("Aw c'mon, just bop\na few more robots!");
	quitmsg[QUIT2MSG3] = M_GetText("Did you get all those Chaos Emeralds?");
	quitmsg[QUIT2MSG4] = M_GetText("If you leave, I'll use\nmy spin attack on you!");
	quitmsg[QUIT2MSG5] = M_GetText("Don't go!\nYou might find the hidden\nlevels!");
	quitmsg[QUIT2MSG6] = M_GetText("Hit the 'N' key, Sonic!\nThe 'N' key!");

	quitmsg[QUIT3MSG] = M_GetText("Are you really going to\ngive up?\nWe certainly would never give you up.");
	quitmsg[QUIT3MSG1] = M_GetText("Come on, just ONE more netgame!");
	quitmsg[QUIT3MSG2] = M_GetText("Press 'N' to unlock\nthe Ultimate Cheat!");
	quitmsg[QUIT3MSG3] = M_GetText("Why don't you go back and try\njumping on that house to\nsee what happens?");
	quitmsg[QUIT3MSG4] = M_GetText("Every time you press 'Y', an\nSRB2 Developer cries...");
	quitmsg[QUIT3MSG5] = M_GetText("You'll be back to play soon, though...\n......right?");
	quitmsg[QUIT3MSG6] = M_GetText("Aww, is Egg Rock Zone too\ndifficult for you?");

	// This is used because DOOM 2 had only one HELP
	//  page. I use CREDIT as second page now, but
	//  kept this hack for educational purposes.
	ReadMenu1[0].itemaction = &MainDef;

#ifndef NONET
	CV_RegisterVar(&cv_serversearch);
	CV_RegisterVar(&cv_serversort);
	CV_RegisterVar(&cv_chooseroom);
#endif
	//todo put this somewhere better...
	CV_RegisterVar(&cv_allcaps);
}

//======================================================================
// OpenGL specific options
//======================================================================

#ifdef HWRENDER

static void M_DrawOpenGLMenu(void);
static void M_OGL_DrawFogMenu(void);
static void M_OGL_DrawColorMenu(void);
static void M_HandleFogColor(INT32 choice);

static menuitem_t OpenGLOptionsMenu[] =
{
	{IT_STRING|IT_CVAR,         NULL, "Field of view",   &cv_grfov,            10},
	{IT_STRING|IT_CVAR,         NULL, "Quality",         &cv_scr_depth,        20},
	{IT_STRING|IT_CVAR,         NULL, "Texture Filter",  &cv_grfiltermode,     30},
	{IT_STRING|IT_CVAR,         NULL, "Anisotropic",     &cv_granisotropicmode,40},
#ifdef _WINDOWS
	{IT_STRING|IT_CVAR,         NULL, "Fullscreen",      &cv_fullscreen,       50},
#endif
	{IT_STRING|IT_CVAR|IT_CV_SLIDER,
	                            NULL, "Translucent HUD", &cv_grtranslucenthud, 60},
#ifdef ALAM_LIGHTING
	{IT_SUBMENU|IT_WHITESTRING, NULL, "Lighting...",     &OGL_LightingDef,     70},
#endif
	{IT_SUBMENU|IT_WHITESTRING, NULL, "Fog...",          &OGL_FogDef,          80},
	{IT_SUBMENU|IT_WHITESTRING, NULL, "Gamma...",        &OGL_ColorDef,        90},
};

#ifdef ALAM_LIGHTING
static menuitem_t OGL_LightingMenu[] =
{
	{IT_STRING|IT_CVAR, NULL, "Coronas",          &cv_grcoronas,          0},
	{IT_STRING|IT_CVAR, NULL, "Coronas size",     &cv_grcoronasize,      10},
	{IT_STRING|IT_CVAR, NULL, "Dynamic lighting", &cv_grdynamiclighting, 20},
	{IT_STRING|IT_CVAR, NULL, "Static lighting",  &cv_grstaticlighting,  30},
};
#endif

static menuitem_t OGL_FogMenu[] =
{
	{IT_STRING|IT_CVAR,       NULL, "Fog",         &cv_grfog,         0},
	{IT_STRING|IT_KEYHANDLER, NULL, "Fog color",   M_HandleFogColor, 10},
	{IT_STRING|IT_CVAR,       NULL, "Fog density", &cv_grfogdensity, 20},
};

static menuitem_t OGL_ColorMenu[] =
{
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "red",   &cv_grgammared,   10},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "green", &cv_grgammagreen, 20},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "blue",  &cv_grgammablue,  30},
};

menu_t OpenGLOptionDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OpenGLOptionsMenu)/sizeof (menuitem_t),
	&VideoOptionsDef,
	OpenGLOptionsMenu,
	M_DrawOpenGLMenu,
	30, 40,
	0,
	NULL
};

#ifdef ALAM_LIGHTING
menu_t OGL_LightingDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OGL_LightingMenu)/sizeof (menuitem_t),
	&OpenGLOptionDef,
	OGL_LightingMenu,
	M_DrawGenericMenu,
	60, 40,
	0,
	NULL
};
#endif

menu_t OGL_FogDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OGL_FogMenu)/sizeof (menuitem_t),
	&OpenGLOptionDef,
	OGL_FogMenu,
	M_OGL_DrawFogMenu,
	60, 40,
	0,
	NULL
};

menu_t OGL_ColorDef =
{
	"M_OPTTTL",
	"OPTIONS",
	sizeof (OGL_ColorMenu)/sizeof (menuitem_t),
	&OpenGLOptionDef,
	OGL_ColorMenu,
	M_OGL_DrawColorMenu,
	60, 40,
	0,
	NULL
};
//======================================================================
// M_DrawOpenGLMenu()
//======================================================================
static void M_DrawOpenGLMenu(void)
{
	INT32 mx, my;

	mx = OpenGLOptionDef.x;
	my = OpenGLOptionDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
#if 0
	V_DrawString(BASEVIDWIDTH - mx - V_StringWidth(cv_scr_depth.string),
		my + currentMenu->menuitems[2].alphaKey, V_YELLOWMAP, cv_scr_depth.string);
#else
	(void)mx;
	(void)my;
#endif
}

#define FOG_COLOR_ITEM  1
//======================================================================
// M_OGL_DrawFogMenu()
//======================================================================
static void M_OGL_DrawFogMenu(void)
{
	INT32 mx, my;

	mx = OGL_FogDef.x;
	my = OGL_FogDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawString(BASEVIDWIDTH - mx - V_StringWidth(cv_grfogcolor.string),
		my + currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey, V_YELLOWMAP, cv_grfogcolor.string);
	// blink cursor on FOG_COLOR_ITEM if selected
	if (itemOn == FOG_COLOR_ITEM && skullAnimCounter < 4)
		V_DrawCharacter(BASEVIDWIDTH - mx,
			my + currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey, '_' | 0x80,false);
}

//======================================================================
// M_OGL_DrawColorMenu()
//======================================================================
static void M_OGL_DrawColorMenu(void)
{
	INT32 mx, my;

	mx = OGL_ColorDef.x;
	my = OGL_ColorDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawString(mx, my + currentMenu->menuitems[0].alphaKey - 10,
		V_YELLOWMAP, "Gamma correction");
}

//======================================================================
// M_OpenGLOption()
//======================================================================
#ifdef SHUFFLE
static void M_OpenGLOption(INT32 choice)
{
	(void)choice;
	if (rendermode != render_soft)
		M_SetupNextMenu(&OpenGLOptionDef);
	else
		M_StartMessage("You are in software mode\nYou can't change the options\n", NULL, MM_NOTHING);
}
#endif
//======================================================================
// M_HandleFogColor()
//======================================================================
static void M_HandleFogColor(INT32 choice)
{
	size_t i, l;
	char temp[8];
	boolean exitmenu = false; // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			itemOn++;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			itemOn--;
			break;

		case KEY_ESCAPE:
			S_StartSound(NULL, sfx_menu1);
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			S_StartSound(NULL, sfx_menu1);
			strcpy(temp, cv_grfogcolor.string);
			strcpy(cv_grfogcolor.zstring, "000000");
			l = strlen(temp)-1;
			for (i = 0; i < l; i++)
				cv_grfogcolor.zstring[i + 6 - l] = temp[i];
			break;

		default:
			if ((choice >= '0' && choice <= '9') || (choice >= 'a' && choice <= 'f')
				|| (choice >= 'A' && choice <= 'F'))
			{
				S_StartSound(NULL, sfx_menu1);
				strcpy(temp, cv_grfogcolor.string);
				strcpy(cv_grfogcolor.zstring, "000000");
				l = strlen(temp);
				for (i = 0; i < l; i++)
					cv_grfogcolor.zstring[5 - i] = temp[l - i];
					cv_grfogcolor.zstring[5] = (char)choice;
			}
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}
#endif

#ifndef NONET
static void M_NextServerPage(void)
{
	if ((serverlistpage + 1) * SERVERS_PER_PAGE < serverlistcount) serverlistpage++;
}

static void M_PrevServerPage(void)
{
	if (serverlistpage > 0) serverlistpage--;
}
#endif

// Descending order. The casts are safe as long as the caller doesn't
// do anything stupid.
#define SERVER_LIST_ENTRY_COMPARATOR(key) \
static int ServerListEntryComparator_##key(const void *entry1, const void *entry2) \
{ \
	return ((const serverelem_t*)entry2)->info.key - ((const serverelem_t*)entry1)->info.key; \
}

SERVER_LIST_ENTRY_COMPARATOR(time)
SERVER_LIST_ENTRY_COMPARATOR(numberofplayer)
SERVER_LIST_ENTRY_COMPARATOR(gametype)

static void M_SortServerList(void)
{
	switch(cv_serversort.value)
	{
	case 0:		// Ping.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_time);
		break;
	case 1:		// Players.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_numberofplayer);
		break;
	case 2:		// Gametype.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_gametype);
		break;
	}
}

// Message responder for turning on
// cheats through the menu system.
void M_CheatActivationResponder(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	CV_SetValue(&cv_cheats, 1);
}

