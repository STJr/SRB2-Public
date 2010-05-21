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
/// \brief Heads up display

#include "doomdef.h"
#include "byteptr.h"
#include "hu_stuff.h"

#include "m_menu.h" // gametype_cons_t

#include "d_clisrv.h"

#include "g_game.h"
#include "g_input.h"

#include "i_video.h"
#include "i_system.h"

#include "dstrings.h"
#include "st_stuff.h" // ST_HEIGHT
#include "r_local.h"

#include "keys.h"
#include "v_video.h"

#include "w_wad.h"
#include "z_zone.h"

#include "console.h"
#include "am_map.h"
#include "d_main.h"

#include "p_tick.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// coords are scaled
#define HU_INPUTX 0
#define HU_INPUTY 0

#define HU_SERVER_SAY	1	// Server message (dedicated).
#define HU_CSAY			2	// Server CECHOes to everyone.

//-------------------------------------------
//              heads up font
//-------------------------------------------
patch_t *hu_font[HU_FONTSIZE];
patch_t *tinynum[10]; // 0-9, tiny numbers

// Level title and credits fonts
patch_t *lt_font[LT_FONTSIZE];
patch_t *cred_font[CRED_FONTSIZE];

static player_t *plr;
boolean chat_on; // entering a chat message?
static char w_chat[HU_MAXMSGLEN];
static boolean headsupactive = false;
boolean hu_showscores; // draw rankings
static char hu_tick;

patch_t *rflagico;
patch_t *bflagico;
patch_t *rmatcico;
patch_t *bmatcico;
patch_t *tagico;

//-------------------------------------------
//              coop hud
//-------------------------------------------

patch_t *emeraldpics[7];
patch_t *tinyemeraldpics[7];
static patch_t *emblemicon;

//-------------------------------------------
//              misc vars
//-------------------------------------------

// crosshair 0 = off, 1 = cross, 2 = angle, 3 = point, see m_menu.c
static patch_t *crosshair[HU_CROSSHAIRS]; // 3 precached crosshair graphics

// -------
// protos.
// -------
static void HU_DrawRankings(void);
static void HU_DrawCoopOverlay(void);

//======================================================================
//                 KEYBOARD LAYOUTS FOR ENTERING TEXT
//======================================================================

char *shiftxform;

char english_shiftxform[] =
{
	0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31,
	' ', '!', '"', '#', '$', '%', '&',
	'"', // shift-'
	'(', ')', '*', '+',
	'<', // shift-,
	'_', // shift--
	'>', // shift-.
	'?', // shift-/
	')', // shift-0
	'!', // shift-1
	'@', // shift-2
	'#', // shift-3
	'$', // shift-4
	'%', // shift-5
	'^', // shift-6
	'&', // shift-7
	'*', // shift-8
	'(', // shift-9
	':',
	':', // shift-;
	'<',
	'+', // shift-=
	'>', '?', '@',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'[', // shift-[
	'!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
	']', // shift-]
	'"', '_',
	'~', // shift-` for some stupid reason this was a '
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'{', '|', '}', '~', 127
};

static char cechotext[1024];
static tic_t cechotimer = 0;
static tic_t cechoduration = 5*TICRATE;
static INT32 cechoflags = 0;

//======================================================================
//                          HEADS UP INIT
//======================================================================

#ifndef NONET
// just after
static void Command_Say_f(void);
static void Command_Sayto_f(void);
static void Command_Sayteam_f(void);
static void Command_CSay_f(void);
static void Got_Saycmd(UINT8 **p, INT32 playernum);
#endif

void HU_LoadGraphics(void)
{
	char buffer[9];
	INT32 i, j;

	if (dedicated)
		return;

	// cache the heads-up font for entire game execution
	j = HU_FONTSTART;
	for (i = 0; i < HU_FONTSIZE; i++)
	{
		sprintf(buffer, "STCFN%.3d", j);
		j++;
		if (i >= HU_REALFONTSIZE && i != '~' - HU_FONTSTART && i != '`' - HU_FONTSTART) /// \note font end hack
			continue;
		hu_font[i] = (patch_t *)W_CachePatchName(buffer, PU_HUDGFX);
	}

	// cache the level title font for entire game execution
	lt_font[0] = (patch_t *)W_CachePatchName("LTFNT039", PU_HUDGFX); /// \note fake start hack

	// Number support
	lt_font[9] = (patch_t *)W_CachePatchName("LTFNT048", PU_HUDGFX);
	lt_font[10] = (patch_t *)W_CachePatchName("LTFNT049", PU_HUDGFX);
	lt_font[11] = (patch_t *)W_CachePatchName("LTFNT050", PU_HUDGFX);
	lt_font[12] = (patch_t *)W_CachePatchName("LTFNT051", PU_HUDGFX);
	lt_font[13] = (patch_t *)W_CachePatchName("LTFNT052", PU_HUDGFX);
	lt_font[14] = (patch_t *)W_CachePatchName("LTFNT053", PU_HUDGFX);
	lt_font[15] = (patch_t *)W_CachePatchName("LTFNT054", PU_HUDGFX);
	lt_font[16] = (patch_t *)W_CachePatchName("LTFNT055", PU_HUDGFX);
	lt_font[17] = (patch_t *)W_CachePatchName("LTFNT056", PU_HUDGFX);
	lt_font[18] = (patch_t *)W_CachePatchName("LTFNT057", PU_HUDGFX);

	j = LT_REALFONTSTART;
	for (i = LT_REALFONTSTART - LT_FONTSTART; i < LT_FONTSIZE; i++)
	{
		sprintf(buffer, "LTFNT%.3d", j);
		j++;
		lt_font[i] = (patch_t *)W_CachePatchName(buffer, PU_HUDGFX);
	}

	// cache the credits font for entire game execution (why not?)
	j = CRED_FONTSTART;
	for (i = 0; i < CRED_FONTSIZE; i++)
	{
		sprintf(buffer, "CRFNT%.3d", j);
		j++;
		cred_font[i] = (patch_t *)W_CachePatchName(buffer, PU_HUDGFX);
	}

	//cache tiny numbers too!
	for (i = 0; i < 10; i++)
	{
		sprintf(buffer, "TINYNUM%d", i);
		tinynum[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// cache the crosshairs, don't bother to know which one is being used,
	// just cache all 3, they're so small anyway.
	for (i = 0; i < HU_CROSSHAIRS; i++)
	{
		sprintf(buffer, "CROSHAI%c", '1'+i);
		crosshair[i] = (patch_t *)W_CachePatchName(buffer, PU_HUDGFX);
	}

	emblemicon = W_CachePatchName("EMBLICON", PU_HUDGFX);
	emeraldpics[0] = W_CachePatchName("CHAOS1", PU_HUDGFX);
	emeraldpics[1] = W_CachePatchName("CHAOS2", PU_HUDGFX);
	emeraldpics[2] = W_CachePatchName("CHAOS3", PU_HUDGFX);
	emeraldpics[3] = W_CachePatchName("CHAOS4", PU_HUDGFX);
	emeraldpics[4] = W_CachePatchName("CHAOS5", PU_HUDGFX);
	emeraldpics[5] = W_CachePatchName("CHAOS6", PU_HUDGFX);
	emeraldpics[6] = W_CachePatchName("CHAOS7", PU_HUDGFX);
	tinyemeraldpics[0] = W_CachePatchName("TEMER1", PU_HUDGFX);
	tinyemeraldpics[1] = W_CachePatchName("TEMER2", PU_HUDGFX);
	tinyemeraldpics[2] = W_CachePatchName("TEMER3", PU_HUDGFX);
	tinyemeraldpics[3] = W_CachePatchName("TEMER4", PU_HUDGFX);
	tinyemeraldpics[4] = W_CachePatchName("TEMER5", PU_HUDGFX);
	tinyemeraldpics[5] = W_CachePatchName("TEMER6", PU_HUDGFX);
	tinyemeraldpics[6] = W_CachePatchName("TEMER7", PU_HUDGFX);
}

// Initialise Heads up
// once at game startup.
//
void HU_Init(void)
{
#ifndef NONET
	COM_AddCommand("say", Command_Say_f);
	COM_AddCommand("sayto", Command_Sayto_f);
	COM_AddCommand("sayteam", Command_Sayteam_f);
	COM_AddCommand("csay", Command_CSay_f);
	RegisterNetXCmd(XD_SAY, Got_Saycmd);
#endif

	// set shift translation table
	shiftxform = english_shiftxform;

	HU_LoadGraphics();
}

static inline void HU_Stop(void)
{
	headsupactive = false;
}

//
// Reset Heads up when consoleplayer spawns
//
void HU_Start(void)
{
	if (headsupactive)
		HU_Stop();

	plr = &players[consoleplayer];

	headsupactive = true;
}

//======================================================================
//                            EXECUTION
//======================================================================

void MatchType_OnChange(void)
{
	INT32 i;

	// Do not execute the below code unless absolutely necessary.
	if (gametype != GT_MATCH || gamestate != GS_LEVEL || cv_matchtype.value == matchtype)
		return;

	// If swapping to team match, ensure that all players that aren't already on a team become
	// a spectator, or join the team of their color, if availiable. The quirk of this new gamtype
	// handling causes us to have to do this. -Jazz 3/4/09
	if (cv_matchtype.value)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator)
			{
				if (server)
				{
					changeteam_union NetPacket;
					UINT16 usvalue;
					NetPacket.value.l = NetPacket.value.b = 0;

					NetPacket.packet.playernum = i;
					NetPacket.packet.verification = true;

					if (players[i].skincolor == 6) //red
						NetPacket.packet.newteam = 1;
					else if (players[i].skincolor == 7) //blue
						NetPacket.packet.newteam = 2;
					else //swap to spectator
						NetPacket.packet.newteam = 0;

					usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
					SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
				}
			}
		}
	}
	else
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator)
				players[i].ctfteam = 0;
		}
	}

	matchtype = cv_matchtype.value;
}

#ifndef NONET
/** Runs a say command, sending an ::XD_SAY message.
  * A say command consists of a signed 8-bit integer for the target, an
  * unsigned 8-bit flag variable, and then the message itself.
  *
  * The target is 0 to say to everyone, 1 to 32 to say to that player, or -1
  * to -32 to say to everyone on that player's team. Note: This means you
  * have to add 1 to the player number, since they are 0 to 31 internally.
  *
  * The flag HU_SERVER_SAY will be set if it is the dedicated server speaking.
  *
  * This function obtains the message using COM_Argc() and COM_Argv().
  *
  * \param target    Target to send message to.
  * \param usedargs  Number of arguments to ignore.
  * \param flags     Set HU_CSAY for server/admin to CECHO everyone.
  * \sa Command_Say_f, Command_Sayteam_f, Command_Sayto_f, Got_Saycmd
  * \author Graue <graue@oceanbase.org>
  */
static void DoSayCommand(SINT8 target, size_t usedargs, UINT8 flags)
{
	XBOXSTATIC char buf[254];
	size_t numwords, ix;
	char *msg = &buf[2];
	const size_t msgspace = sizeof buf - 2;

	numwords = COM_Argc() - usedargs;
	I_Assert(numwords > 0);

	if (cv_mute.value && !(server || adminplayer == consoleplayer))
	{
		CONS_Printf("The chat is muted. You can't say anything at the moment.\n");
		return;
	}

	// Only servers/admins can CSAY.
	if(!server && adminplayer != consoleplayer)
		flags &= ~HU_CSAY;

	// We handle HU_SERVER_SAY, not the caller.
	flags &= ~HU_SERVER_SAY;
	if(dedicated && !(flags & HU_CSAY))
		flags |= HU_SERVER_SAY;

	buf[0] = target;
	buf[1] = flags;
	msg[0] = '\0';

	for (ix = 0; ix < numwords; ix++)
	{
		if (ix > 0)
			strlcat(msg, " ", msgspace);
		strlcat(msg, COM_Argv(ix + usedargs), msgspace);
	}

	SendNetXCmd(XD_SAY, buf, strlen(msg) + 1 + msg-buf);
}

/** Send a message to everyone.
  * \sa DoSayCommand, Command_Sayteam_f, Command_Sayto_f
  * \author Graue <graue@oceanbase.org>
  */
static void Command_Say_f(void)
{
	if (COM_Argc() < 2)
	{
		CONS_Printf("say <message>: send a message\n");
		return;
	}

	DoSayCommand(0, 1, 0);
}

/** Send a message to a particular person.
  * \sa DoSayCommand, Command_Sayteam_f, Command_Say_f
  * \author Graue <graue@oceanbase.org>
  */
static void Command_Sayto_f(void)
{
	INT32 target;

	if (COM_Argc() < 3)
	{
		CONS_Printf("sayto <playername|playernum> <message>: send a message to a player\n");
		return;
	}

	target = nametonum(COM_Argv(1));
	if (target == -1)
	{
		CONS_Printf("sayto: No player with that name!\n");
		return;
	}
	target++; // Internally we use 0 to 31, but say command uses 1 to 32.

	DoSayCommand((SINT8)target, 2, 0);
}

/** Send a message to members of the player's team.
  * \sa DoSayCommand, Command_Say_f, Command_Sayto_f
  * \author Graue <graue@oceanbase.org>
  */
static void Command_Sayteam_f(void)
{
	if (COM_Argc() < 2)
	{
		CONS_Printf("sayteam <message>: send a message to your team\n");
		return;
	}

	if (dedicated)
	{
		CONS_Printf("Dedicated servers can't send team messages. Use \"say\".\n");
		return;
	}

	DoSayCommand((SINT8)(-(consoleplayer+1)), 1, 0);
}

/** Send a message to everyone, to be displayed by CECHO. Only
  * permitted to servers and admins.
  */
static void Command_CSay_f(void)
{
	if (COM_Argc() < 2)
	{
		CONS_Printf("csay <message>: send a message to be shown in the middle of the screen\n");
		return;
	}

	if(!server && adminplayer != consoleplayer)
	{
		CONS_Printf("Only servers and admins can use csay.\n");
		return;
	}

	DoSayCommand(0, 1, HU_CSAY);
}

/** Receives a message, processing an ::XD_SAY command.
  * \sa DoSayCommand
  * \author Graue <graue@oceanbase.org>
  */
static void Got_Saycmd(UINT8 **p, INT32 playernum)
{
	SINT8 target;
	UINT8 flags;
	const char *dispname;
	char *msg;
	boolean action = false;
	char *ptr;

	target = READSINT8(*p);
	flags = READUINT8(*p);

	if ((cv_mute.value || (flags & HU_CSAY)) && playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(cv_mute.value ?
			"Illegal say command received from %s while muted\n" :
			"Illegal csay command received from non-admin %s\n",
			player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	msg = (char *)*p;
	SKIPSTRING(*p);

	//check for invalid characters (0x80 or above)
	{
		size_t i;
		const size_t j = strlen(msg);
		for (i = 0; i < j; i++)
		{
			if (msg[i] & 0x80)
			{
				CONS_Printf("Illegal say command received from %s containing invalid characters\n",
					player_names[playernum]);
				if (server)
				{
					XBOXSTATIC char buf[2];

					buf[0] = (char)playernum;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}
				return;
			}
		}
	}

	// If it's a CSAY, just CECHO and be done with it.
	if(flags & HU_CSAY)
	{
		HU_SetCEchoDuration(5);
		I_OutputMsg("Server message: %s\n", msg);
		HU_DoCEcho(msg);
		return;
	}

	// Handle "/me" actions, but only in messages to everyone.
	if (target == 0 && strlen(msg) > 4 && strnicmp(msg, "/me ", 4) == 0)
	{
		msg += 4;
		action = true;
	}

	if (flags & HU_SERVER_SAY)
	{
		if (playernum == 0)
			dispname = "SERVER";
		else // HAX!
			return;
	}
	else
		dispname = player_names[playernum];

	// Clean up message a bit
	// If you use a \r character, you can remove your name
	// from before the text and then pretend to be someone else!
	ptr = msg;
	while (*ptr != '\0')
	{
		if (*ptr == '\r')
			*ptr = ' ';

		ptr++;
	}

	// Show messages sent by you, to you, to your team, or to everyone:
	if (consoleplayer == playernum // By you
		|| consoleplayer == target-1 // To you
		|| (target < 0 && ST_SameTeam(&players[consoleplayer],
			&players[-target - 1])) // To your team
		|| target == 0) // To everyone
	{
		const char *cstart = "", *cend = "", *adminchar = "~", *remotechar = "@", *fmt;
		char *tempchar = NULL;

		// In CTF and team match, color the player's name.
		if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
		{
			cend = "\x80";
			if (players[playernum].ctfteam == 1) // red
				cstart = "\x85";
			else if (players[playernum].ctfteam == 2) // blue
				cstart = "\x84";
		}

		// Give admins and remote admins their symbols.
		if (playernum == serverplayer || playernum == adminplayer)
		{
			// Give the admin his special character ala SRB2Live.
			if (playernum == serverplayer)
				tempchar = (char *)calloc(strlen(cstart) + strlen(adminchar) + 1, sizeof(char));
			else
				tempchar = (char *)calloc(strlen(cstart) + strlen(remotechar) + 1, sizeof(char));

			if (tempchar)
			{
				strcat(tempchar, cstart);

				if (playernum == serverplayer)
					strcat(tempchar, adminchar);
				else
					strcat(tempchar, remotechar);

				cstart = tempchar;
			}
		}

		// Choose the proper format string for display.
		// Each format includes four strings: color start, display
		// name, color end, and the message itself.
		// '\4' makes the message yellow and beeps; '\3' just beeps.
		if (action)
			fmt = "\4* %s%s%s \x82%s\n";
		else if (target == 0) // To everyone
			fmt = "\3<%s%s%s> %s\n";
		else if (target-1 == consoleplayer) // To you
			fmt = "\3*%s%s%s* %s\n";
		else if (target > 0) // By you, to another player
		{
			// Use target's name.
			dispname = player_names[target-1];
			fmt = "\3->*%s%s%s* %s\n";
		}
		else // To your team
			fmt = "\3>>%s%s%s<< (team) %s\n";

		CONS_Printf(fmt, cstart, dispname, cend, msg);

		if (tempchar)
			free(tempchar);
	}
}
#endif

// Handles key input and string input
//
static inline boolean HU_keyInChatString(char *s, char ch)
{
	size_t l;

	if (ch >= ' ' && (ch <= 'z' || ch == '~' || ch == '`')) /// \note font end hack
	{
		l = strlen(s);
		if (l < HU_MAXMSGLEN - 1)
		{
			s[l++] = ch;
			s[l]=0;
			return true;
		}
		return false;
	}
	else if (ch == KEY_BACKSPACE)
	{
		l = strlen(s);
		if (l)
			s[--l] = 0;
		else
			return false;
	}
	else if (ch != KEY_ENTER)
		return false; // did not eat key

	return true; // ate the key
}

//
//
void HU_Ticker(void)
{
	if (dedicated)
		return;

	hu_tick++;
	hu_tick &= 7; // currently only to blink chat input cursor

	if ((gamekeydown[gamecontrol[gc_scores][0]] || gamekeydown[gamecontrol[gc_scores][1]]))
		hu_showscores = !chat_on;
	else
		hu_showscores = false;
}

#define QUEUESIZE 256

static boolean teamtalk = false;
static char chatchars[QUEUESIZE];
static INT32 head = 0, tail = 0;

//
// HU_dequeueChatChar
//
char HU_dequeueChatChar(void)
{
	char c;

	if (head != tail)
	{
		c = chatchars[tail];
		tail = (tail + 1) & (QUEUESIZE-1);
	}
	else
		c = 0;

	return c;
}

//
//
static void HU_queueChatChar(char c)
{
	if (((head + 1) & (QUEUESIZE-1)) == tail)
		CONS_Printf("%s", text[HUSTR_MSGU]); // message not sent
	else
	{
		if (c == KEY_BACKSPACE)
		{
			if (tail != head)
				head = (head - 1) & (QUEUESIZE-1);
		}
		else
		{
			chatchars[head] = c;
			head = (head + 1) & (QUEUESIZE-1);
		}
	}

	// send automaticly the message (no more chat char)
	if (c == KEY_ENTER)
	{
		char buf[255];
		size_t ci = 0;

		do {
			c = HU_dequeueChatChar();
			if (c != 13) // Graue 07-04-2004: don't know why this has to be done
				buf[ci++]=c;
		} while (c);
		// Graue 09-04-2004: 1 not 2, hell if I know why
		if (ci > 1) // Graue 07-02-2004: 2 not 3, with HU_BROADCAST disposed of
		{
			if (teamtalk)
				COM_BufInsertText(va("sayteam \"%s\"", buf)); // Graue 07-04-2004: quote it!
			else
				COM_BufInsertText(va("say \"%s\"", buf)); // Graue 07-04-2004: quote it!
		}
	}
}

void HU_clearChatChars(void)
{
	while (tail != head)
		HU_queueChatChar(KEY_BACKSPACE);
	chat_on = false;
}

//
// Returns true if key eaten
//
boolean HU_Responder(event_t *ev)
{
	static boolean shiftdown = false, altdown = false;
	boolean eatkey = false;
	UINT8 c;

	if (ev->data1 == KEY_LSHIFT || ev->data1 == KEY_RSHIFT)
	{
		shiftdown = (ev->type == ev_keydown);
		return chat_on;
	}
	else if (ev->data1 == KEY_LALT || ev->data1 == KEY_RALT)
	{
		altdown = (ev->type == ev_keydown);
		return false;
	}

	if (ev->type != ev_keydown)
		return false;

	// only KeyDown events now...

	if (!chat_on)
	{
		// enter chat mode
		if ((ev->data1 == gamecontrol[gc_talkkey][0] || ev->data1 == gamecontrol[gc_talkkey][1])
			&& netgame && (!cv_mute.value || server || (adminplayer == consoleplayer)))
		{
			eatkey = chat_on = true;
			w_chat[0] = 0;
			teamtalk = false;
		}
		if ((ev->data1 == gamecontrol[gc_teamkey][0] || ev->data1 == gamecontrol[gc_teamkey][1])
			&& netgame && (!cv_mute.value || server || (adminplayer == consoleplayer)))
		{
			eatkey = chat_on = true;
			w_chat[0] = 0;
			teamtalk = true;
		}
	}
	else
	{
		c = (UINT8)ev->data1;

		// use console translations
		if (shiftdown)
			c = shiftxform[c];

		if (c == '"') // Graue 07-04-2004: quote marks mess it up
			c = '\'';

		eatkey = HU_keyInChatString(w_chat,c);
		if (eatkey)
			HU_queueChatChar(c);
		if (c == KEY_ENTER)
			chat_on = false;
		else if (c == KEY_ESCAPE)
			chat_on = false;

		eatkey = true;
	}

	return eatkey;
}

//======================================================================
//                         HEADS UP DRAWING
//======================================================================

//
// HU_DrawChat
//
// Draw chat input
//
static void HU_DrawChat(void)
{
	INT32 t = 0, c = 0, y = HU_INPUTY;
	size_t i = 0;
	const char *ntalk = "Say: ", *ttalk = "Say-Team: ";
	const char *talk = ntalk;

	if (teamtalk)
	{
		talk = ttalk;
#if 0
		     if (players[consoleplayer].ctfteam == 1)
			t = 0x500;  // Red
		else if (players[consoleplayer].ctfteam == 2)
			t = 0x400; // Blue
#endif
	}

	while (talk[i])
	{
		V_DrawCharacter(HU_INPUTX + (c<<3), y, talk[i++] | V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);
		c++;
	}

	i = 0;
	while (w_chat[i])
	{
		//Hurdler: isn't it better like that?
		V_DrawCharacter(HU_INPUTX + (c<<3), y, w_chat[i++] | V_NOSCALEPATCH|V_NOSCALESTART|t, !cv_allcaps.value);

		c++;
		if (c >= (vid.width>>3))
		{
			c = 0;
			y += 8;
		}
	}

	if (hu_tick < 4)
		V_DrawCharacter(HU_INPUTX + (c<<3), y, '_' | V_NOSCALEPATCH|V_NOSCALESTART|t, !cv_allcaps.value);
}


// draw the Crosshair, at the exact center of the view.
//
// Crosshairs are pre-cached at HU_Init

static inline void HU_DrawCrosshair(void)
{
	INT32 i, y;

	i = cv_crosshair.value & 3;
	if (!i)
		return;

	if ((netgame || multiplayer) && players[displayplayer].spectator)
		return;

#ifdef HWRENDER
	if (rendermode != render_soft)
		y = (INT32)gr_basewindowcentery;
	else
#endif
		y = viewwindowy + (viewheight>>1);

	V_DrawTranslucentPatch(vid.width>>1, y, V_NOSCALESTART, crosshair[i - 1]);
}

static inline void HU_DrawCrosshair2(void)
{
	INT32 i, y;

	i = cv_crosshair2.value & 3;
	if (!i)
		return;

	if ((netgame || multiplayer) && players[secondarydisplayplayer].spectator)
		return;

#ifdef HWRENDER
	if (rendermode != render_soft)
		y = (INT32)gr_basewindowcentery;
	else
#endif
		y = viewwindowy + (viewheight>>1);

	if (splitscreen)
	{
#ifdef HWRENDER
		if (rendermode != render_soft)
			y += (INT32)gr_viewheight;
		else
#endif
			y += viewheight;

		V_DrawTranslucentPatch(vid.width>>1, y, V_NOSCALESTART, crosshair[i - 1]);
	}
}

static void HU_drawGametype(void)
{
	INT32 i = 0;

	if (gametype == GT_COOP)
		i = 0;
	else if (gametype == GT_MATCH)
	{
		if (!cv_matchtype.value)
			i = 1;
		else
			i = 2;
	}
	else if (gametype == GT_RACE)
	{
		if (!cv_racetype.value)
			i = 3;
		else
			i = 4;
	}
	else if (gametype == GT_TAG)
	{
		if (!cv_tagtype.value)
			i = 5;
		else
			i = 6;
	}
	else if (gametype == GT_CTF)
		i = 7;

	if (splitscreen)
		V_DrawString(4, 184, 0, gametype_cons_t[i].strvalue);
	else
		V_DrawString(4, 192, 0, gametype_cons_t[i].strvalue);
}

#define MAXCECHOLINES 16

// Heads up displays drawer, call each frame
//
void HU_Drawer(void)
{
	// draw chat string plus cursor
	if (chat_on)
		HU_DrawChat();

	if (gamestate == GS_INTERMISSION || gamestate == GS_CUTSCENE || gamestate == GS_CREDITS)
		return;

	// draw multiplayer rankings
	if (hu_showscores)
	{
		if (gametype == GT_MATCH || gametype == GT_RACE || gametype == GT_TAG || gametype == GT_CTF
#ifdef CHAOSISNOTDEADYET
			|| gametype == GT_CHAOS
#endif
			)
			HU_DrawRankings();
		else if (gametype == GT_COOP)
		{
			HU_DrawCoopOverlay();

			if (multiplayer || netgame)
				HU_DrawRankings();
		}
	}

	// draw the crosshair, not when viewing demos nor with chasecam
	if (!automapactive && cv_crosshair.value && !demoplayback && !cv_chasecam.value)
		HU_DrawCrosshair();

	if (!automapactive && cv_crosshair2.value && !demoplayback && !cv_chasecam2.value)
		HU_DrawCrosshair2();

	if (cechotimer)
	{
		INT32 i = 0;
		INT32 y = (BASEVIDHEIGHT/2)-4;
		INT32 pnumlines = 0;
		char *line;
		char *echoptr;
		char temp[1024];

		cechotimer--;

		while (cechotext[i] != '\0')
		{
			if (cechotext[i] == '\\' && pnumlines < MAXCECHOLINES)
				pnumlines++;

			i++;
		}

		y -= (pnumlines-1)*6;

		strcpy(temp, cechotext);

		echoptr = &temp[0];

		while (*echoptr != '\0')
		{
			line = strchr(echoptr, '\\');

			if (line == NULL)
				return;

			*line = '\0';

			V_DrawCenteredString(BASEVIDWIDTH/2, y, cechoflags, echoptr);
			y += 12;

			echoptr = line;
			echoptr++;
		}
	}
}

//======================================================================
//                 HUD MESSAGES CLEARING FROM SCREEN
//======================================================================

// Clear old messages from the borders around the view window
// (only for reduced view, refresh the borders when needed)
//
// startline: y coord to start clear,
// clearlines: how many lines to clear.
//
static INT32 oldclearlines;

void HU_Erase(void)
{
	INT32 topline, bottomline;
	INT32 y, yoffset;

	// clear hud msgs on double buffer (OpenGL mode)
	boolean secondframe;
	static INT32 secondframelines;

	if (con_clearlines == oldclearlines && !con_hudupdate && !chat_on)
		return;

	// clear the other frame in double-buffer modes
	secondframe = (con_clearlines != oldclearlines);
	if (secondframe)
		secondframelines = oldclearlines;

	// clear the message lines that go away, so use _oldclearlines_
	bottomline = oldclearlines;
	oldclearlines = con_clearlines;
	if (chat_on)
		if (bottomline < 8)
			bottomline = 8;

	if (automapactive || viewwindowx == 0) // hud msgs don't need to be cleared
		return;

	// software mode copies view border pattern & beveled edges from the backbuffer
	if (rendermode == render_soft)
	{
		topline = 0;
		for (y = topline, yoffset = y*vid.width; y < bottomline; y++, yoffset += vid.width)
		{
			if (y < viewwindowy || y >= viewwindowy + viewheight)
				R_VideoErase(yoffset, vid.width); // erase entire line
			else
			{
				R_VideoErase(yoffset, viewwindowx); // erase left border
				// erase right border
				R_VideoErase(yoffset + viewwindowx + viewwidth, viewwindowx);
			}
		}
		con_hudupdate = false; // if it was set..
	}
#ifdef HWRENDER
	else if (rendermode != render_none)
	{
		// refresh just what is needed from the view borders
		HWR_DrawViewBorder(secondframelines);
		con_hudupdate = secondframe;
	}
#endif
}

//======================================================================
//                   IN-LEVEL MULTIPLAYER RANKINGS
//======================================================================

//
// HU_DrawTabRankings
//
void HU_DrawTabRankings(INT32 x, INT32 y, playersort_t *tab, INT32 scorelines, INT32 whiteplayer)
{
	INT32 i;
	const UINT8 *colormap;

	//this function is designed for 9 or less score lines only
	I_Assert(scorelines <= 9);

	V_DrawFill(1, 26, 318, 1, 0); //Draw a horizontal line because it looks nice!

	for (i = 0; i < scorelines; i++)
	{
		if (players[tab[i].num].spectator)
			continue; //ignore them.

		V_DrawString(x + 24, y,
		             ((tab[i].num == whiteplayer) ? V_YELLOWMAP : 0)
		             | ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT)
		             | V_ALLOWLOWERCASE, tab[i].name);

		// Draw emeralds
		if (!players[tab[i].num].powers[pw_super]
			|| ((leveltime/7) & 1))
		{
			HU_DrawEmeralds(x-12,y+2,tab[i].emeralds);
		}

		if (tab[i].color == 0)
		{
			colormap = colormaps;
			if (players[tab[i].num].powers[pw_super])
				V_DrawSmallScaledPatch (x, y-4, 0, superprefix[players[tab[i].num].skin]);
			else
			{
				if (players[tab[i].num].health <= 0)
					V_DrawSmallTranslucentPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin]);
				else
					V_DrawSmallScaledPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin]);
			}
		}
		else
		{
			if (players[tab[i].num].powers[pw_super])
			{
				colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (((players[tab[i].num].powers[pw_super]) ? 15 : players[tab[i].num].skincolor)<<8);
				V_DrawSmallMappedPatch (x, y-4, 0, superprefix[players[tab[i].num].skin], colormap);
			}
			else
			{
				colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (tab[i].color<<8);
				if (players[tab[i].num].health <= 0)
					V_DrawSmallTranslucentMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
				else
					V_DrawSmallMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
			}
		}

		if (gametype == GT_TAG && players[tab[i].num].pflags & PF_TAGIT)
			V_DrawSmallScaledPatch(x-32, y-4, 0, tagico);

		if (gametype == GT_RACE)
		{
			if (circuitmap)
			{
				if (players[tab[i].num].exiting)
					V_DrawRightAlignedString(x+240, y, 0, va("%i:%02i.%02i", G_TicsToMinutes(players[tab[i].num].realtime,true), G_TicsToSeconds(players[tab[i].num].realtime), G_TicsToCentiseconds(players[tab[i].num].realtime)));
				else
					V_DrawRightAlignedString(x+240, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%u", tab[i].count));
			}
			else
				V_DrawRightAlignedString(x+240, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%i:%02i.%02i", G_TicsToMinutes(tab[i].count,true), G_TicsToSeconds(tab[i].count), G_TicsToCentiseconds(tab[i].count)));
		}
		else
			V_DrawRightAlignedString(x+240, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%u", tab[i].count));

		y += 16;
	}
}

//
// HU_DrawTeamTabRankings
//
void HU_DrawTeamTabRankings(playersort_t *tab, INT32 whiteplayer)
{
	INT32 i,x,y;
	INT32 redplayers = 0, blueplayers = 0;
	const UINT8 *colormap;
	char name[MAXPLAYERNAME+1];

	V_DrawFill(160, 26, 1, 154, 0); //Draw a vertical line to separate the two teams.
	V_DrawFill(1, 26, 318, 1, 0); //And a horizontal line to make a T.
	V_DrawFill(1, 180, 318, 1, 0); //And a horizontal line near the bottom.

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (players[tab[i].num].spectator)
			continue; //ignore them.

		if (tab[i].color == 6) //red
		{
			if (redplayers++ > 8)
				continue;
			x = 32 + (BASEVIDWIDTH/2);
			y = (redplayers * 16) + 16;
		}
		else if (tab[i].color == 7) //blue
		{
			if (blueplayers++ > 8)
				continue;
			x = 32;
			y = (blueplayers * 16) + 16;
		}
		else //er?  not on red or blue, so ignore them
			continue;

		strlcpy(name, tab[i].name, 9);
		V_DrawString(x + 24, y,
		             ((tab[i].num == whiteplayer) ? V_YELLOWMAP : 0)
		             | ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT)
		             | V_ALLOWLOWERCASE, name);

		if (gametype == GT_CTF)
		{
			if (players[tab[i].num].gotflag & MF_REDFLAG) // Red
				V_DrawSmallScaledPatch(x-28, y-4, 0, rflagico);
			else if (players[tab[i].num].gotflag & MF_BLUEFLAG) // Blue
				V_DrawSmallScaledPatch(x-28, y-4, 0, bflagico);
		}

		// Draw emeralds
		if (!players[tab[i].num].powers[pw_super]
			|| ((leveltime/7) & 1))
		{
			HU_DrawEmeralds(x-12,y+2,tab[i].emeralds);
		}

		if (players[tab[i].num].powers[pw_super])
		{
			colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (((players[tab[i].num].powers[pw_super]) ? 15 : players[tab[i].num].skincolor)<<8);
			V_DrawSmallMappedPatch (x, y-4, 0, superprefix[players[tab[i].num].skin], colormap);
		}
		else
		{
			colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (tab[i].color<<8);
			if (players[tab[i].num].health <= 0)
				V_DrawSmallTranslucentMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
			else
				V_DrawSmallMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
		}
		V_DrawRightAlignedString(x+120, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%u", tab[i].count));
	}
}

//
// HU_DrawDualTabRankings
//
void HU_DrawDualTabRankings(INT32 x, INT32 y, playersort_t *tab, INT32 scorelines, INT32 whiteplayer)
{
	INT32 i;
	const UINT8 *colormap;
	char name[MAXPLAYERNAME+1];

	V_DrawFill(160, 26, 1, 154, 0); //Draw a vertical line to separate the two sides.
	V_DrawFill(1, 26, 318, 1, 0); //And a horizontal line to make a T.
	V_DrawFill(1, 180, 318, 1, 0); //And a horizontal line near the bottom.

	if (gametype == GT_RACE || gametype == GT_COOP)
		x -= 32; //we need more room!

	for (i = 0; i < scorelines; i++)
	{
		if (players[tab[i].num].spectator)
			continue; //ignore them.

		strlcpy(name, tab[i].name, 9);
		V_DrawString(x + 24, y,
		             ((tab[i].num == whiteplayer) ? V_YELLOWMAP : 0)
		             | ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT)
		             | V_ALLOWLOWERCASE, name);

		if (gametype == GT_TAG && players[tab[i].num].pflags & PF_TAGIT)
			V_DrawSmallScaledPatch(x-28, y-4, 0, tagico);

		// Draw emeralds
		if (!players[tab[i].num].powers[pw_super]
			|| ((leveltime/7) & 1))
		{
			HU_DrawEmeralds(x-12,y+2,tab[i].emeralds);
		}

		if (tab[i].color == 0)
		{
			colormap = colormaps;
			if (players[tab[i].num].powers[pw_super])
				V_DrawSmallScaledPatch (x, y-4, 0, superprefix[players[tab[i].num].skin]);
			else
			{
				if (players[tab[i].num].health <= 0)
					V_DrawSmallTranslucentPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin]);
				else
					V_DrawSmallScaledPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin]);
			}
		}
		else
		{
			if (players[tab[i].num].powers[pw_super])
			{
				colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (((players[tab[i].num].powers[pw_super]) ? 15 : players[tab[i].num].skincolor)<<8);
				V_DrawSmallMappedPatch (x, y-4, 0, superprefix[players[tab[i].num].skin], colormap);
			}
			else
			{
				colormap = (const UINT8 *) translationtables[players[tab[i].num].skin] - 256 + (tab[i].color<<8);
				if (players[tab[i].num].health <= 0)
					V_DrawSmallTranslucentMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
				else
					V_DrawSmallMappedPatch (x, y-4, 0, faceprefix[players[tab[i].num].skin], colormap);
			}
		}

		if (gametype == GT_RACE)
		{
			if (circuitmap)
			{
				if (players[tab[i].num].exiting)
					V_DrawRightAlignedString(x+156, y, 0, va("%i:%02i.%02i", G_TicsToMinutes(players[tab[i].num].realtime,true), G_TicsToSeconds(players[tab[i].num].realtime), G_TicsToCentiseconds(players[tab[i].num].realtime)));
				else
					V_DrawRightAlignedString(x+156, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%u", tab[i].count));
			}
			else
				V_DrawRightAlignedString(x+156, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%i:%02i.%02i", G_TicsToMinutes(tab[i].count,true), G_TicsToSeconds(tab[i].count), G_TicsToCentiseconds(tab[i].count)));
		}
		else
			V_DrawRightAlignedString(x+120, y, ((players[tab[i].num].health > 0) ? 0 : V_TRANSLUCENT), va("%u", tab[i].count));

		y += 16;
		if (y > 160)
		{
			y = 32;
			x += BASEVIDWIDTH/2;
		}
	}
}

//
// HU_DrawEmeralds
//
void HU_DrawEmeralds(INT32 x, INT32 y, INT32 pemeralds)
{
	//Draw the emeralds, in the CORRECT order, using tiny emerald sprites.
	if (pemeralds & EMERALD1)
		V_DrawSmallScaledPatch(x  , y-6, 0, tinyemeraldpics[0]);

	if (pemeralds & EMERALD2)
		V_DrawSmallScaledPatch(x+4, y-3, 0, tinyemeraldpics[1]);

	if (pemeralds & EMERALD3)
		V_DrawSmallScaledPatch(x+4, y+3, 0, tinyemeraldpics[2]);

	if (pemeralds & EMERALD4)
		V_DrawSmallScaledPatch(x  , y+6, 0, tinyemeraldpics[3]);

	if (pemeralds & EMERALD5)
		V_DrawSmallScaledPatch(x-4, y+3, 0, tinyemeraldpics[4]);

	if (pemeralds & EMERALD6)
		V_DrawSmallScaledPatch(x-4, y-3, 0, tinyemeraldpics[5]);

	if (pemeralds & EMERALD7)
		V_DrawSmallScaledPatch(x,   y,   0, tinyemeraldpics[6]);
}

//
// HU_DrawSpectatorTicker
//
static inline void HU_DrawSpectatorTicker(void)
{
	int i;
	int length = 0, height = 174;
	int totallength = 0, templength = 0;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].spectator)
			totallength += (signed)strlen(player_names[i]) * 8 + 16;

	length -= (leveltime % (totallength + BASEVIDWIDTH));
	length += BASEVIDWIDTH;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].spectator)
		{
			char *pos;
			char initial[MAXPLAYERNAME+1];
			char current[MAXPLAYERNAME+1];

			strcpy(initial, player_names[i]);
			pos = initial;

			if (length >= -((signed)strlen(player_names[i]) * 8 + 16) && length <= BASEVIDWIDTH)
			{
				if (length < 0)
				{
					UINT8 eatenchars = (UINT8)(abs(length) / 8 + 1);

					if (eatenchars <= strlen(initial))
					{
						// Eat one letter off the left side,
						// then compensate the drawing position.
						pos += eatenchars;
						strcpy(current, pos);
						templength = length % 8 + 8;
					}
					else
					{
						strcpy(current, " ");
						templength = length;
					}
				}
				else
				{
					strcpy(current, initial);
					templength = length;
				}

				V_DrawString(templength, height + 8, V_TRANSLUCENT, current);
			}

			length += (signed)strlen(player_names[i]) * 8 + 16;
		}
}

//
// HU_DrawRankings
//
static void HU_DrawRankings(void)
{
	patch_t *p;
	playersort_t tab[MAXPLAYERS];
	INT32 i, j, scorelines;
	boolean completed[MAXPLAYERS];
	UINT32 whiteplayer;

	// draw the ranking title panel
/*	if (gametype != GT_CTF)
	{
		p = W_CachePatchName("RESULT", PU_CACHE);
		V_DrawScaledPatch((BASEVIDWIDTH - SHORT(p->width))/2, 5, 0, p);
	}*/

	// draw the current gametype in the lower right
	HU_drawGametype();

	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (gametype == GT_CTF)
			p = bflagico;
		else
			p = bmatcico;

		V_DrawSmallScaledPatch(128 - SHORT(p->width)/4, 4, 0, p);
		V_DrawCenteredString(128, 16, 0, va("%u", bluescore));

		if (gametype == GT_CTF)
			p = rflagico;
		else
			p = rmatcico;

		V_DrawSmallScaledPatch(192 - SHORT(p->width)/4, 4, 0, p);
		V_DrawCenteredString(192, 16, 0, va("%u", redscore));
	}

	if (gametype != GT_RACE && gametype != GT_COOP)
	{
		if (cv_timelimit.value && timelimitintics > 0)
		{
			INT32 timeval = (timelimitintics+1-leveltime)/TICRATE;

			if (leveltime <= timelimitintics)
			{
				V_DrawCenteredString(64, 8, 0, "TIME LEFT");
				V_DrawCenteredString(64, 16, 0, va("%u", timeval));
			}

			// overtime
			if ((leveltime > (timelimitintics + TICRATE/2)) && cv_overtime.value)
			{
				V_DrawCenteredString(64, 8, 0, "TIME LEFT");
				V_DrawCenteredString(64, 16, 0, "OVERTIME");
			}
		}

		if (cv_pointlimit.value > 0)
		{
			V_DrawCenteredString(256, 8, 0, "POINT LIMIT");
			V_DrawCenteredString(256, 16, 0, va("%d", cv_pointlimit.value));
		}
	}
	else if (gametype == GT_COOP)
	{
		INT32 totalscore = 0;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
				totalscore += players[i].score;
		}

		V_DrawCenteredString(256, 8, 0, "TOTAL SCORE");
		V_DrawCenteredString(256, 16, 0, va("%u", totalscore));
	}
	else
	{
		if (circuitmap)
		{
			V_DrawCenteredString(64, 8, 0, "NUMBER OF LAPS");
			V_DrawCenteredString(64, 16, 0, va("%d", cv_numlaps.value));
		}
	}

	// When you play, you quickly see your score because your name is displayed in white.
	// When playing back a demo, you quickly see who's the view.
	whiteplayer = demoplayback ? displayplayer : consoleplayer;

	scorelines = 0;
	memset(completed, 0, sizeof (completed));
	memset(tab, 0, sizeof (playersort_t)*MAXPLAYERS);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		tab[i].num = -1;
		tab[i].name = 0;

		if (gametype == GT_RACE && !circuitmap)
			tab[i].count = INT32_MAX;
	}

	for (j = 0; j < MAXPLAYERS; j++)
	{
		if (!playeringame[j] || players[j].spectator)
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator)
			{
				if (gametype == GT_TAG)
				{
					if (players[i].score >= tab[scorelines].count && completed[i] == false)
					{
						tab[scorelines].count = players[i].score;
						tab[scorelines].num = i;
						tab[scorelines].color = players[i].skincolor;
						tab[scorelines].name = player_names[i];
					}
				}
				else if (gametype == GT_RACE)
				{
					if (circuitmap)
					{
						if (players[i].laps+1 >= tab[scorelines].count && completed[i] == false)
						{
							tab[scorelines].count = players[i].laps+1;
							tab[scorelines].num = i;
							tab[scorelines].color = players[i].skincolor;
							tab[scorelines].name = player_names[i];
						}
					}
					else
					{
						if (players[i].realtime <= tab[scorelines].count && completed[i] == false)
						{
							tab[scorelines].count = players[i].realtime;
							tab[scorelines].num = i;
							tab[scorelines].color = players[i].skincolor;
							tab[scorelines].name = player_names[i];
						}
					}
				}
				else
				{
					if (players[i].score >= tab[scorelines].count && completed[i] == false)
					{
						tab[scorelines].count = players[i].score;
						tab[scorelines].num = i;
						tab[scorelines].color = players[i].skincolor;
						tab[scorelines].name = player_names[i];
						tab[scorelines].emeralds = players[i].powers[pw_emeralds];
					}
				}
			}
		}
		completed[tab[scorelines].num] = true;
		scorelines++;
	}

	if (scorelines > 20)
		scorelines = 20; //dont draw past bottom of screen, show the best only

	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
		HU_DrawTeamTabRankings(tab, whiteplayer); //separate function for Spazzo's silly request
	else if (scorelines <= 9)
		HU_DrawTabRankings(40, 32, tab, scorelines, whiteplayer);
	else
		HU_DrawDualTabRankings(32, 32, tab, scorelines, whiteplayer);

	// draw spectators in a ticker across the bottom
	if (!splitscreen && (gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF))
		HU_DrawSpectatorTicker();
}

static void HU_DrawCoopOverlay(void)
{
	if (!netgame && (!modifiedgame || savemoddata))
	{
		char emblemsfound[20];
		INT32 found = 0;
		INT32 i;

		for (i = 0; i < MAXEMBLEMS; i++)
		{
			if (emblemlocations[i].collected)
				found++;
		}

		sprintf(emblemsfound, "- %d/%d", found, numemblems);
		V_DrawString(160, 144, 0, emblemsfound);
		V_DrawScaledPatch(128, 144 - SHORT(emblemicon->height)/4, 0, emblemicon);
	}

	if (emeralds & EMERALD1)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8   , (BASEVIDHEIGHT/3)-32, V_TRANSLUCENT, emeraldpics[0]);
	if (emeralds & EMERALD2)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8+24, (BASEVIDHEIGHT/3)-16, V_TRANSLUCENT, emeraldpics[1]);
	if (emeralds & EMERALD3)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8+24, (BASEVIDHEIGHT/3)+16, V_TRANSLUCENT, emeraldpics[2]);
	if (emeralds & EMERALD4)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8   , (BASEVIDHEIGHT/3)+32, V_TRANSLUCENT, emeraldpics[3]);
	if (emeralds & EMERALD5)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8-24, (BASEVIDHEIGHT/3)+16, V_TRANSLUCENT, emeraldpics[4]);
	if (emeralds & EMERALD6)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8-24, (BASEVIDHEIGHT/3)-16, V_TRANSLUCENT, emeraldpics[5]);
	if (emeralds & EMERALD7)
		V_DrawScaledPatch((BASEVIDWIDTH/2)-8   , (BASEVIDHEIGHT/3)   , V_TRANSLUCENT, emeraldpics[6]);
}


// Interface to CECHO settings for the outside world, avoiding the
// expense (and security problems) of going via the console buffer.
void HU_ClearCEcho(void)
{
	cechotimer = 0;
}

void HU_SetCEchoDuration(INT32 seconds)
{
	cechoduration = seconds * TICRATE;
}

void HU_SetCEchoFlags(INT32 flags)
{
	cechoflags = flags;
}

void HU_DoCEcho(const char *msg)
{
	strncpy(cechotext, msg, sizeof(cechotext));
	strncat(cechotext, "\\", sizeof(cechotext));
	cechotext[sizeof(cechotext) - 1] = '\0';
	cechotimer = cechoduration;
}
