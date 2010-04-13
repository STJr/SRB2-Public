// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004-2005 by Sonic Team Junior.
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
//
// Actually, most of this file was written by Graue
// <graue@oceanbase.org>. You can use Graue's parts with no
// restriction, as they are copyright-free.
//-----------------------------------------------------------------------------
/// \file
/// \brief Intermission

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_net.h"
#include "i_video.h"
#include "p_tick.h"
#include "r_defs.h"
#include "r_things.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "y_inter.h"
#include "z_zone.h"
#include "m_menu.h"
#include "m_misc.h"
#include "i_system.h"

#include "r_local.h"
#include "p_local.h"

#if defined (SHUFFLE) && defined(HWRENDER)
#include "hardware/hw_main.h"
#endif

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

typedef union
{
	struct
	{
		UINT32 score; // fake score
		INT32 timebonus, ringbonus, perfbonus, total;
		INT32 min, sec, tics;
		boolean gotperfbonus; // true if we should show the perfect bonus line
		patch_t *ttlnum; // act number being displayed
		patch_t *ptimebonus; // TIME BONUS
		patch_t *pringbonus; // RING BONUS
		patch_t *pperfbonus; // PERFECT BONUS
		patch_t *ptotal; // TOTAL
		char passed1[13]; // KNUCKLES GOT
		char passed2[16]; // THROUGH THE ACT
		INT32 passedx1, passedx2;
		INT32 gotlife; // Player # that got an extra life
	} coop;

	struct
	{
		UINT32 score; // fake score
		INT32 ringbonus;
		INT32 headx;
		patch_t *cemerald; // CHAOS EMERALDS (or GOT THEM ALL!)
		patch_t *nowsuper; // SONIC CAN NOW BE SUPER SONIC
		patch_t *pringbonus; // RING BONUS
		patch_t *cscore; // SCORE
	} spec;

	struct
	{
		UINT32 scores[MAXPLAYERS]; // Winner's score
		INT32 *color[MAXPLAYERS]; // Winner's color #
		boolean spectator[MAXPLAYERS]; // Spectator list
		INT32 *character[MAXPLAYERS]; // Winner's character #
		INT32 num[MAXPLAYERS]; // Winner's player #
		char *name[MAXPLAYERS]; // Winner's name
		patch_t *result; // RESULT
		patch_t *blueflag;
		patch_t *redflag; // int_ctf uses this struct too.
		INT32 numplayers; // Number of players being displayed
		char levelstring[40]; // holds levelnames up to 32 characters
	} match;

	struct
	{
		UINT32 scores[4]; // player scores
		INT32 timemin[4]; // time (minutes)
		INT32 timesec[4]; // time (seconds)
		INT32 timetic[4]; // time (tics)
		INT32 rings[4]; // rings
		INT32 totalrings[4]; // total rings
		INT32 itemboxes[4]; // item boxes
		INT32 totalwins[4]; // how many wins each player has
		INT32 numplayersshown; // how many players are displayed (1-4)
		INT32 playersshown[4]; // the player numbers of these players
		const char *winnerstrings[5]; // string for winner in each category
		INT32 winner; // the overall winner's player number
		patch_t *result; // RESULT
		char levelstring[40]; // holds levelnames up to 32 characters
	} race;

} y_data;

static y_data data;

// graphics
static patch_t *bgpatch = NULL;     // INTERSCR
static patch_t *widebgpatch = NULL; // INTERSCW
static patch_t *bgtile = NULL;      // SPECTILE/SRB2BACK
static patch_t *interpic = NULL;    // custom picture defined in map header
static boolean usetile;
static boolean usebuffer;
static boolean useinterpic;
static INT32 timer;
static boolean gottimebonus;
static boolean gotemblem;

static INT32 intertic;
static INT32 endtic = -1;

static enum
{
	int_none,
	int_coop,     // Single Player/Cooperative
	int_match,    // Match
	int_teammatch,// Team Match
//	int_tag,      // Tag
	int_ctf,      // CTF
#ifdef CHAOSISNOTDEADYET
	int_chaos,    // Chaos
#endif
	int_spec,     // Special Stage
	int_race,     // Race
	int_classicrace, // Race (Time Only)
} inttype = int_none;

static void Y_AwardCoopBonuses(void);
static void Y_AwardSpecialStageBonus(void);
static void Y_CalculateRaceWinners(void);
static void Y_CalculateTimeRaceWinners(void);
static void Y_CalculateMatchWinners(void);
static void Y_DrawScaledNum(INT32 x, INT32 y, INT32 flags, INT32 num);
#define Y_DrawNum(x,y,n) Y_DrawScaledNum(x, y, 0, n)
static void Y_FollowIntermission(void);
static void Y_UnloadData(void);

//
// Y_IntermissionDrawer
//
// Called by D_Display. Nothing is modified here; all it does is draw.
// Neat concept, huh?
//
void Y_IntermissionDrawer(void)
{
	if (inttype == int_none || rendermode == render_none)
		return;

	if (!usebuffer)
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (useinterpic)
		V_DrawScaledPatch(0, 0, 0, interpic);
	else if (!usetile)
	{
		if (rendermode == render_soft && usebuffer)
			VID_BlitLinearScreen(screens[1], screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.rowbytes);
#if defined (SHUFFLE) && defined (HWRENDER)
		else if(rendermode != render_soft && usebuffer)
		{
			HWR_DrawIntermissionBG();
		}
#endif
		else
		{
			if (widebgpatch && rendermode == render_soft && vid.width / vid.dupx == 400)
				V_DrawScaledPatch(0, 0, V_SNAPTOLEFT, widebgpatch);
			else
				V_DrawScaledPatch(0, 0, 0, bgpatch);
		}
	}
	else
		V_DrawPatchFill(bgtile);

	if (inttype == int_coop)
	{
		// draw score
		V_DrawScaledPatch(hudinfo[HUD_SCORE].x, hudinfo[HUD_SCORE].y, V_SNAPTOLEFT, sboscore);
		Y_DrawScaledNum(hudinfo[HUD_SCORENUM].x, hudinfo[HUD_SCORENUM].y, V_SNAPTOLEFT, data.coop.score);

		// draw time
		V_DrawScaledPatch(hudinfo[HUD_TIME].x, hudinfo[HUD_TIME].y, V_SNAPTOLEFT, sbotime);
		if (cv_timetic.value == 1)
			Y_DrawScaledNum(hudinfo[HUD_SECONDS].x, hudinfo[HUD_SECONDS].y, V_SNAPTOLEFT, data.coop.tics);
		else
		{
			if (data.coop.sec < 10)
				Y_DrawScaledNum(hudinfo[HUD_LOWSECONDS].x, hudinfo[HUD_LOWSECONDS].y, V_SNAPTOLEFT, 0);
			Y_DrawScaledNum(hudinfo[HUD_SECONDS].x, hudinfo[HUD_SECONDS].y, V_SNAPTOLEFT, data.coop.sec);
			Y_DrawScaledNum(hudinfo[HUD_MINUTES].x, hudinfo[HUD_MINUTES].y, V_SNAPTOLEFT, data.coop.min);
			V_DrawScaledPatch(hudinfo[HUD_TIMECOLON].x, hudinfo[HUD_TIMECOLON].y, V_SNAPTOLEFT, sbocolon);
			// we should show centiseconds on the intermission screen too, if the conditions are right.
			if (timeattacking || cv_timetic.value == 2)
			{
				INT32 tics = G_TicsToCentiseconds(data.coop.tics);
				if (tics < 10)
					Y_DrawScaledNum(hudinfo[HUD_LOWTICS].x, hudinfo[HUD_LOWTICS].y, V_SNAPTOLEFT, 0);
				V_DrawScaledPatch(hudinfo[HUD_TIMETICCOLON].x, hudinfo[HUD_TIMETICCOLON].y, V_SNAPTOLEFT, sbocolon);
				Y_DrawScaledNum(hudinfo[HUD_TICS].x, hudinfo[HUD_TICS].y, V_SNAPTOLEFT, tics);
			}
		}

		// draw the "got through act" lines and act number
		V_DrawLevelTitle(data.coop.passedx1, 49, 0, data.coop.passed1);
		V_DrawLevelTitle(data.coop.passedx2, 49+V_LevelNameHeight(data.coop.passed2)+2, 0, data.coop.passed2);

		if (mapheaderinfo[gamemap-1].actnum)
			V_DrawScaledPatch(244, 57, 0, data.coop.ttlnum);

		V_DrawScaledPatch(68, 84 + 3*SHORT(tallnum[0]->height)/2, 0, data.coop.ptimebonus);
		Y_DrawNum(BASEVIDWIDTH - 68, 85 + 3*SHORT(tallnum[0]->height)/2, data.coop.timebonus);

		V_DrawScaledPatch(68, 84 + 3*SHORT(tallnum[0]->height), 0, data.coop.pringbonus);
		Y_DrawNum(BASEVIDWIDTH - 68, 85 + 3*SHORT(tallnum[0]->height), data.coop.ringbonus);

		//PERFECT BONUS
		if (data.coop.gotperfbonus)
		{
			V_DrawScaledPatch(56, 84 + ((9*SHORT(tallnum[0]->height))+1)/2, 0, data.coop.pperfbonus);
			Y_DrawNum(BASEVIDWIDTH - 68, 85 + ((9*SHORT(tallnum[0]->height))+1)/2, data.coop.perfbonus);
		}

		V_DrawScaledPatch(88, 84 + 6*SHORT(tallnum[0]->height), 0, data.coop.ptotal);
		Y_DrawNum(BASEVIDWIDTH - 68, 85 + 6*SHORT(tallnum[0]->height), data.coop.total);

		if (gottimebonus && endtic != -1)
			V_DrawCenteredString(BASEVIDWIDTH/2, 136, V_YELLOWMAP, "GOT TIME BONUS EMBLEM!");
		if (gotemblem && !gottimebonus && endtic != -1)
			V_DrawCenteredString(BASEVIDWIDTH/2, 172, V_YELLOWMAP, "GOT PERFECT BONUS EMBLEM!");
	}
	else if (inttype == int_spec)
	{
		// draw the header
		if (endtic != -1 && ALL7EMERALDS(emeralds) && data.spec.nowsuper != NULL)
			V_DrawScaledPatch(48, 32, 0, data.spec.nowsuper);
		else
			V_DrawScaledPatch(data.spec.headx, 26, 0, data.spec.cemerald);

		// draw the emeralds
		if (intertic & 1)
		{
			if (emeralds & EMERALD1)
				V_DrawScaledPatch(80, 92, 0, emeraldpics[0]);
			if (emeralds & EMERALD2)
				V_DrawScaledPatch(104, 92, 0, emeraldpics[1]);
			if (emeralds & EMERALD3)
				V_DrawScaledPatch(128, 92, 0, emeraldpics[2]);
			if (emeralds & EMERALD4)
				V_DrawScaledPatch(152, 92, 0, emeraldpics[3]);
			if (emeralds & EMERALD5)
				V_DrawScaledPatch(176, 92, 0, emeraldpics[4]);
			if (emeralds & EMERALD6)
				V_DrawScaledPatch(200, 92, 0, emeraldpics[5]);
			if (emeralds & EMERALD7)
				V_DrawScaledPatch(224, 92, 0, emeraldpics[6]);
		}

		V_DrawScaledPatch(80, 132, 0, data.spec.pringbonus);
		Y_DrawNum(232, 133, data.spec.ringbonus);
		V_DrawScaledPatch(80, 148, 0, data.spec.cscore);
		Y_DrawNum(232, 149, data.spec.score);
	}
	else if (inttype == int_match || inttype == int_race)
	{
		INT32 i = 0, j = 0;
		INT32 x = 4;
		INT32 y = 48;
		char name[MAXPLAYERNAME+1];

		// draw the header
		V_DrawScaledPatch(112, 2, 0, data.match.result);

		// draw the level name
		V_DrawCenteredString(BASEVIDWIDTH/2, 20, 0, data.match.levelstring);
		V_DrawFill(4, 42, 312, 1, 0);

		if (data.match.numplayers > 9)
		{
			V_DrawFill(160, 32, 1, 152, 0);

			if (inttype == int_race)
				V_DrawRightAlignedString(x+152, 32, V_YELLOWMAP, "TIME");
			else
				V_DrawRightAlignedString(x+152, 32, V_YELLOWMAP, "SCORE");

			V_DrawCenteredString(x+(BASEVIDWIDTH/2)+6, 32, V_YELLOWMAP, "#");
			V_DrawString(x+(BASEVIDWIDTH/2)+36, 32, V_YELLOWMAP, "NAME");
		}

		V_DrawCenteredString(x+6, 32, V_YELLOWMAP, "#");
		V_DrawString(x+36, 32, V_YELLOWMAP, "NAME");

		if (inttype == int_race)
			V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, V_YELLOWMAP, "TIME");
		else
			V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, V_YELLOWMAP, "SCORE");

		for (i = 0; i < data.match.numplayers; i++)
		{
			char strtime[10];

			if (data.match.spectator[i])
				continue; //Ignore spectators.

			V_DrawCenteredString(x+6, y, 0, va("%d", j+1));
			j++; //We skip spectators, but not their number.

			if (playeringame[data.match.num[i]])
			{
				if (data.match.color[i] == 0)
					V_DrawSmallScaledPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]]);
				else
				{
					UINT8 *colormap = (UINT8 *) translationtables[*data.match.character[i]] - 256 + (*data.match.color[i]<<8);
					V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]], colormap);
				}

				if (data.match.numplayers > 9)
				{
					if (inttype == int_race)
						strlcpy(name, data.match.name[i], 8);
					else
						strlcpy(name, data.match.name[i], 9);
				}
				else
					STRBUFCPY(name, data.match.name[i]);

				V_DrawString(x+36, y, V_ALLOWLOWERCASE, name);

				if (data.match.numplayers > 9)
				{
					if (inttype == int_match)
						V_DrawRightAlignedString(x+152, y, 0, va("%i", data.match.scores[i]));
					else if (inttype == int_race)
					{
						snprintf(strtime, sizeof strtime,
							"%i:%02i.%02i",
							G_TicsToMinutes(data.match.scores[i], true),
							G_TicsToSeconds(data.match.scores[i]), G_TicsToCentiseconds(data.match.scores[i]));
						strtime[sizeof strtime - 1] = '\0';
						V_DrawRightAlignedString(x+152, y, 0, strtime);
					}
				}
				else
				{
					if (inttype == int_match)
						V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, va("%u", data.match.scores[i]));
					else if (inttype == int_race)
					{
						if (players[data.match.num[i]].pflags & PF_TIMEOVER)
						{
							if (data.match.numplayers > 9)
								snprintf(strtime, sizeof strtime, "TIME O.");
							else
								snprintf(strtime, sizeof strtime, "TIME OVER");
						}
						else if (players[data.match.num[i]].lives <= 0)
						{
							if (data.match.numplayers > 9)
								snprintf(strtime, sizeof strtime, "GAME O.");
							else
								snprintf(strtime, sizeof strtime, "GAME OVER");
						}
						else
							snprintf(strtime, sizeof strtime, "%i:%02i.%02i", G_TicsToMinutes(data.match.scores[i], true),
									G_TicsToSeconds(data.match.scores[i]), G_TicsToCentiseconds(data.match.scores[i]));

						strtime[sizeof strtime - 1] = '\0';

						V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, strtime);
					}
				}
			}

			y += 16;

			if (y > 176)
			{
				y = 48;
				x += BASEVIDWIDTH/2;
			}
		}
	}
	else if (inttype == int_ctf || inttype == int_teammatch)
	{
		INT32 i, x = 4, y = 0;
		INT32 redplayers = 0, blueplayers = 0;
		char name[MAXPLAYERNAME+1];

		// Show the team flags and the team score at the top instead of "RESULTS"
		V_DrawSmallScaledPatch(128 - SHORT(data.match.blueflag->width)/4, 2, 0, data.match.blueflag);
		V_DrawCenteredString(128, 16, 0, va("%u", bluescore));

		V_DrawSmallScaledPatch(192 - SHORT(data.match.redflag->width)/4, 2, 0, data.match.redflag);
		V_DrawCenteredString(192, 16, 0, va("%u", redscore));

		// draw the level name
		V_DrawCenteredString(BASEVIDWIDTH/2, 24, 0, data.match.levelstring);
		V_DrawFill(4, 42, 312, 1, 0);

		//vert. line
		V_DrawFill(160, 32, 1, 152, 0);

		//strings at the top of the list
		V_DrawCenteredString(x+6, 32, V_YELLOWMAP, "#");
		V_DrawCenteredString(x+(BASEVIDWIDTH/2)+6, 32, V_YELLOWMAP, "#");

		V_DrawString(x+36, 32, V_YELLOWMAP, "NAME");
		V_DrawString(x+(BASEVIDWIDTH/2)+36, 32, V_YELLOWMAP, "NAME");

		V_DrawRightAlignedString(x+152, 32, V_YELLOWMAP, "SCORE");
		V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, V_YELLOWMAP, "SCORE");

		for (i = 0; i < data.match.numplayers; i++)
		{
			if (playeringame[data.match.num[i]] && !(data.match.spectator[i]))
			{
				UINT8 *colormap = (UINT8 *) translationtables[*data.match.character[i]] - 256 + (*data.match.color[i]<<8);

				if (*data.match.color[i] == 6) //red
				{
					if (redplayers++ > 9)
						continue;
					x = 4 + (BASEVIDWIDTH/2);
					y = (redplayers * 16) + 32;
					V_DrawCenteredString(x+6, y, 0, va("%d", redplayers));
				}
				else if (*data.match.color[i] == 7) //blue
				{
					if (blueplayers++ > 9)
						continue;
					x = 4;
					y = (blueplayers * 16) + 32;
					V_DrawCenteredString(x+6, y, 0, va("%d", blueplayers));
				}
				else
					continue;

				//color is ALWAYS going to be 6/7 here, no need to check if it's nonzero.
				V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]], colormap);

				strlcpy(name, data.match.name[i], 9);

				V_DrawString(x+36, y, V_ALLOWLOWERCASE, name);

				V_DrawRightAlignedString(x+152, y, 0, va("%u", data.match.scores[i]));
			}
		}
	}
	else if (inttype == int_classicrace)
	{
		char name[9] = "xxxxxxxx";
		INT32 i;

		// draw the header
		V_DrawScaledPatch(112, 8, 0, data.race.result);

		// draw the level name
		V_DrawCenteredString(BASEVIDWIDTH/2, 28, 0, data.race.levelstring);

		// draw the category names
		V_DrawString(8, 66, 0, "SCORE");
		V_DrawString(8, 78, 0, "TIME");
		V_DrawString(8, 90, 0, "RING");
		V_DrawString(8, 102, 0, "TOT. RING");
		V_DrawString(8, 114, 0, "ITEM BOX");
		V_DrawString(0, 138, 0, "* TOTAL *");

		// draw the W
		V_DrawCharacter(304, 50, 'W', false);

		// draw the winner in each category
		V_DrawCenteredString(308, 66, V_YELLOWMAP, data.race.winnerstrings[0]);
		V_DrawCenteredString(308, 78, V_YELLOWMAP, data.race.winnerstrings[1]);
		V_DrawCenteredString(308, 90, V_YELLOWMAP, data.race.winnerstrings[2]);
		V_DrawCenteredString(308, 102, V_YELLOWMAP, data.race.winnerstrings[3]);
		V_DrawCenteredString(308, 114, V_YELLOWMAP, data.race.winnerstrings[4]);

		// draw the overall winner
		if (data.race.winner == -1)
			V_DrawCenteredString(BASEVIDWIDTH/2, 163, V_YELLOWMAP, "TIED");
		else
		{
			V_DrawCenteredString(BASEVIDWIDTH/2, 159, V_YELLOWMAP|V_ALLOWLOWERCASE,
				player_names[data.race.winner]);
			V_DrawCenteredString(BASEVIDWIDTH/2, 167, V_YELLOWMAP, "WINS");
		}

		for (i = 0; i < data.race.numplayersshown; i++)
		{
			// draw the player's name (max 8 chars)
			strlcpy(name, player_names[data.race.playersshown[i]], 9);
			V_DrawRightAlignedString(104 + 64*i, 46, V_ALLOWLOWERCASE, name);

			// draw 1P/2P/3P/4P
			name[2] = '\0';
			name[1] = 'P';
			name[0] = (char)('1' + (char)i);
			V_DrawRightAlignedString(104 + 64*i, 54, 0, name);

			name[sizeof name - 1] = '\0';

			// draw score
			snprintf(name, sizeof name - 1, "%d", data.race.scores[i]);
			V_DrawRightAlignedString(104 + 64*i, 66, 0, name);

			// draw time
			snprintf(name, sizeof name - 1, "%d:%02d.%02d",
				data.race.timemin[i], data.race.timesec[i],
				data.race.timetic[i]);
			V_DrawRightAlignedString(104 + 64*i, 78, 0, name);

			// draw ring count
			snprintf(name, sizeof name - 1, "%d", data.race.rings[i]);
			V_DrawRightAlignedString(104 + 64*i, 90, 0, name);

			// draw total ring count
			snprintf(name, sizeof name - 1, "%d",
				data.race.totalrings[i]);
			V_DrawRightAlignedString(104 + 64*i, 102, 0, name);

			// draw item box count
			snprintf(name, sizeof name - 1, "%d",
				data.race.itemboxes[i]);
			V_DrawRightAlignedString(104 + 64*i, 114, 0, name);

			// draw total number of wins
			snprintf(name, sizeof name - 1, "%d",
				data.race.totalwins[i]);
			V_DrawRightAlignedString(104 + 64*i, 138, 0, name);
		}
	}

	if (timer)
		V_DrawCenteredString(BASEVIDWIDTH/2, 188, V_YELLOWMAP,
			va("start in %d seconds", timer/TICRATE));

	// Make it obvious that scrambling is happening next round.
	if (cv_scrambleonchange.value && cv_teamscramble.value && (intertic/TICRATE % 2 == 0))
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, V_YELLOWMAP, va("Teams will be scrambled next round!"));
}

//
// Y_Ticker
//
// Manages fake score tally for single player end of act, and decides when intermission is over.
//
void Y_Ticker(void)
{
	if (inttype == int_none)
		return;

	// Check for pause or menu up in single player
	if (paused || (!netgame && menuactive && !demoplayback))
		return;

	intertic++;

	// Team scramble code for team match and CTF.
	// Don't do this if we're going to automatically scramble teams next round.
	if ((gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	    && cv_teamscramble.value && !cv_scrambleonchange.value && server)
	{
		// If we run out of time in intermission, the beauty is that
		// the P_Ticker() team scramble code will pick it up.
		if ((intertic % (TICRATE/7)) == 0)
			P_DoTeamscrambling();
	}

	// multiplayer uses timer (based on cv_inttime)
	if (timer)
	{
		if (!--timer)
		{
			Y_EndIntermission();
			Y_FollowIntermission();
			return;
		}
	}
	// single player is hardcoded to go away after awhile
	else if (intertic == endtic)
	{
		Y_EndIntermission();
		Y_FollowIntermission();
		return;
	}

	if (endtic != -1)
		return; // tally is done

	if (inttype == int_coop) // coop or single player, normal level
	{
		if (!intertic) // first time only
			S_ChangeMusic(mus_lclear, false); // don't loop it

		if (intertic < TICRATE) // one second pause before tally begins
			return;

		if (data.coop.ringbonus || data.coop.timebonus || data.coop.perfbonus)
		{
			INT32 i;
			boolean skip = false;

			if (!(intertic & 1))
				S_StartSound(NULL, sfx_menu1); // tally sound effect

			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (players[i].cmd.buttons & BT_USE)
				{
					skip = true;
					break;
				}
			}

			// ring and time bonuses count down by 222 each tic
			if (data.coop.ringbonus)
			{
				data.coop.ringbonus -= 222;
				data.coop.total += 222;
				data.coop.score += 222;
				if (data.coop.ringbonus < 0 || skip == true) // went too far
				{
					data.coop.score += data.coop.ringbonus;
					data.coop.total += data.coop.ringbonus;
					data.coop.ringbonus = 0;

					if (skip == true && (data.coop.gotlife == consoleplayer || data.coop.gotlife == secondarydisplayplayer))
					{
						// lives are already added since tally is fake, but play the music
						if (mariomode)
							S_StartSound(NULL, sfx_marioa);
						else
						{
							S_StopMusic(); // otherwise it won't restart if this is done twice in a row
							S_ChangeMusic(mus_xtlife, false);
						}
					}
				}
			}
			if (data.coop.timebonus)
			{
				data.coop.timebonus -= 222;
				data.coop.total += 222;
				data.coop.score += 222;
				if (data.coop.timebonus < 0 || skip == true)
				{
					data.coop.score += data.coop.timebonus;
					data.coop.total += data.coop.timebonus;
					data.coop.timebonus = 0;
				}
			}
			if (data.coop.perfbonus)
			{
				data.coop.perfbonus -= 222;
				data.coop.total += 222;
				data.coop.score += 222;
				if (data.coop.perfbonus < 0 || skip == true)
				{
					data.coop.score += data.coop.perfbonus;
					data.coop.total += data.coop.perfbonus;
					data.coop.perfbonus = 0;
				}
			}
			if (!data.coop.timebonus && !data.coop.ringbonus && !data.coop.perfbonus)
			{
				endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
				S_StartSound(NULL, sfx_chchng); // cha-ching!
			}

			if (data.coop.score % 50000 < 222) // just passed a 50000 point mark
			{
				// lives are already added since tally is fake, but play the music
				if (mariomode)
					S_StartSound(NULL, sfx_marioa);
				else
				{
					S_StopMusic(); // otherwise it won't restart if this is done twice in a row
					S_ChangeMusic(mus_xtlife, false);
				}
			}
		}
		else
		{
			endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
			S_StartSound(NULL, sfx_chchng); // cha-ching!
		}
	}
	else if (inttype == int_spec) // coop or single player, special stage
	{
		if (!intertic) // first time only
			S_ChangeMusic(mus_lclear, false); // don't loop it

		if (intertic < TICRATE) // one second pause before tally begins
			return;

		if (data.spec.ringbonus)
		{
			INT32 i;
			boolean skip = false;

			if (!(intertic & 1))
				S_StartSound(NULL, sfx_menu1); // tally sound effect

			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (players[i].cmd.buttons & BT_USE)
				{
					skip = true;
					break;
				}
			}

			// ring bonus counts down by 222 each tic
			data.spec.ringbonus -= 222;
			data.spec.score += 222;
			if (data.spec.ringbonus < 0 || skip == true) // went too far
			{
				data.spec.score += data.spec.ringbonus;
				data.spec.ringbonus = 0;

				if (skip == true && (data.coop.gotlife == consoleplayer || data.coop.gotlife == secondarydisplayplayer))
				{
					// lives are already added since tally is fake, but play the music
					if (mariomode)
						S_StartSound(NULL, sfx_marioa);
					else
					{
						S_StopMusic(); // otherwise it won't restart if this is done twice in a row
						S_ChangeMusic(mus_xtlife, false);
					}
				}
			}

			if (!data.spec.ringbonus)
			{
				endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
				S_StartSound(NULL, sfx_chchng); // cha-ching!
			}

			if (data.spec.score % 50000 < 222 && skip == false) // just passed a 50000 point mark
			{
				// lives are already added since tally is fake, but play the music
				if (mariomode)
					S_StartSound(NULL, sfx_marioa);
				else
				{
					S_StopMusic(); // otherwise it won't restart if this is done twice in a row
					S_ChangeMusic(mus_xtlife, false);
				}
			}
		}
		else
		{
			endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
			S_StartSound(NULL, sfx_chchng); // cha-ching!
		}
	}
	else if (inttype == int_match || inttype == int_ctf || inttype == int_teammatch) // match
	{
		if (!intertic) // first time only
			S_ChangeMusic(mus_racent, true); // loop it

		// If a player has left or joined, recalculate scores.
		if (data.match.numplayers != D_NumPlayers())
			Y_CalculateMatchWinners();
	}
	else if (inttype == int_race || inttype == int_classicrace) // race
	{
		if (!intertic) // first time only
			S_ChangeMusic(mus_racent, true); // loop it

		// Don't bother recalcing for race. It doesn't make as much sense.
	}
}

//
// Y_StartIntermission
//
// Called by G_DoCompleted. Sets up data for intermission drawer/ticker.
//
void Y_StartIntermission(void)
{
	intertic = -1;

#ifdef PARANOIA
	if (endtic != -1)
		I_Error("endtic is dirty");
#endif

	if (!multiplayer)
	{
		timer = 0;

		if (G_IsSpecialStage(gamemap))
			inttype = int_spec;
		else
			inttype = int_coop;
	}
	else
	{
		if (cv_inttime.value == 0 && gametype == GT_COOP)
			timer = 0;
		else
		{
			timer = cv_inttime.value*TICRATE;

			if (!timer)
				timer = 1;
		}

		if (gametype == GT_COOP)
		{
			if (G_IsSpecialStage(gamemap))
				inttype = int_spec;
			else
				inttype = int_coop;
		}
		else if (gametype == GT_MATCH
#ifdef CHAOSISNOTDEADYET
			|| gametype == GT_CHAOS
#endif
			|| gametype == GT_TAG)
		{
			if (gametype != GT_TAG && cv_matchtype.value) // Team Match
				inttype = int_teammatch;
			else
				inttype = int_match;
		}
		else if (gametype == GT_RACE)
		{
			if (cv_racetype.value) // Classic (Full Race in 1.09.4)
				inttype = int_classicrace;
			else // Normal (Time-Only in 1.09.4)
				inttype = int_race;
		}
		else if (gametype == GT_CTF)
		{
			inttype = int_ctf;
		}
	}

	// We couldn't display the intermission even if we wanted to.
	if (dedicated) return;

	switch (inttype)
	{
		case int_coop: // coop or single player, normal level
		{
			gottimebonus = false;
			gotemblem = false;

			// award time and ring bonuses
			Y_AwardCoopBonuses();

			// setup time data
			data.coop.tics = players[consoleplayer].realtime; // used if cv_timetic is on
			data.coop.sec = players[consoleplayer].realtime / TICRATE;
			data.coop.min = data.coop.sec / 60;
			data.coop.sec %= 60;

			if ((!modifiedgame || savemoddata) && !multiplayer && !demoplayback)
			{
				if(timeattacking)
				{
					if ((players[consoleplayer].realtime < timedata[gamemap-1].time) ||
						(timedata[gamemap-1].time == 0))
						timedata[gamemap-1].time = players[consoleplayer].realtime;

					if (!savemoddata && !(grade & 512))
					{
						INT32 emblemcount = 0;
						INT32 i;

						if (M_GotLowEnoughTime(23*60))
						{
							gottimebonus = true;
							emblemlocations[MAXEMBLEMS-3].collected = true;
							gotemblem = true;
							grade |= 512;
						}

						for (i = 0; i < MAXEMBLEMS; i++)
						{
							if (emblemlocations[i].collected)
								emblemcount++;
						}

						if (emblemcount >= numemblems/2 && !(grade & 4)) // Got half of emblems
							grade |= 4;

						if (emblemcount >= numemblems/4 && !(grade & 16)) // NiGHTS
							grade |= 16;

						if (emblemcount == numemblems && !(grade & 8)) // Got ALL emblems!
							grade |= 8;
					}
					G_SaveGameData();
				}
				else if (gamemap == 40) // Cleared NAGZ
					grade |= 2048;
			}

			// get act number
			if (mapheaderinfo[prevmap].actnum)
				data.coop.ttlnum = W_CachePatchName(va("TTL%.2d", mapheaderinfo[prevmap].actnum),
					PU_STATIC);
			else
				data.coop.ttlnum = W_CachePatchName("TTL01", PU_STATIC);

			// get background patches
			widebgpatch = W_CachePatchName("INTERSCW", PU_STATIC);
			bgpatch = W_CachePatchName("INTERSCR", PU_STATIC);

			// grab an interscreen if appropriate
			if (mapheaderinfo[gamemap-1].interscreen[0] != '#')
			{
				interpic = W_CachePatchName(mapheaderinfo[gamemap-1].interscreen, PU_STATIC);
				useinterpic = true;
				usebuffer = false;
			}
			else
			{
				useinterpic = false;
				usebuffer = true;
			}
			usetile = false;

			// get single player specific patches
			data.coop.ptotal = W_CachePatchName("YTOTAL", PU_STATIC);
			data.coop.ptimebonus = W_CachePatchName("YTMBONUS", PU_STATIC);
			data.coop.pringbonus = W_CachePatchName("YRINGBNS", PU_STATIC);
			data.coop.pperfbonus = W_CachePatchName("YPFBONUS", PU_STATIC);

			// set up the "got through act" message according to skin name
			if (strlen(skins[players[consoleplayer].skin].name) <= 8)
			{
				snprintf(data.coop.passed1,
					sizeof data.coop.passed1, "%s GOT",
					skins[players[consoleplayer].skin].name);
				data.coop.passed1[sizeof data.coop.passed1 - 1] = '\0';
				if (mapheaderinfo[gamemap-1].actnum)
				{
					strcpy(data.coop.passed2, "THROUGH ACT");
					data.coop.passedx1 = 62 + (176 - V_LevelNameWidth(data.coop.passed1))/2;
					data.coop.passedx2 = 62 + (176 - V_LevelNameWidth(data.coop.passed2))/2;
				}
				else
				{
					strcpy(data.coop.passed2, "THROUGH THE ACT");
					data.coop.passedx1 = (BASEVIDWIDTH - V_LevelNameWidth(data.coop.passed1))/2;
					data.coop.passedx2 = (BASEVIDWIDTH - V_LevelNameWidth(data.coop.passed2))/2;
				}
				// The above value is not precalculated because it needs only be computed once
				// at the start of intermission, and precalculating it would preclude mods
				// changing the font to one of a slightly different width.
			}
			else
			{
				strcpy(data.coop.passed1, skins[players[consoleplayer].skin].name);
				data.coop.passedx1 = 62 + (176 - V_LevelNameWidth(data.coop.passed1))/2;
				if (mapheaderinfo[gamemap-1].actnum)
				{
					strcpy(data.coop.passed2, "PASSED ACT");
					data.coop.passedx2 = 62 + (176 - V_LevelNameWidth(data.coop.passed2))/2;
				}
				else
				{
					strcpy(data.coop.passed2, "PASSED THE ACT");
					data.coop.passedx2 = 62 + (240 - V_LevelNameWidth(data.coop.passed2))/2;
				}
			}
			break;
		}

		case int_match:
		{
			// Calculate who won
			Y_CalculateMatchWinners();

			// set up the levelstring
			if (mapheaderinfo[prevmap].actnum)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"%.32s * %d *",
					mapheaderinfo[prevmap].lvlttl, mapheaderinfo[prevmap].actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap].lvlttl);

			data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

			// get RESULT header
			data.match.result =
				W_CachePatchName("RESULT", PU_STATIC);

			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;
			break;
		}

		case int_race: // (time-only race)
		{
			// Calculate who won
			Y_CalculateTimeRaceWinners();

			// set up the levelstring
			if (mapheaderinfo[prevmap].actnum)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"%.32s * %d *",
					mapheaderinfo[prevmap].lvlttl, mapheaderinfo[prevmap].actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap].lvlttl);

			data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

			// get RESULT header
			data.match.result = W_CachePatchName("RESULT", PU_STATIC);

			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;
			break;
		}

		case int_teammatch:
		case int_ctf:
		{
			// Calculate who won
			Y_CalculateMatchWinners();

			// set up the levelstring
			if (mapheaderinfo[prevmap].actnum)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"%.32s * %d *",
					mapheaderinfo[prevmap].lvlttl, mapheaderinfo[prevmap].actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap].lvlttl);

			data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

			if (inttype == int_ctf)
			{
				data.match.redflag = rflagico;
				data.match.blueflag = bflagico;
			}
			else // team match
			{
				data.match.redflag = rmatcico;
				data.match.blueflag = bmatcico;
			}

			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;
			break;
		}

		case int_spec: // coop or single player, special stage
		{
			// give out ring bonuses
			Y_AwardSpecialStageBonus();

			// get background tile
			bgtile = W_CachePatchName("SPECTILE", PU_STATIC);

			// grab an interscreen if appropriate
			if (mapheaderinfo[gamemap-1].interscreen[0] != '#')
			{
				interpic = W_CachePatchName(mapheaderinfo[gamemap-1].interscreen, PU_STATIC);
				useinterpic = true;
			}
			else
				useinterpic = false;

			// tile if using the default background
			usetile = !useinterpic;

			// get special stage specific patches
			if (ALL7EMERALDS(emeralds))
			{
				data.spec.cemerald = W_CachePatchName("GOTEMALL", PU_STATIC);
				data.spec.headx = 70;
				data.spec.nowsuper = players[consoleplayer].skin
					? NULL : W_CachePatchName("NOWSUPER", PU_STATIC);
			}
			else
			{
				data.spec.cemerald = W_CachePatchName("CEMERALD", PU_STATIC);
				data.spec.headx = 48;
				data.spec.nowsuper = NULL;
			}
			data.spec.pringbonus = W_CachePatchName("YRINGBNS", PU_STATIC);
			data.spec.cscore = W_CachePatchName("CSCORE", PU_STATIC);
			break;
		}
		case int_classicrace: // classic (full race)
		{
			// find out who won
			Y_CalculateRaceWinners();

			// set up the levelstring
			if (mapheaderinfo[prevmap].actnum)
				snprintf(data.race.levelstring,
					sizeof data.race.levelstring,
					"%.32s * %d *",
					mapheaderinfo[prevmap].lvlttl, mapheaderinfo[prevmap].actnum);
			else
				snprintf(data.race.levelstring,
					sizeof data.race.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap].lvlttl);

			data.race.levelstring[sizeof data.race.levelstring - 1] = '\0';

			// get background tile
			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;

			// get RESULT header
			data.race.result = W_CachePatchName("RESULT", PU_STATIC);
			break;
		}

		case int_none:
		default:
			break;
	}
}

//
// Y_AwardCoopBonuses
//
// Awards the time and ring bonuses.
//
static void Y_AwardCoopBonuses(void)
{
	INT32 i;
	INT32 sharedringtotal = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		//for the sake of my sanity, let's get this out of the way first
		if (!playeringame[i])
			continue;

		sharedringtotal += players[i].health - 1;
	}

	// with that out of the way, go back to calculating bonuses like usual
	for (i = 0; i < MAXPLAYERS; i++)
	{
		INT32 secs, bonus, oldscore;

		if (!playeringame[i])
			continue;

		// calculate time bonus
		secs = players[i].realtime / TICRATE;
		if (secs < 30)
			bonus = 50000;
		else if (secs < 45)
			bonus = 10000;
		else if (secs < 60)
			bonus = 5000;
		else if (secs < 90)
			bonus = 4000;
		else if (secs < 120)
			bonus = 3000;
		else if (secs < 180)
			bonus = 2000;
		else if (secs < 240)
			bonus = 1000;
		else if (secs < 300)
			bonus = 500;
		else
			bonus = 0;

		if (i == consoleplayer)
		{
			data.coop.timebonus = bonus;
			if (players[i].health)
				data.coop.ringbonus = (players[i].health-1) * 100;
			else
				data.coop.ringbonus = 0;
			data.coop.total = 0;
			data.coop.score = players[i].score;

			if (sharedringtotal && sharedringtotal >= nummaprings && (!mapheaderinfo[gamemap-1].noperfectbns)) //perfectionist!
			{
				data.coop.perfbonus = 50000;
				data.coop.gotperfbonus = true;
			}
			else
			{
				data.coop.perfbonus = 0;
				data.coop.gotperfbonus = false;
			}
		}

		oldscore = players[i].score;

		players[i].score += bonus;
		if (players[i].health)
			players[i].score += (players[i].health-1) * 100; // ring bonus

		//todo: more conditions where we shouldn't award a perfect bonus?
		if (sharedringtotal && sharedringtotal >= nummaprings && (!mapheaderinfo[gamemap-1].noperfectbns))
		{
			players[i].score += 50000; //perfect bonus

			if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && !demoplayback)
			{
				if (!emblemlocations[MAXEMBLEMS-4].collected)
				{
					INT32 j;
					INT32 emblemcount = 0;
					emblemlocations[MAXEMBLEMS-4].collected = true;
					gotemblem = true;

					for (j = 0; j < MAXEMBLEMS; j++)
					{
						if (emblemlocations[j].collected)
							emblemcount++;
					}

					if (emblemcount >= numemblems/2 && !(grade & 4)) // Got half of emblems
						grade |= 4;

					if (emblemcount >= numemblems/4 && !(grade & 16)) // NiGHTS
						grade |= 16;

					if (emblemcount == numemblems && !(grade & 8)) // Got ALL emblems!
						grade |= 8;

					G_SaveGameData();
				}
			}
		}

		// grant extra lives right away since tally is faked
		P_GivePlayerLives(&players[i], (players[i].score/50000) - (oldscore/50000));

		if ((players[i].score/50000) - (oldscore/50000) > 0)
			data.coop.gotlife = i;
		else
			data.coop.gotlife = -1;
	}
}

//
// Y_AwardSpecialStageBonus
//
// Gives a ring bonus only.
static void Y_AwardSpecialStageBonus(void)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		INT32 oldscore;

		if (!playeringame[i])
			continue;

		if (i == consoleplayer)
		{
			if (players[i].health)
				data.spec.ringbonus = (players[i].health-1) * 100;
			else
				data.spec.ringbonus = 0;
			data.spec.score = players[i].score;
		}

		oldscore = players[i].score;

		if (players[i].health)
			players[i].score += (players[i].health-1) * 100; // ring bonus

		// grant extra lives right away since tally is faked
		P_GivePlayerLives(&players[i], (players[i].score/50000) - (oldscore/50000));

		if ((players[i].score/50000) - (oldscore/50000) > 0)
			data.coop.gotlife = i;
		else
			data.coop.gotlife = -1;
	}
}

//
// Y_CalculateMatchWinners
//
static void Y_CalculateMatchWinners(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];

	// Initialize variables
	memset(data.match.scores, 0, sizeof (data.match.scores));
	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(data.match.spectator, 0, sizeof (data.match.spectator));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;
	i = j = 0;

	for (j = 0; j < MAXPLAYERS; j++)
	{
		if (!playeringame[j])
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (gametype == GT_TAG)
			{
				if (players[i].score >= data.match.scores[data.match.numplayers] && completed[i] == false)
				{
					data.match.scores[data.match.numplayers] = players[i].score;
					data.match.color[data.match.numplayers] = &players[i].skincolor;
					data.match.character[data.match.numplayers] = &players[i].skin;
					data.match.name[data.match.numplayers] = player_names[i];
					data.match.spectator[data.match.numplayers] = players[i].spectator;
					data.match.num[data.match.numplayers] = i;
				}
			}
			else if (players[i].score >= data.match.scores[data.match.numplayers] && completed[i] == false)
			{
				data.match.scores[data.match.numplayers] = players[i].score;
				data.match.color[data.match.numplayers] = &players[i].skincolor;
				data.match.character[data.match.numplayers] = &players[i].skin;
				data.match.name[data.match.numplayers] = player_names[i];
				data.match.spectator[data.match.numplayers] = players[i].spectator;
				data.match.num[data.match.numplayers] = i;
			}
		}
		completed[data.match.num[data.match.numplayers]] = true;
		data.match.numplayers++;
	}
}

//
// Y_CalculateTimeRaceWinners
//
static void Y_CalculateTimeRaceWinners(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];

	// Initialize variables

	for (i = 0; i < MAXPLAYERS; i++)
		data.match.scores[i] = INT32_MAX;

	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(data.match.spectator, 0, sizeof (data.match.spectator));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;
	i = j = 0;

	for (j = 0; j < MAXPLAYERS; j++)
	{
		if (!playeringame[j])
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (players[i].realtime <= data.match.scores[data.match.numplayers] && completed[i] == false)
			{
				data.match.scores[data.match.numplayers] = players[i].realtime;
				data.match.color[data.match.numplayers] = &players[i].skincolor;
				data.match.character[data.match.numplayers] = &players[i].skin;
				data.match.name[data.match.numplayers] = player_names[i];
				data.match.num[data.match.numplayers] = i;
			}
		}
		completed[data.match.num[data.match.numplayers]] = true;
		data.match.numplayers++;
	}
}

//
// Y_CalculateRaceWinners
//
static void Y_CalculateRaceWinners(void)
{
	INT32 winners[5], numwins[MAXPLAYERS];
	INT32 i = 0, n = 0, ring, totalring, itembox, wins;
	INT32 numplayersingame;
	UINT32 score = 0, racetime;

	// Everyone has zero wins.
	memset(numwins, 0, sizeof (INT32)*MAXPLAYERS);

	// No one has won anything.
	winners[0] = winners[1] = winners[2] = winners[3] = winners[4] = -1;

	// Find the highest score.
	for (; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].score < score)
			continue;

		if (players[i].score == score)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this score
		score = players[i].score; // best score so far
		winners[0] = i; // winner so far
	}

	if (n == 1)
		numwins[winners[0]]++;
	else
		winners[0] = -1; // tie

	// Find the lowest time.
	for (i = 0, n = 0, racetime = leveltime; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].realtime > racetime)
			continue;

		if (players[i].realtime == racetime)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this time
		racetime = players[i].realtime; // best time so far
		winners[1] = i; // winner so far
	}

	if (n == 1)
		numwins[winners[1]]++;
	else
		winners[1] = -1; // tie

	// Find the highest ring count.
	for (i = 0, n = 0, ring = -1; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || (players[i].health ? players[i].health-1 : 0) < ring)
			continue;

		if ((players[i].health ? players[i].health-1 : 0) == ring)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this many rings
		ring = (players[i].health ? players[i].health-1 : 0); // best ring count so far
		winners[2] = i; // winner so far
	}

	if (n == 1)
		numwins[winners[2]]++;
	else
		winners[2] = -1; // tie

	// Find the highest total ring count.
	for (i = 0, n = 0, totalring = -1; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].totalring < totalring)
			continue;

		if (players[i].totalring == totalring)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this many total rings
		totalring = players[i].totalring; // best total ring count so far
		winners[3] = i; // winner so far
	}

	if (n == 1)
		numwins[winners[3]]++;
	else
		winners[3] = -1; // tie

	// Find the highest item box count.
	for (i = 0, n = 0, itembox = -1; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].numboxes < itembox)
			continue;

		if (players[i].numboxes == itembox)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this many item boxes
		itembox = players[i].numboxes; // best item box count so far
		winners[4] = i; // winner so far
	}

	if (n == 1)
		numwins[winners[4]]++;
	else
		winners[4] = -1; // tie

	numplayersingame = 0;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
			numplayersingame++;
	}

	// Decide which players to display in the list.
	if (numplayersingame <= 4) // This is easy!
	{
		data.race.numplayersshown = numplayersingame;
		for (i = 0, n = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				data.race.playersshown[n++] = i;

		for (i = data.race.numplayersshown; i < 4; i++)
			data.race.playersshown[i] = -1; // no player here
	}
	else // This is hard!
	{
		INT32 j, k;

		data.race.numplayersshown = 4;
		for (i = 0, n = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				if (n < 4)
				{
					data.race.playersshown[n++] = i;
					continue;
				}

				// n == 4, meaning all four slots are full.

				if (!numwins[i])
					continue;

				// But this player won at least one category, so maybe he can
				// replace one who didn't.

				for (j = 3; j >= 0; j--)
					if (!numwins[j])
						break;

				if (j < 0)
					break; // Five winners, four slots. Sorry, you get stiffed.

				// j (0 <= j <= 3) is a slot whose player did not win any
				// categories. Player i can go here instead.

				// If j < 3, first we need to move the other displayed players
				// back one.

				for (k = j; k < 3; k++)
					data.race.playersshown[k] = data.race.playersshown[k+1];

				// Now player i gets his rightful position.

				data.race.playersshown[3] = i;
			}
	}

	// Set up the winner string for each category.
	//  "1P", "2P", "3P", "4P", "5P" or "T"
	for (i = 0; i < 5; i++)
	{
		if (winners[i] == -1)
			data.race.winnerstrings[i] = "T";
		else if (winners[i] == data.race.playersshown[0])
			data.race.winnerstrings[i] = "1P";
		else if (winners[i] == data.race.playersshown[1])
			data.race.winnerstrings[i] = "2P";
		else if (winners[i] == data.race.playersshown[2])
			data.race.winnerstrings[i] = "3P";
		else if (winners[i] == data.race.playersshown[3])
			data.race.winnerstrings[i] = "4P";
		else
			data.race.winnerstrings[i] = "5P";
	}

	// Set up the display slot data.
	for (i = 0; i < data.race.numplayersshown; i++)
	{
		data.race.scores[i] = players[data.race.playersshown[i]].score;
		data.race.timemin[i] = G_TicsToMinutes(players[data.race.playersshown[i]].realtime, true);
		data.race.timesec[i] = G_TicsToSeconds(players[data.race.playersshown[i]].realtime);
		data.race.timetic[i] = G_TicsToCentiseconds(players[data.race.playersshown[i]].realtime);
		data.race.rings[i] = players[data.race.playersshown[i]].health ? players[data.race.playersshown[i]].health-1 : 0;
		data.race.totalrings[i] = players[data.race.playersshown[i]].totalring;
		data.race.itemboxes[i] = players[data.race.playersshown[i]].numboxes;
		data.race.totalwins[i] = numwins[data.race.playersshown[i]];

		// Make sure it's in bounds so it won't screw up the display.
		if (data.race.scores[i] > 999999)
			data.race.scores[i] = 999999;
		if (data.race.timemin[i] > 99)
			data.race.timemin[i] = 99;
		if (data.race.rings[i] > 999)
			data.race.rings[i] = 999;
		if (data.race.totalrings[i] > 999)
			data.race.totalrings[i] = 999;
		if (data.race.itemboxes[i] > 999)
			data.race.itemboxes[i] = 999;
	}

	// Find the overall winner.
	for (i = 0, n = 0, wins = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || numwins[i] < wins)
			continue;

		if (numwins[i] == wins)
		{
			n++;
			continue;
		}

		n = 1; // number of players with this many wins
		wins = numwins[i]; // best number of wins so far
		data.race.winner = i; // winner so far
	}

	if (n != 1 || !wins)
		data.race.winner = -1; // tie
}

//
// Y_DrawScaledNum
//
// Dumb display function for positive numbers.
// Like ST_DrawOverlayNum, but scales the start and isn't translucent.
//
static void Y_DrawScaledNum(INT32 x, INT32 y, INT32 flags, INT32 num)
{
	INT32 w = SHORT(tallnum[0]->width);

	// special case for 0
	if (!num)
	{
		V_DrawScaledPatch(x - w, y, flags, tallnum[0]);
		return;
	}

#ifdef PARANOIA
	if (num < 0)
		I_Error("Intermission drawer used negative number!");
#endif

	// draw the number
	while (num)
	{
		x -= w;
		V_DrawScaledPatch(x, y, flags, tallnum[num % 10]);
		num /= 10;
	}
}

//
// Y_EndIntermission
//
void Y_EndIntermission(void)
{
	Y_UnloadData();

	endtic = -1;
	inttype = int_none;
}

//
// Y_EndGame
//
// Why end the game?
// Because Y_FollowIntermission and F_EndCutscene would
// both do this exact same thing *in different ways* otherwise,
// which made it so that you could only unlock Ultimate mode
// if you had a cutscene after the final level and crap like that.
// This function simplifies it so only one place has to be updated
// when something new is added.
void Y_EndGame(void)
{
	// Only do evaluation and credits in coop games.
	if (gametype == GT_COOP)
	{
		if (nextmap == 1102-1) // end game with credits
		{
			F_StartCredits();
			return;
		}
		if (nextmap == 1101-1) // end game with evaluation
		{
			F_StartGameEvaluation();
			return;
		}
	}

	// 1100 or competitive multiplayer, so go back to title screen.
	D_StartTitle();
}

//
// Y_FollowIntermission
//
static void Y_FollowIntermission(void)
{
	if (timeattacking)
	{
		char nameofdemo[256];
		snprintf(nameofdemo, sizeof nameofdemo,
			"%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%02d.lmp",
			srb2home, timeattackfolder, G_BuildMapName(gamemap),
			cv_chooseskin.value-1);
		nameofdemo[sizeof nameofdemo - 1] = '\0';
		G_CheckDemoStatus();

		Command_ExitGame_f();

		timeattacking = true;

		M_StartControlPanel();

		I_mkdir(va("%s"PATHSEP"replay", srb2home), 0755);
		I_mkdir(va("%s"PATHSEP"replay"PATHSEP"%s", srb2home, timeattackfolder), 0755);

		if (FIL_FileExists(va("replay/%s", timeattackfolder)))
		{
			if (FIL_FileExists(va("%s"PATHSEP"temp.lmp",srb2home)))
			{
				if (FIL_FileExists(nameofdemo))
					remove(nameofdemo);

				rename(va("%s"PATHSEP"temp.lmp",srb2home), nameofdemo);
			}
		}

		CV_AddValue(&cv_nextmap, 1);
		CV_AddValue(&cv_nextmap, -1);
		return;
	}

	if (nextmap < 1100-1)
	{
		// normal level
		G_AfterIntermission();
		return;
	}

	// Start a custom cutscene if there is one.
	if (mapheaderinfo[gamemap-1].cutscenenum)
	{
		F_StartCustomCutscene(mapheaderinfo[gamemap-1].cutscenenum-1, false, false);
		return;
	}

	Y_EndGame();
}

#define UNLOAD(x) Z_ChangeTag(x, PU_CACHE); x = NULL

//
// Y_UnloadData
//
static void Y_UnloadData(void)
{
	// In hardware mode, don't Z_ChangeTag a pointer returned by W_CachePatchName().
	// It doesn't work and is unnecessary.
	if (rendermode != render_soft)
		return;

	// unload the background patches
	UNLOAD(bgpatch);
	UNLOAD(widebgpatch);
	UNLOAD(bgtile);
	UNLOAD(interpic);

	switch (inttype)
	{
		case int_coop:
			// unload the coop and single player patches
			UNLOAD(data.coop.ttlnum);
			UNLOAD(data.coop.ptimebonus);
			UNLOAD(data.coop.pringbonus);
			UNLOAD(data.coop.pperfbonus);
			UNLOAD(data.coop.ptotal);
			break;
		case int_spec:
			// unload the special stage patches
			UNLOAD(data.spec.cemerald);
			UNLOAD(data.spec.pringbonus);
			UNLOAD(data.spec.cscore);
			UNLOAD(data.spec.nowsuper);
			break;
		case int_match:
			UNLOAD(data.match.result);
			break;
		case int_ctf:
			UNLOAD(data.match.blueflag);
			UNLOAD(data.match.redflag);
			break;
		case int_race:
		case int_classicrace:
			// unload the RESULT patch
			UNLOAD(data.race.result);
			break;
		default:
			//without this default, int_none, int_tag, int_ctf,
			//int_chaos, and int_classicrace are not handled
			break;
	}
}
