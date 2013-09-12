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
/// \brief SRB2 Network game communication and protocol, all OS independent parts.

#if !defined (UNDER_CE)
#include <time.h>
#endif
#ifdef __GNUC__
#include <unistd.h> //for unlink
#endif

#include "i_net.h"
#include "i_system.h"
#include "i_video.h"
#include "d_net.h"
#include "d_main.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "m_menu.h"
#include "console.h"
#include "d_netfil.h"
#include "byteptr.h"
#include "p_saveg.h"
#include "z_zone.h"
#include "p_local.h"
#include "m_misc.h"
#include "am_map.h"
#include "m_random.h"
#include "mserv.h"
#include "y_inter.h"
#include "r_local.h"
#include "m_argv.h"
#include "p_setup.h"

#ifdef _XBOX
#include "sdl/SRB2XBOX/xboxhelp.h"
#endif

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// server:
//   nettics is the tic for each node
//   firstticstosend is the lowest value of nettics
// client:
//   neededtic is the tic needed by the client for run the game
//   firstticstosend is used to optimize a condition
// normally maketic >= gametic > 0

#define PREDICTIONQUEUE BACKUPTICS
#define PREDICTIONMASK (PREDICTIONQUEUE-1)
#define MAX_REASONLENGTH 30

boolean server = true; // true or false but !server == client
boolean nodownload = false;
static boolean serverrunning = false;
INT32 serverplayer = 0;
char adminpassword[9], motd[254]; // Password And Message of the Day

// server specific vars
UINT8 playernode[MAXPLAYERS];
UINT8 consfailcount[MAXPLAYERS];
UINT8 consfailstatus[MAXPLAYERS];

#ifdef NEWPING
UINT16 pingmeasurecount = 1;
UINT32 realpingtable[MAXPLAYERS]; //the base table of ping where an average will be sent to everyone.
UINT32 playerpingtable[MAXPLAYERS]; //table of player latency values.
#endif
SINT8 nodetoplayer[MAXNETNODES];
SINT8 nodetoplayer2[MAXNETNODES]; // say the numplayer for this node if any (splitscreen)
UINT8 playerpernode[MAXNETNODES]; // used specialy for scplitscreen
boolean nodeingame[MAXNETNODES]; // set false as nodes leave game
static tic_t nettics[MAXNETNODES]; // what tic the client have received
static tic_t supposedtics[MAXNETNODES]; // nettics prevision for smaller packet
static UINT8 nodewaiting[MAXNETNODES];
static tic_t firstticstosend; // min of the nettics
static INT16 consistancy[BACKUPTICS];
static tic_t tictoclear = 0; // optimize d_clearticcmd
static tic_t maketic;

// client specific
static ticcmd_t localcmds;
static ticcmd_t localcmds2;
static boolean cl_packetmissed;
// here it is for the secondary local player (splitscreen)
static UINT8 mynode; // my address pointofview server

static UINT8 localtextcmd[MAXTEXTCMD];
static UINT8 localtextcmd2[MAXTEXTCMD]; // splitscreen
static tic_t neededtic;
SINT8 servernode = 0; // the number of the server node
/// \brief do we accept new players?
/// \todo WORK!
boolean acceptnewnode = true;

// engine
ticcmd_t netcmds[BACKUPTICS][MAXPLAYERS];
static UINT8 textcmds[BACKUPTICS][MAXPLAYERS][MAXTEXTCMD];

static consvar_t cv_showjoinaddress = {"showjoinaddress", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_playdemospeed = {"playdemospeed", "0", 0, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

void D_ResetTiccmds(void)
{
	memset(&localcmds, 0, sizeof(ticcmd_t));
	memset(&localcmds2, 0, sizeof(ticcmd_t));
}

static inline void *G_DcpyTiccmd(void* dest, const ticcmd_t* src, const size_t n)
{
	const size_t d = n / sizeof(ticcmd_t);
	const size_t r = n % sizeof(ticcmd_t);
	UINT8 *ret = dest;

	if (r)
		M_Memcpy(dest, src, n);
	else if (d)
		G_MoveTiccmd(dest, src, d);
	return ret+n;
}

static inline void *G_ScpyTiccmd(ticcmd_t* dest, void* src, const size_t n)
{
	const size_t d = n / sizeof(ticcmd_t);
	const size_t r = n % sizeof(ticcmd_t);
	UINT8 *ret = src;

	if (r)
		M_Memcpy(dest, src, n);
	else if (d)
		G_MoveTiccmd(dest, src, d);
	return ret+n;
}



// some software don't support largest packet
// (original sersetup, not exactely, but the probabylity of sending a packet
// of 512 octet is like 0.1)
UINT16 software_MAXPACKETLENGTH;

tic_t ExpandTics(INT32 low)
{
	INT32 delta;

	delta = low - (maketic & UINT8_MAX);

	if (delta >= -64 && delta <= 64)
		return (maketic & ~UINT8_MAX) + low;
	else if (delta > 64)
		return (maketic & ~UINT8_MAX) - 256 + low;
	else //if (delta < -64)
		return (maketic & ~UINT8_MAX) + 256 + low;
}

// -----------------------------------------------------------------
// Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void (*listnetxcmd[MAXNETXCMD])(UINT8 **p, INT32 playernum);

void RegisterNetXCmd(netxcmd_t id, void (*cmd_f)(UINT8 **p, INT32 playernum))
{
#ifdef PARANOIA
	if (id >= MAXNETXCMD)
		I_Error("command id %d too big", id);
	if (listnetxcmd[id] != 0)
		I_Error("Command id %d already used", id);
#endif
	listnetxcmd[id] = cmd_f;
}

void SendNetXCmd(netxcmd_t id, const void *param, size_t nparam)
{
	if (demoplayback)
		return;

	if (localtextcmd[0]+2+nparam > MAXTEXTCMD)
	{
		// Don't allow stupid users to fill up the command buffer.
		if (cv_debug) // If you're not in debug, it just ain't gonna happen...
			I_Error(text[NOROOMNETBUF], id, localtextcmd[0], nparam);
		else // ...but it will warn you.
			CONS_Printf(text[NOROOMNETBUF], id, localtextcmd[0], nparam);
		return;
	}
	localtextcmd[0]++;
	localtextcmd[localtextcmd[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd[localtextcmd[0]+1], param, nparam);
		localtextcmd[0] = (UINT8)(localtextcmd[0] + (UINT8)nparam);
	}
}

// splitscreen player
void SendNetXCmd2(netxcmd_t id, const void *param, size_t nparam)
{
	if (demoplayback)
		return;

	if (localtextcmd2[0]+2+nparam > MAXTEXTCMD)
	{
		I_Error("No more place in the buffer for netcmd %d\n",id);
		return;
	}
	localtextcmd2[0]++;
	localtextcmd2[localtextcmd2[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd2[localtextcmd2[0]+1], param, nparam);
		localtextcmd2[0] = (UINT8)(localtextcmd2[0] + (UINT8)nparam);
	}
}

UINT8 GetFreeXCmdSize(void)
{
	// -1 for the size and another -1 for the ID.
	return (UINT8)(localtextcmd[0] - 2);
}

static void ExtraDataTicker(void)
{
	INT32 i, tic;
	UINT8 *curpos, *bufferend;

	tic = gametic % BACKUPTICS;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] || i == 0)
		{
			curpos = (UINT8 *)&(textcmds[tic][i]);
			bufferend = &curpos[curpos[0]+1];
			curpos++;
			while (curpos < bufferend)
			{
				if (*curpos < MAXNETXCMD && listnetxcmd[*curpos])
				{
					const UINT8 id = *curpos;
					curpos++;
					DEBFILE(va("executing x_cmd %u ply %u ", id, i));
					(listnetxcmd[id])(&curpos, i);
					DEBFILE("done\n");
				}
				else
				{
					if (server)
					{
						XBOXSTATIC UINT8 buf[3];

						buf[0] = (UINT8)i;
						buf[1] = KICK_MSG_CON_FAIL;
						SendNetXCmd(XD_KICK, &buf, 2);
						DEBFILE(va("player %d kicked [gametic=%u] reason as follows:\n", i, gametic));
					}
					CONS_Printf(text[UNKNOWNNETCMD], curpos - (UINT8 *)&(textcmds[tic][i]), *curpos, textcmds[tic][i][0]);
					return;
				}
			}
		}
}

static void D_Clearticcmd(tic_t tic)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		textcmds[tic%BACKUPTICS][i][0] = 0;
		netcmds[tic%BACKUPTICS][i].angleturn = 0;
	}
	DEBFILE(va("clear tic %5u (%2u)\n", tic, tic%BACKUPTICS));
}

// -----------------------------------------------------------------
// end of extra data function
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// extra data function for lmps
// -----------------------------------------------------------------

// if extradatabit is set, after the ziped tic you find this:
//
//   type   |  description
// ---------+--------------
//   byte   | size of the extradata
//   byte   | the extradata (xd) bits: see XD_...
//            with this byte you know what parameter folow
// if (xd & XDNAMEANDCOLOR)
//   byte   | color
//   char[MAXPLAYERNAME] | name of the player
// endif
// if (xd & XD_WEAPON_PREF)
//   byte   | original weapon switch: boolean, true if use the old
//          | weapon switch methode
//   char[NUMWEAPONS] | the weapon switch priority
//   byte   | autoaim: true if use the old autoaim system
// endif
boolean AddLmpExtradata(UINT8 **demo_point, INT32 playernum)
{
	INT32 tic;

	tic = gametic % BACKUPTICS;
	if (textcmds[tic][playernum][0] == 0)
		return false;

	M_Memcpy(*demo_point, textcmds[tic][playernum], textcmds[tic][playernum][0]+1);
	*demo_point += textcmds[tic][playernum][0]+1;
	return true;
}

void ReadLmpExtraData(UINT8 **demo_pointer, INT32 playernum)
{
	UINT8 nextra;

	if (!demo_pointer)
	{
		textcmds[gametic%BACKUPTICS][playernum][0] = 0;
		return;
	}
	nextra = **demo_pointer;
	M_Memcpy(textcmds[gametic%BACKUPTICS][playernum], *demo_pointer, nextra + 1);
	// increment demo pointer
	*demo_pointer += nextra + 1;
}

// -----------------------------------------------------------------
// end extra data function for lmps
// -----------------------------------------------------------------

static INT16 Consistancy(void);

#ifndef NONET
#define JOININGAME
#endif

typedef enum
{
	cl_searching,
	cl_downloadfiles,
	cl_askjoin,
	cl_waitjoinresponse,
#ifdef JOININGAME
	cl_downloadsavegame,
#endif
	cl_connected,
	cl_aborted
} cl_mode_t;

static void GetPackets(void);

static cl_mode_t cl_mode = cl_searching;

//
// CL_SendJoin
//
// send a special packet for declare how many player in local
// used only in arbitratrenetstart()
static boolean CL_SendJoin(void)
{
	CONS_Printf("%s",text[JOINREQUEST]);
	netbuffer->packettype = PT_CLIENTJOIN;

	netbuffer->u.clientcfg.localplayers = (UINT8)((UINT8)splitscreen + 1);
	netbuffer->u.clientcfg.version = VERSION;
	netbuffer->u.clientcfg.subversion = SUBVERSION;

	return HSendPacket(servernode, true, 0, sizeof (clientconfig_pak));
}

static void SV_SendServerInfo(INT32 node, tic_t servertime)
{
	UINT8 *p;

	netbuffer->packettype = PT_SERVERINFO;
	netbuffer->u.serverinfo.version = VERSION;
	netbuffer->u.serverinfo.subversion = SUBVERSION;
	// return back the time value so client can compute their ping
	netbuffer->u.serverinfo.time = (tic_t)LONG(servertime);
	netbuffer->u.serverinfo.leveltime = (tic_t)LONG(leveltime);

	netbuffer->u.serverinfo.numberofplayer = (UINT8)D_NumPlayers();
	netbuffer->u.serverinfo.maxplayer = (UINT8)cv_maxplayers.value;
	netbuffer->u.serverinfo.gametype = (UINT8)gametype;
	netbuffer->u.serverinfo.modifiedgame = (UINT8)modifiedgame;
	netbuffer->u.serverinfo.cheatsenabled = (UINT8)cv_cheats.value;
	netbuffer->u.serverinfo.isdedicated = (UINT8)dedicated;
	strncpy(netbuffer->u.serverinfo.servername, cv_servername.string,
		MAXSERVERNAME);
	strncpy(netbuffer->u.serverinfo.mapname, G_BuildMapName(gamemap), 7);

	M_Memcpy(netbuffer->u.serverinfo.mapmd5, mapmd5, 16);

	if (strcmp(mapheaderinfo[gamemap-1].lvlttl, ""))
		strncpy(netbuffer->u.serverinfo.maptitle, (char *)mapheaderinfo[gamemap-1].lvlttl, 33);
	else
		strncpy(netbuffer->u.serverinfo.maptitle, "UNKNOWN", 33);

	netbuffer->u.serverinfo.maptitle[32] = '\0';

	if (!mapheaderinfo[gamemap-1].nozone)
		netbuffer->u.serverinfo.iszone = 1;
	else
		netbuffer->u.serverinfo.iszone = 0;

	netbuffer->u.serverinfo.actnum = mapheaderinfo[gamemap-1].actnum;

	p = PutFileNeeded();

	HSendPacket(node, false, 0, p - ((UINT8 *)&netbuffer->u));
}

static void SV_SendPlayerInfo(INT32 node)
{
	UINT8 i;
	netbuffer->packettype = PT_PLAYERINFO;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
		{
			netbuffer->u.playerinfo[i].node = 255; // This slot is empty.
			continue;
		}

		netbuffer->u.playerinfo[i].node = (UINT8)playernode[i];
		strncpy(netbuffer->u.playerinfo[i].name, (const char *)&player_names[i], MAXPLAYERNAME+1);
		netbuffer->u.playerinfo[i].name[MAXPLAYERNAME] = '\0';

		//fetch IP address
		{
			const char *claddress;
			UINT32 numericaddress[4];

			if (playernode[i] == 0)
				netbuffer->u.playerinfo[i].address = 2130706433; //127.0.0.1
			else if (playernode[i] > 0 && I_GetNodeAddress && (claddress = I_GetNodeAddress(playernode[i])) != NULL)
			{
				if (sscanf(claddress, "%d.%d.%d.%d", &numericaddress[0], &numericaddress[1], &numericaddress[2], &numericaddress[3]) < 4)
					goto badaddress;
				netbuffer->u.playerinfo[i].address = ((numericaddress[0]%256) << 24) + ((numericaddress[1]%256) << 16) +
				                                     ((numericaddress[2]%256) <<  8) +  (numericaddress[3]%256);
			}
			else
			{
				badaddress:
				netbuffer->u.playerinfo[i].address = 0;
			}
		}

		if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
		{
			if (!players[i].ctfteam)
				netbuffer->u.playerinfo[i].team = 255;
			else
				netbuffer->u.playerinfo[i].team = (UINT8)players[i].ctfteam;
		}
		else
		{
			if (players[i].spectator)
				netbuffer->u.playerinfo[i].team = 255;
			else
				netbuffer->u.playerinfo[i].team = 0;
		}

		netbuffer->u.playerinfo[i].score = players[i].score;
		netbuffer->u.playerinfo[i].timeinserver = (UINT16)(players[i].jointime / TICRATE);
		netbuffer->u.playerinfo[i].skin = (UINT8)players[i].skin;

		// Extra data
		netbuffer->u.playerinfo[i].data = (UINT8)players[i].skincolor;

		if (players[i].pflags & PF_TAGIT)
			netbuffer->u.playerinfo[i].data |= 32;

		if (players[i].gotflag)
			netbuffer->u.playerinfo[i].data |= 64;

		if (players[i].powers[pw_super])
			netbuffer->u.playerinfo[i].data |= 128;
	}

	HSendPacket(node, false, 0, sizeof(plrinfo) * MAXPLAYERS);
}

static boolean SV_SendServerConfig(INT32 node)
{
	INT32 i;
	UINT8 *p, *op;
	boolean waspacketsent;
	UINT32 playermask = 0;

	netbuffer->packettype = PT_SERVERCFG;
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			playermask |= 1<<i;

	netbuffer->u.servercfg.version = VERSION;
	netbuffer->u.servercfg.subversion = SUBVERSION;

	netbuffer->u.servercfg.serverplayer = (UINT8)serverplayer;
	netbuffer->u.servercfg.totalslotnum = (UINT8)(doomcom->numslots);
	netbuffer->u.servercfg.playerdetected = LONG(playermask);
	netbuffer->u.servercfg.gametic = (tic_t)LONG(gametic);
	netbuffer->u.servercfg.clientnode = (UINT8)node;
	netbuffer->u.servercfg.gamestate = (UINT8)gamestate;
	netbuffer->u.servercfg.gametype = (UINT8)gametype;
	netbuffer->u.servercfg.adminplayer = (SINT8)adminplayer;
	netbuffer->u.servercfg.modifiedgame = (UINT8)modifiedgame;
	op = p = netbuffer->u.servercfg.netcvarstates;
	CV_SaveNetVars(&p);
	{
		const size_t len = sizeof (serverconfig_pak) + (size_t)(p - op);

#ifdef DEBUGFILE
		if (debugfile)
		{
			fprintf(debugfile, "ServerConfig Packet about to be sent, size of packet:%"PRIdS" to node:%d\n",
				len, node);
		}
#endif

		waspacketsent = HSendPacket(node, true, 0, len);
	}

#ifdef DEBUGFILE
	if (debugfile)
	{
		if (waspacketsent)
		{
			fprintf(debugfile, "ServerConfig Packet was sent\n");
		}
		else
		{
			fprintf(debugfile, "ServerConfig Packet could not be sent right now\n");
		}
	}
#endif

	return waspacketsent;
}

#ifdef JOININGAME
#define SAVEGAMESIZE (768*1024)

static void SV_SendSaveGame(INT32 node)
{
	size_t length;
	UINT8 *savebuffer;

	// first save it in a malloced buffer
	save_p = savebuffer = (UINT8 *)malloc(SAVEGAMESIZE);
	if (!save_p)
	{
		CONS_Printf("%s",text[NOSAVEGAMEMEM]);
		return;
	}

	P_SaveNetGame();

	length = save_p - savebuffer;
	if (length > SAVEGAMESIZE)
	{
		free(savebuffer);
		save_p = NULL;
		I_Error("Savegame buffer overrun");
	}

	// then send it!
	SendRam(node, savebuffer, length, SF_RAM, 0);
	//free(savebuffer); //but don't free the data, we will do that later after the real send
	save_p = NULL;
}

/*
#define TMPSAVENAME "badmath.sav"
static consvar_t cv_dumpconsistency = {"dumpconsistency", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

static void SV_SavedGame(void)
{
	size_t length;
	UINT8 *savebuffer;
	XBOXSTATIC char tmpsave[256];

	if (!cv_dumpconsistency.value)
		return;

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);

	// first save it in a malloced buffer
	save_p = savebuffer = (UINT8 *)malloc(SAVEGAMESIZE);
	if (!save_p)
	{
		CONS_Printf("%s",NOSAVEGAMEMEM);
		return;
	}

	P_SaveNetGame();

	length = save_p - savebuffer;
	if (length > SAVEGAMESIZE)
	{
		free(savebuffer);
		save_p = NULL;
		I_Error("Savegame buffer overrun");
	}

	// then save it!
	if (!FIL_WriteFile(tmpsave, savebuffer, length))
		CONS_Printf("Didn't save %s for netgame",tmpsave);

	free(savebuffer);
	save_p = NULL;
}

#undef  TMPSAVENAME
*/
#define TMPSAVENAME "$$$.sav"


static void CL_LoadReceivedSavegame(void)
{
	UINT8 *savebuffer = NULL;
	size_t length;
	XBOXSTATIC char tmpsave[256];

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);

	length = FIL_ReadFile(tmpsave, &savebuffer);

	CONS_Printf(text[LOADSAVEGAME], length);
	if (!length)
	{
		I_Error("Can't read savegame sent");
		return;
	}

	save_p = savebuffer;

	paused = false;
	demoplayback = false;
	automapactive = false;

	// load a base level
	playerdeadview = false;

	if (P_LoadNetGame())
	{
		const INT32 actnum = mapheaderinfo[gamemap-1].actnum;
		CONS_Printf(text[MAPISNOW], G_BuildMapName(gamemap));
		if (strcmp(mapheaderinfo[gamemap-1].lvlttl, ""))
		{
			CONS_Printf(": %s", mapheaderinfo[gamemap-1].lvlttl);
			if (!mapheaderinfo[gamemap-1].nozone)
				CONS_Printf(" %s",text[ZONE]);
			if (actnum > 0)
				CONS_Printf(" %2d", actnum);
		}
		CONS_Printf("\"\n");
	}
	else
	{
		CONS_Printf("%s",text[CANNOTLOADLEVEL]);
		Z_Free(savebuffer);
		save_p = NULL;
		if (unlink(tmpsave) == -1)
			CONS_Printf(text[CANNOTDELETE], tmpsave);
		return;
	}

	// done
	Z_Free(savebuffer);
	save_p = NULL;
	if (unlink(tmpsave) == -1)
		CONS_Printf(text[CANNOTDELETE], tmpsave);
	consistancy[gametic%BACKUPTICS] = Consistancy();
	CON_ToggleOff();
}
#endif

static void SendAskInfo(INT32 node, boolean viams)
{
	const tic_t asktime = I_GetTime();
	netbuffer->packettype = PT_ASKINFO;
	netbuffer->u.askinfo.version = VERSION;
	netbuffer->u.askinfo.time = (tic_t)LONG(asktime);

	// Even if this never arrives due to the host being firewalled, we've
	// now allowed traffic from the host to us in, so once the MS relays
	// our address to the host, it'll be able to speak to us.
	HSendPacket(node, false, 0, sizeof (askinfo_pak));

	// Also speak to the MS.
	if (viams && node != 0 && node != BROADCASTADDR)
		SendAskInfoViaMS(node, asktime);
}

serverelem_t serverlist[MAXSERVERLIST];
UINT32 serverlistcount = 0;

static void SL_ClearServerList(INT32 connectedserver)
{
	UINT32 i;

	for (i = 0; i < serverlistcount; i++)
		if (connectedserver != serverlist[i].node)
		{
			Net_CloseConnection(serverlist[i].node);
			serverlist[i].node = 0;
		}
	serverlistcount = 0;
}

static UINT32 SL_SearchServer(INT32 node)
{
	UINT32 i;
	for (i = 0; i < serverlistcount; i++)
		if (serverlist[i].node == node)
			return i;

	return UINT32_MAX;
}

static void SL_InsertServer(serverinfo_pak* info, SINT8 node)
{
	UINT32 i;
	boolean moved;

	// search if not already on it
	i = SL_SearchServer(node);
	if (i == UINT32_MAX)
	{
		// not found add it
		if (serverlistcount >= MAXSERVERLIST)
			return; // list full

		if (info->version != VERSION)
			return; // Not same version.

		if (info->subversion != SUBVERSION)
			return; // Close, but no cigar.

		i = serverlistcount++;
	}

	serverlist[i].info = *info;
	serverlist[i].node = node;

	// list is sorted, so move the entry until it is sorted
	do
	{
		INT32 keycurr = 0, keyprev = 0, keynext = 0;
		switch(cv_serversort.value)
		{
		case 0:		// Ping.
			keycurr = (tic_t)LONG(serverlist[i].info.time);
			if (i > 0) keyprev = (tic_t)LONG(serverlist[i-1].info.time);
			if (i < serverlistcount - 1) keynext = (tic_t)LONG(serverlist[i+1].info.time);
			break;
		case 1:		// Players.
			keycurr = serverlist[i].info.numberofplayer;
			if (i > 0) keyprev = serverlist[i-1].info.numberofplayer;
			if (i < serverlistcount - 1) keynext = serverlist[i+1].info.numberofplayer;
			break;
		case 2:		// Gametype.
			keycurr = serverlist[i].info.gametype;
			if (i > 0) keyprev = serverlist[i-1].info.gametype;
			if (i < serverlistcount - 1) keynext = serverlist[i+1].info.gametype;
			break;
		}

		moved = false;
		if (i > 0 && keycurr < keyprev)
		{
			serverelem_t s;
			s = serverlist[i];
			serverlist[i] = serverlist[i-1];
			serverlist[i-1] = s;
			i--;
			moved = true;
		}
		else if (i < serverlistcount - 1 && keycurr > keynext)
		{
			serverelem_t s;
			s = serverlist[i];
			serverlist[i] = serverlist[i+1];
			serverlist[i+1] = s;
			i++;
			moved = true;
		}
	} while (moved);
}

void CL_UpdateServerList(boolean internetsearch, INT32 room)
{
	SL_ClearServerList(0);

	if (!netgame && I_NetOpenSocket)
	{
		MSCloseUDPSocket();		// Tidy up before wiping the slate.
		if (I_NetOpenSocket())
		{
			netgame = true;
			multiplayer = true;
		}
	}

	// search for local servers
	if (netgame)
		SendAskInfo(BROADCASTADDR, false);

	if (internetsearch)
	{
		const msg_server_t *server_list;
		INT32 i = -1;
		server_list = GetShortServersList(room);
		if (server_list)
		{
			char version[8] = "";
			snprintf(version, sizeof (version), "%d.%d.%d", VERSION/100, VERSION%100, SUBVERSION);
			version[sizeof (version) - 1] = '\0';

			for (i = 0; server_list[i].header.buffer[0]; i++)
			{
				// Make sure MS version matches our own, to
				// thwart nefarious servers who lie to the MS.

				if(strcmp(version, server_list[i].version) == 0)
				{
					INT32 node;
					XBOXSTATIC char addr_str[24];

					// insert ip (and optionally port) in node list
					sprintf(addr_str, "%s:%s", server_list[i].ip, server_list[i].port);
					node = I_NetMakeNode(addr_str);
					if (node == -1)
						break; // no more node free
					SendAskInfo(node, true);
				}
			}
		}

		//no server list?(-1) or no servers?(0)
		if (!i)
		{
			; /// TODO: display error or warning?
		}
	}
}

// use adaptive send using net_bandwidth and stat.sendbytes
static void CL_ConnectToServer(boolean viams)
{
	INT32 pnumnodes, nodewaited = doomcom->numnodes, i;
	boolean waitmore;
	tic_t asksent, oldtic;
#ifdef JOININGAME
	XBOXSTATIC char tmpsave[256];

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);
#endif

	cl_mode = cl_searching;

#ifdef JOININGAME
	// don't get a corrupt savegame error because tmpsave already exists
	if (FIL_WriteFileOK(tmpsave) && unlink(tmpsave) == -1)
		I_Error("Can't delete %s", tmpsave);
#endif

	CONS_Printf("%s",text[ESCABORT]);
	if (servernode < 0 || servernode >= MAXNETNODES)
		CONS_Printf("%s",text[SEARCHSERV]);
	else
		CONS_Printf("%s",text[CONTACTSERV]);

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission(); // clean up intermission graphics etc

	DEBFILE(va("waiting %d nodes\n", doomcom->numnodes));
	G_SetGamestate(GS_WAITINGPLAYERS);
	wipegamestate = GS_WAITINGPLAYERS;

	adminplayer = -1;
	pnumnodes = 1;
	oldtic = I_GetTime() - 1;
	asksent = (tic_t)-TICRATE;
	i = SL_SearchServer(servernode);
	if (i != -1)
	{
		INT32 j;
		const char *gametypestr = NULL;
		CONS_Printf(text[CONNECTINGTO],serverlist[i].info.servername);
		for (j = 0; gametype_cons_t[j].strvalue; j++)
		{
			if (gametype_cons_t[j].value == serverlist[i].info.gametype)
			{
				gametypestr = gametype_cons_t[j].strvalue;
				break;
			}
		}
		if (gametypestr)
			CONS_Printf(text[NETGAMETYPE], gametypestr);
		CONS_Printf(text[NETVERSION], serverlist[i].info.version/100,
		 serverlist[i].info.version%100, serverlist[i].info.subversion);
	}
	SL_ClearServerList(servernode);
	do
	{
		switch (cl_mode)
		{
			case cl_searching:
				// serverlist is updated by GetPacket function
				if (serverlistcount > 0)
				{
					// this can be a responce to our broadcast request
					if (servernode == -1 || servernode >= MAXNETNODES)
					{
						i = 0;
						servernode = serverlist[i].node;
						CONS_Printf("%s",text[FOUND]);
					}
					else
					{
						i = SL_SearchServer(servernode);
						if (i < 0)
							break; // the case
					}

					// Quit here rather than downloading files and being refused later.
					if (serverlist[i].info.numberofplayer >= serverlist[i].info.maxplayer)
					{
						M_StartMessage(va("Maximum players reached: %d\nPress ESC", serverlist[i].info.maxplayer), NULL, MM_NOTHING);
						D_QuitNetGame();
						CL_Reset();
						D_StartTitle();
						return;
					}

					if (!server)
					{
						D_ParseFileneeded(serverlist[i].info.fileneedednum,
							serverlist[i].info.fileneeded);
						CONS_Printf("%s",text[CHECKINGFILES]);
						i = CL_CheckFiles();
						if (i == 2) // cannot join for some reason
						{
							D_QuitNetGame();
							CL_Reset();
							D_StartTitle();
							return;
						}
						else if (i == 1)
							cl_mode = cl_askjoin;
						else
						{ // must download something
							// no problem if can't send packet, we will retry later
							if (SendRequestFile())
								cl_mode = cl_downloadfiles;
						}
					}
					else
						cl_mode = cl_askjoin; // files need not be checked for the server.
					break;
				}
				// ask the info to the server (askinfo packet)
				if (asksent + TICRATE < I_GetTime())
				{
					SendAskInfo(servernode, viams);
					asksent = I_GetTime();
				}
				break;
			case cl_downloadfiles:
				waitmore = false;
				for (i = 0; i < fileneedednum; i++)
					if (fileneeded[i].status == FS_DOWNLOADING
						|| fileneeded[i].status == FS_REQUESTED)
					{
						waitmore = true;
						break;
					}
				if (waitmore)
					break; // exit the case

				cl_mode = cl_askjoin; // don't break case continue to cljoin request now
			case cl_askjoin:
				CL_LoadServerFiles();
#ifdef JOININGAME
				// prepare structures to save the file
				// WARNING: this can be useless in case of server not in GS_LEVEL
				// but since the network layer doesn't provide ordered packets...
				CL_PrepareDownloadSaveGame(tmpsave);
#endif
				if (CL_SendJoin())
					cl_mode = cl_waitjoinresponse;
				break;
#ifdef JOININGAME
			case cl_downloadsavegame:
				if (fileneeded[0].status == FS_FOUND)
				{
					// Gamestate is now handled within CL_LoadReceivedSavegame()
					CL_LoadReceivedSavegame();
					cl_mode = cl_connected;
				} // don't break case continue to cl_connected
				else
					break;
#endif
			case cl_waitjoinresponse:
			case cl_connected:
			default:
				break;

			// Connection closed by cancel, timeout or refusal.
			case cl_aborted:
				cl_mode = cl_searching;
				return;
		}

		GetPackets();
		Net_AckTicker();

		// call it only one by tic
		if (oldtic != I_GetTime())
		{
			INT32 key;

			I_OsPolling();
			key = I_GetKey();
			if (key == KEY_ESCAPE)
			{
				CONS_Printf("%s",text[NETSYNCABORT]);
//				M_StartMessage("Network game synchronization aborted.\n\nPress ESC\n", NULL, MM_NOTHING);
				D_QuitNetGame();
				CL_Reset();
				D_StartTitle();
				return;
			}
			if (key == 's' && server)
				doomcom->numnodes = (INT16)pnumnodes;

			FiletxTicker();
			oldtic = I_GetTime();

			CON_Drawer();
			I_FinishUpdate(); // page flip or blit buffer
		}
		else I_Sleep();

		if (server)
		{
			pnumnodes = 0;
			for (i = 0; i < MAXNETNODES; i++)
				if (nodeingame[i]) pnumnodes++;
		}
	}
	while (!(cl_mode == cl_connected && (!server || (server && nodewaited <= pnumnodes))));

	DEBFILE(va("Synchronisation Finished\n"));

	if (cv_cheats.value)
	{
		if (!server)
			HU_DoCEcho(va("%s", text[CHEATS_ACTIVATED]));
		I_OutputMsg("%s", text[CHEATS_ACTIVATED]);
	}

	displayplayer = consoleplayer;
}

#ifndef NONET
typedef struct banreason_s
{
	char *reason;
	struct banreason_s *prev; //-1
	struct banreason_s *next; //+1
} banreason_t;

static banreason_t *reasontail = NULL; //last entry, use prev
static banreason_t *reasonhead = NULL; //1st entry, use next

static void Command_ShowBan(void) //Print out ban list
{
	size_t i;
	const char *address, *mask;
	banreason_t *reasonlist = reasonhead;

	if (I_GetBanAddress)
		CONS_Printf("%s", text[BANLIST]);
	else
		return;

	for (i = 0;(address = I_GetBanAddress(i)) != NULL;i++)
	{
		if (!I_GetBanMask || (mask = I_GetBanMask(i)) == NULL)
			CONS_Printf(text[BANLIST2], i+1, address);
		else
			CONS_Printf(text[BANLIST3], i+1, address, mask);

		if (reasonlist && reasonlist->reason)
			CONS_Printf(text[BANREASON], reasonlist->reason);

		if (reasonlist) reasonlist = reasonlist->next;
	}

	if (i == 0 && !address)
		CONS_Printf("%s", text[BANNONE]);
}

void D_SaveBan(void)
{
	FILE *f;
	size_t i;
	banreason_t *reasonlist = reasonhead;
	const char *address, *mask;

	if (!reasonhead)
		return;

	f = fopen(va("%s"PATHSEP"%s", srb2home, "ban.txt"), "w");

	if (!f)
	{
		CONS_Printf("%s",text[CANNOTSAVEBANLIST]);
		return;
	}

	for (i = 0;(address = I_GetBanAddress(i)) != NULL;i++)
	{
		if (!I_GetBanMask || (mask = I_GetBanMask(i)) == NULL)
			fprintf(f, "%s 0", address);
		else
			fprintf(f, "%s %s", address, mask);

		if (reasonlist && reasonlist->reason)
			fprintf(f, " %s\n", reasonlist->reason);
		else
			fprintf(f, " %s\n", "NA");

		if (reasonlist) reasonlist = reasonlist->next;
	}

	fclose(f);
}

static void Ban_Add(const char *reason)
{
	banreason_t *reasonlist = malloc(sizeof(*reasonlist));

	if (!reasonlist)
		return;
	if (!reason)
		reason = "NA";

	reasonlist->next = NULL;
	reasonlist->reason = Z_StrDup(reason);
	if ((reasonlist->prev = reasontail) == NULL)
		reasonhead = reasonlist;
	else
		reasontail->next = reasonlist;
	reasontail = reasonlist;
}

static void Command_ClearBans(void)
{
	banreason_t *temp;

	if (!I_ClearBans)
		return;

	I_ClearBans();
	reasontail = NULL;
	while (reasonhead)
	{
		temp = reasonhead->next;
		Z_Free(reasonhead->reason);
		free(reasonhead);
		reasonhead = temp;
	}
}

static void Ban_Load_File(boolean warning)
{
	FILE *f;
	size_t i;
	const char *address, *mask;
	char buffer[MAX_WADPATH];

	f = fopen(va("%s"PATHSEP"%s", srb2home, "ban.txt"), "r");

	if (!f)
	{
		if (warning)
			CONS_Printf("%s", text[CANNOTLOADBANLIST]);
		return;
	}

	if (I_ClearBans)
		Command_ClearBans();
	else
		return;

	for (i=0; fgets(buffer, (int)sizeof(buffer), f); i++)
	{
		address = strtok(buffer, " \t\r\n");
		mask = strtok(NULL, " \t\r\n");

		I_SetBanAddress(address, mask);

		Ban_Add(strtok(NULL, "\r\n"));
	}

	fclose(f);
}

static void Command_ReloadBan(void)  //recheck ban.txt
{
	Ban_Load_File(true);
}

static void Command_connect(void)
{
	// Assume we connect directly.
	boolean viams = false;

	if (COM_Argc() < 2)
	{
		CONS_Printf("%s", text[CONNECTHELP]);
		return;
	}

	if (Playing())
	{
		CONS_Printf("%s", text[NOCONNECTINGAME]);
		return;
	}

	if (modifiedgame)
	{
		M_StartMessage("You have wad files loaded and/or\nmodified the game in some way.\nPlease restart SRB2 before\nconnecting.", NULL, MM_NOTHING);
		return;
	}

	server = false;

	if (!stricmp(COM_Argv(1), "self"))
	{
		servernode = 0;
		server = true;
		/// \bug should be but...
		//SV_SpawnServer();
	}
	else
	{
		// used in menu to connect to a server in the list
		if (netgame && !stricmp(COM_Argv(1), "node"))
		{
			servernode = (SINT8)atoi(COM_Argv(2));

			// Use MS to traverse NAT firewalls.
			viams = true;
		}
		else if (netgame)
		{
			CONS_Printf("%s", text[NOCONNECTINGAME]);
			return;
		}
		else if (I_NetOpenSocket)
		{
			MSCloseUDPSocket();		// Tidy up before wiping the slate.
			I_NetOpenSocket();
			netgame = true;
			multiplayer = true;

			if (!stricmp(COM_Argv(1), "any"))
				servernode = BROADCASTADDR;
			else if (I_NetMakeNode)
				servernode = I_NetMakeNode(COM_Argv(1));
			else
			{
				CONS_Printf("%s", text[NOSERVERIDENTIFY]);
				D_CloseConnection();
				return;
			}
		}
		else
			CONS_Printf("There is no network driver\n");
	}

	splitscreen = false;
	SplitScreen_OnChange();
	CL_ConnectToServer(viams);
}
#endif

static void ResetNode(INT32 node);

//
// CL_ClearPlayer
//
// Clears the player data so that a future client can use this slot
//
void CL_ClearPlayer(INT32 playernum)
{
	if (players[playernum].mo)
	{
		// Don't leave a NiGHTS ghost!
		if ((players[playernum].pflags & PF_NIGHTSMODE) && players[playernum].mo->tracer)
			P_SetMobjState(players[playernum].mo->tracer, S_DISS);

		players[playernum].mo->player = NULL;
		P_UnsetThingPosition(players[playernum].mo);
		if (sector_list)
		{
			P_DelSeclist(sector_list);
			sector_list = NULL;
		}
		players[playernum].mo->flags = 0;
		P_SetThingPosition(players[playernum].mo);
		players[playernum].mo->health = 0;
		P_RemoveMobj(players[playernum].mo);
		players[playernum].mo->subsector = NULL; // Just in case...
	}
	players[playernum].mo = NULL;

	memset(&players[playernum], 0, sizeof (player_t));
}

//
// CL_RemovePlayer
//
// Removes a player from the current game
//
static void CL_RemovePlayer(INT32 playernum)
{
	// Sanity check: exceptional cases (i.e. c-fails) can cause multiple
	// kick commands to be issued for the same player.
	if (!playeringame[playernum])
		return;

	if (server && !demoplayback)
	{
		INT32 node = playernode[playernum];
		playerpernode[node]--;
		if (playerpernode[node] <= 0)
		{
			nodeingame[playernode[playernum]] = false;
			Net_CloseConnection(playernode[playernum]);
			ResetNode(node);
		}
	}

	if (gametype == GT_CTF)
		P_PlayerFlagBurst(&players[playernum], false); // Don't take the flag with you!

	// If in a special stage, redistribute the player's rings across
	// the remaining players.
	if (G_IsSpecialStage(gamemap))
	{
		INT32 i, count, increment, rings;

		for (i = 0, count = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
				count++;
		}

		count--;
		rings = players[playernum].health - 1;
		increment = rings/count;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && i != playernum)
			{
				if (rings < increment)
					P_GivePlayerRings(&players[i], rings, false);
				else
					P_GivePlayerRings(&players[i], increment, false);

				rings -= increment;
			}
		}
	}

	// Reset player data
	CL_ClearPlayer(playernum);

	// remove avatar of player
	playeringame[playernum] = false;
	playernode[playernum] = UINT8_MAX;
	while (!playeringame[doomcom->numslots-1] && doomcom->numslots > 1)
		doomcom->numslots--;

	// Reset the name
	sprintf(player_names[playernum], "Player %d", playernum+1);

	if (playernum == adminplayer)
		adminplayer = -1; // don't stay admin after you're gone

	if (playernum == displayplayer)
		displayplayer = consoleplayer; // don't look through someone's view who isn't there

	consfailcount[playernum] = 0;
	consfailstatus[playernum] = 0;

	if (gametype == GT_TAG)//Check if you still have a game. Location flexible. =P
		P_CheckSurvivors();
	else if (gametype == GT_RACE)
		P_CheckRacers();
}

void CL_Reset(void)
{
	if (demorecording)
		G_CheckDemoStatus();

	// reset client/server code
	DEBFILE(va("\n-=-=-=-=-=-=-= Client reset =-=-=-=-=-=-=-\n\n"));

	if (servernode > 0 && servernode < MAXNETNODES)
	{
		nodeingame[(UINT8)servernode] = false;
		Net_CloseConnection(servernode);
	}
	D_CloseConnection(); // netgame = false
	multiplayer = false;
	servernode = 0;
	server = true;
	doomcom->numnodes = 1;
	doomcom->numslots = 1;
	SV_StopServer();
	SV_ResetServer();

	// D_StartTitle should get done now, but the calling function will handle it
}

#ifndef NONET
static void Command_GetPlayerNum(void)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			if (serverplayer == i)
				CONS_Printf(text[SERVERPLAYERNUM], i, playernode[i], player_names[i]);
			else
				CONS_Printf(text[PLAYERNUM], i, playernode[i], player_names[i]);
		}
}

SINT8 nametonum(const char *name)
{
	INT32 playernum, i;

	if (!strcmp(name, "0"))
		return 0;

	playernum = (SINT8)atoi(name);

	if (playernum < 0 || playernum >= MAXPLAYERS)
		return -1;

	if (playernum)
	{
		if (playeringame[playernum])
			return (SINT8)playernum;
		else
			return -1;
	}

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && !stricmp(player_names[i], name))
			return (SINT8)i;

	CONS_Printf(text[NOPLAYERNAMED], name);

	return -1;
}

/** Lists all players and their player numbers.
  *
  * \sa Command_GetPlayerNum
  */
static void Command_Nodes(void)
{
	INT32 i;
	size_t maxlen = 0;
	const char *address;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		const size_t plen = strlen(player_names[i]);
		if (playeringame[i] && plen > maxlen)
			maxlen = plen;
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			CONS_Printf(text[NODESCMDTXT], i, maxlen, player_names[i]);
			CONS_Printf(text[NODESCMDTXT2], playernode[i]);
			if (I_GetNodeAddress && (address = I_GetNodeAddress(playernode[i])) != NULL)
				CONS_Printf(text[NODESCMDTXT3], address);

			if (i == adminplayer)
				CONS_Printf("%s", text[NODESCMDADMIN]);

			if (players[i].spectator)
				CONS_Printf("%s", text[NODESCMDSPEC]);

			CONS_Printf("\n");
		}
	}
}

static void Command_Ban(void)
{
	if (COM_Argc() == 1)
	{
		CONS_Printf("%s", text[BANHELP]);
		return;
	}

	if (server || adminplayer == consoleplayer)
	{
		XBOXSTATIC UINT8 buf[3 + MAX_REASONLENGTH];
		UINT8 *p = buf;
		const SINT8 pn = nametonum(COM_Argv(1));
		const INT32 node = playernode[(INT32)pn];

		if (pn == -1 || pn == 0)
			return;
		else
			WRITEUINT8(p, pn);
		if (I_Ban && !I_Ban(node))
		{
			CONS_Printf("%s", text[TOOMANYBANS]);
			WRITEUINT8(p, KICK_MSG_GO_AWAY);
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		else
		{
			Ban_Add(COM_Argv(2));

			if (COM_Argc() == 2)
			{
				WRITEUINT8(p, KICK_MSG_BANNED);
				SendNetXCmd(XD_KICK, &buf, 2);
			}
			else
			{
				size_t i, j = COM_Argc();
				char message[MAX_REASONLENGTH];

				//Steal from the motd code so you don't have to put the reason in quotes.
				strlcpy(message, COM_Argv(2), sizeof message);
				for (i = 3; i < j; i++)
				{
					strlcat(message, " ", sizeof message);
					strlcat(message, COM_Argv(i), sizeof message);
				}

				WRITEUINT8(p, KICK_MSG_CUSTOM_BAN);
				WRITESTRINGN(p, message, MAX_REASONLENGTH);
				SendNetXCmd(XD_KICK, &buf, p - buf);
			}
		}
	}
	else
		CONS_Printf("%s", text[YOUARENOTTHESERVER]);

}

static void Command_Kick(void)
{
	XBOXSTATIC UINT8 buf[3 + MAX_REASONLENGTH];
	UINT8 *p = buf;

	if (COM_Argc() == 1)
	{
		CONS_Printf("%s", text[KICKHELP]);
		return;
	}

	if (server || adminplayer == consoleplayer)
	{
		const SINT8 pn = nametonum(COM_Argv(1));
		WRITESINT8(p, pn);
		if (pn == -1 || pn == 0)
			return;
		if (COM_Argc() == 2)
		{
			WRITEUINT8(p, KICK_MSG_GO_AWAY);
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		else
		{
			size_t i, j = COM_Argc();
			char message[MAX_REASONLENGTH];

			//Steal from the motd code so you don't have to put the reason in quotes.
			strlcpy(message, COM_Argv(2), sizeof message);
			for (i = 3; i < j; i++)
			{
				strlcat(message, " ", sizeof message);
				strlcat(message, COM_Argv(i), sizeof message);
			}

			WRITEUINT8(p, KICK_MSG_CUSTOM_KICK);
			WRITESTRINGN(p, message, MAX_REASONLENGTH);
			SendNetXCmd(XD_KICK, &buf, p - buf);
		}
	}
	else
		CONS_Printf("%s", text[YOUARENOTTHESERVER]);
}
#endif

static void Got_KickCmd(UINT8 **p, INT32 playernum)
{
	INT32 pnum, msg;
	XBOXSTATIC char buf[3 + MAX_REASONLENGTH];
	char *reason = buf;

	pnum = READUINT8(*p);
	msg = READUINT8(*p);

	if (pnum == serverplayer && playernum == adminplayer)
	{
		CONS_Printf("%s", text[SERVERREMOTESHUTDOWN]);

		if (server)
			COM_BufAddText("quit\n");

		return;
	}

	// Is playernum authorized to make this kick?
	if (playernum != serverplayer && playernum != adminplayer
		&& !(playerpernode[playernode[playernum]] == 2
		&& nodetoplayer2[playernode[playernum]] == pnum))
	{
		// We received a kick command from someone who isn't the
		// server or admin, and who isn't in splitscreen removing
		// player 2. Thus, it must be someone with a modified
		// binary, trying to kick someone but without having
		// authorization.

		// We deal with this by changing the kick reason to
		// "consistency failure" and kicking the offending user
		// instead.

		// Note: Splitscreen in netgames is broken because of
		// this. Only the server has any idea of which players
		// are using splitscreen on the same computer, so
		// clients cannot always determine if a kick is
		// legitimate.

		CONS_Printf(text[ILLEGALKICKCMD], player_names[playernum], pnum);
		// In debug, print a longer message with more details.
#ifndef _DEBUG
		if (cv_debug || devparm)
#endif
		{
			CONS_Printf("So, you must be asking, why is this an illegal kick?\n"
			            "Well, let's take a look at the facts, shall we?\n"
			            "\n"
			            "playernum (this is the guy who did it), he's %d.\n"
			            "pnum (the guy he's trying to kick) is %d.\n"
			            "playernum's node is %d.\n"
			            "That node has %d players.\n"
			            "Player 2 on that node is %d.\n"
			            "pnum's node is %d.\n"
			            "That node has %d players.\n"
			            "Player 2 on that node is %d.\n"
			            "\n"
			            "If you think this is a bug, please report it, including all of the details above.\n",
				playernum, pnum,
				playernode[playernum], playerpernode[playernode[playernum]],
				nodetoplayer2[playernode[playernum]],
				playernode[pnum], playerpernode[playernode[pnum]],
				nodetoplayer2[playernode[pnum]]);
		};
		pnum = playernum;
		msg = KICK_MSG_CON_FAIL;
	}

	CONS_Printf(text[KICKEDPLAYERNAME], player_names[pnum]);

	// If a verified admin banned someone, the server needs to know about it.
	// If the playernum isn't zero (the server) then the server needs to record the ban.
	if (server && playernum && msg == KICK_MSG_BANNED)
	{
		if (I_Ban && !I_Ban(playernode[(INT32)pnum]))
		{
			CONS_Printf("%s", text[TOOMANYBANS]);
		}
	}

	switch (msg)
	{
		case KICK_MSG_GO_AWAY:
			CONS_Printf("%s", text[KICKEDGOAWAY]);
			break;
#ifdef NEWPING
		case KICK_MSG_PING_HIGH:
			CONS_Printf("%s", text[KICKEDPINGTOOHIGH]);
			break;
#endif
		case KICK_MSG_CON_FAIL:
			CONS_Printf("%s", text[KICKEDCONSFAIL]);

			if (M_CheckParm("-consisdump")) // Helps debugging some problems
			{
				INT32 i;

				CONS_Printf(text[CONSIS_DUMP1], pnum);

				for (i = 0; i < MAXPLAYERS; i++)
				{
					CONS_Printf("-------------------------------------\n");
					if (players[i].mo)
						CONS_Printf(text[CONSIS_DUMP2], i, players[i].mo->x, players[i].mo->y, players[i].mo->z);
					else
						CONS_Printf(text[CONSIS_DUMP3], i);
					CONS_Printf("-------------------------------------\n");
				}
			}
			break;
		case KICK_MSG_TIMEOUT:
			CONS_Printf("%s", text[LEFTGAME_TIMEOUT]);
			break;
		case KICK_MSG_PLAYER_QUIT:
			CONS_Printf("%s", text[LEFTGAME]);
			break;
		case KICK_MSG_BANNED:
			CONS_Printf("%s", text[KICKEDBANNED]);
			break;
		case KICK_MSG_CUSTOM_KICK:
			READSTRINGN(*p, reason, MAX_REASONLENGTH+1);
			CONS_Printf(text[CUSTOMKICKMSG], reason);
			break;
		case KICK_MSG_CUSTOM_BAN:
			READSTRINGN(*p, reason, MAX_REASONLENGTH+1);
			CONS_Printf(text[CUSTOMBANMSG], reason);
			break;
	}

	if (pnum == consoleplayer)
	{
//		if (msg == KICK_MSG_CON_FAIL) SV_SavedGame();
		D_QuitNetGame();
		CL_Reset();
		D_StartTitle();
		if (msg == KICK_MSG_CON_FAIL)
		{
			M_StartMessage("You have been kicked\n(consistency failure)\nPress ESC\n", NULL,
				MM_NOTHING);
		}
#ifdef NEWPING
		else if (msg == KICK_MSG_PING_HIGH)
			M_StartMessage("You have been kicked\n(Broke ping limit)\nPress ESC\n", NULL, MM_NOTHING);
#endif
		else if (msg == KICK_MSG_BANNED)
			M_StartMessage("You have been banned by the server\n\nPress ESC\n", NULL, MM_NOTHING);
		else if (msg == KICK_MSG_CUSTOM_KICK)
			M_StartMessage(va("You have been kicked\n(%s)\nPress ESC\n", reason), NULL, MM_NOTHING);
		else if (msg == KICK_MSG_CUSTOM_BAN)
			M_StartMessage(va("You have been banned\n(%s)\nPress ESC\n", reason), NULL, MM_NOTHING);
		else
			M_StartMessage("You have been kicked by the server\n\nPress ESC\n", NULL, MM_NOTHING);
	}
	else
		CL_RemovePlayer(pnum);
}

consvar_t cv_allownewplayer = {"allowjoin", "On", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL	};
consvar_t cv_joinnextround = {"joinnextround", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL}; /// \todo not done
static CV_PossibleValue_t maxplayers_cons_t[] = {{2, "MIN"}, {32, "MAX"}, {0, NULL}};
consvar_t cv_maxplayers = {"maxplayers", "8", CV_SAVE, maxplayers_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static CV_PossibleValue_t consfailprotect_cons_t[] = {{0, "MIN"}, {20, "MAX"}, {0, NULL}};
consvar_t cv_consfailprotect = {"consfailprotect", "10", 0, consfailprotect_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL	};
consvar_t cv_blamecfail = {"blamecfail", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL	};

// max file size to send to a player (in kilobytes)
static CV_PossibleValue_t maxsend_cons_t[] = {{0, "MIN"}, {51200, "MAX"}, {0, NULL}};
consvar_t cv_maxsend = {"maxsend", "1024", CV_SAVE, maxsend_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static void Got_AddPlayer(UINT8 **p, INT32 playernum);

// called one time at init
void D_ClientServerInit(void)
{
	DEBFILE(va("- - -== SRB2 v%d.%.2d.%d"VERSIONSTRING" debugfile ==- - -\n",
		VERSION/100, VERSION%100, SUBVERSION));

#ifndef NONET
	COM_AddCommand("getplayernum", Command_GetPlayerNum);
	COM_AddCommand("kick", Command_Kick);
	COM_AddCommand("ban", Command_Ban);
	COM_AddCommand("clearbans", Command_ClearBans);
	COM_AddCommand("showbanlist", Command_ShowBan);
	COM_AddCommand("reloadbans", Command_ReloadBan);
	COM_AddCommand("connect", Command_connect);
	COM_AddCommand("nodes", Command_Nodes);
#endif

	RegisterNetXCmd(XD_KICK, Got_KickCmd);
	RegisterNetXCmd(XD_ADDPLAYER, Got_AddPlayer);
#ifndef NONET
	CV_RegisterVar(&cv_allownewplayer);
	CV_RegisterVar(&cv_joinnextround);
	CV_RegisterVar(&cv_showjoinaddress);
	CV_RegisterVar(&cv_consfailprotect);
	CV_RegisterVar(&cv_blamecfail);
//	CV_RegisterVar(&cv_dumpconsistency);
	Ban_Load_File(false);
#endif

	gametic = 0;
	localgametic = 0;

	// do not send anything before the real begin
	SV_StopServer();
	SV_ResetServer();
	if (dedicated)
		SV_SpawnServer();
}

static void ResetNode(INT32 node)
{
	nodeingame[node] = false;
	nodetoplayer[node] = -1;
	nodetoplayer2[node] = -1;
	nettics[node] = gametic;
	supposedtics[node] = gametic;
	nodewaiting[node] = 0;
	playerpernode[node] = 0;
}

void SV_ResetServer(void)
{
	INT32 i;

	// +1 because this command will be executed in com_executebuffer in
	// tryruntic so gametic will be incremented, anyway maketic > gametic
	// is not a issue

	maketic = gametic + 1;
	neededtic = maketic;
	tictoclear = maketic;

	for (i = 0; i < MAXNETNODES; i++)
		ResetNode(i);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		playeringame[i] = false;
		playernode[i] = UINT8_MAX;
		sprintf(player_names[i], "Player %d", i + 1);
	}

	mynode = 0;
	cl_packetmissed = false;

	if (dedicated)
	{
		nodeingame[0] = true;
		serverplayer = 0;
	}
	else
		serverplayer = consoleplayer;

	if (server)
		servernode = 0;

	doomcom->numslots = 0;

	DEBFILE(va("\n-=-=-=-=-=-=-= Server Reset =-=-=-=-=-=-=-\n\n"));
}

//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame(void)
{
	if (!netgame || !netbuffer)
		return;

	DEBFILE("===========================================================================\n"
	        "                  Quitting Game, closing connection\n"
	        "===========================================================================\n");

	// abort send/receive of files
	CloseNetFile();

	if (server)
	{
		INT32 i;

		netbuffer->packettype = PT_SERVERSHUTDOWN;
		for (i = 0; i < MAXNETNODES; i++)
			if (nodeingame[i])
				HSendPacket(i, true, 0, 0);
		if (serverrunning && cv_internetserver.value)
			UnregisterServer();
	}
	else if (servernode > 0 && servernode < MAXNETNODES && nodeingame[(UINT8)servernode]!=0)
	{
		netbuffer->packettype = PT_CLIENTQUIT;
		HSendPacket(servernode, true, 0, 0);
	}

	D_CloseConnection();
	adminplayer = -1;

	DEBFILE("===========================================================================\n"
	        "                         Log finish\n"
	        "===========================================================================\n");
#ifdef DEBUGFILE
	if (debugfile)
	{
		fclose(debugfile);
		debugfile = NULL;
	}
#endif
}

// add a node to the game (player will follow at map change or at savegame....)
static inline void SV_AddNode(INT32 node)
{
	nettics[node] = gametic;
	supposedtics[node] = gametic;
	// little hack because the server connect to itself and put
	// nodeingame when connected not here
	if (node)
		nodeingame[node] = true;
}

// Xcmd XD_ADDPLAYER
static void Got_AddPlayer(UINT8 **p, INT32 playernum)
{
	INT16 node, newplayernum;
	boolean splitscreenplayer;
	static UINT32 sendconfigtic = UINT32_MAX;

	if (playernum != serverplayer && playernum != adminplayer)
	{
		// protect against hacked/buggy client
		CONS_Printf(text[ILLEGALADDPLRCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	node = READUINT8(*p);
	newplayernum = READUINT8(*p);
	splitscreenplayer = newplayernum & 0x80;
	newplayernum &= ~0x80;

	playeringame[newplayernum] = true;
	G_AddPlayer(newplayernum);
	if (newplayernum+1 > doomcom->numslots)
		doomcom->numslots = (INT16)(newplayernum+1);

	CONS_Printf(text[PLAYERINGAME], newplayernum+1, node);

	// the server is creating my player
	if (node == mynode)
	{
		playernode[newplayernum] = 0; // for information only
		if (!splitscreenplayer)
		{
			consoleplayer = newplayernum;
			displayplayer = newplayernum;
			secondarydisplayplayer = newplayernum;
			DEBFILE("spawning me\n");
		}
		else
		{
			secondarydisplayplayer = newplayernum;
			DEBFILE("spawning my brother\n");
		}
		addedtogame = true;
	}
	else if (server && cv_showjoinaddress.value)
	{
		const char *address;
		if (I_GetNodeAddress && (address = I_GetNodeAddress(node)) != NULL)
			CONS_Printf(text[PLAYERADDRESS], address);
	}

	// the new player sends his config
	// and the old players send their configs to the new one
	/// \todo fixthis
	/// WARNING: this can cause a bottleneck in the txtcmd
	///          this can also produce a consistency failure if the packet gets lost
	///          because everybody knows the actual config except the joiner

	// Don't forget to copy your name to the new location!
	if (newplayernum == consoleplayer && newplayernum != 0)
		strcpy(player_names[newplayernum], player_names[0]);

	// don't send the config more than once per tic (multiple players join)
	if (sendconfigtic != gametic)
	{
		sendconfigtic = gametic;
		D_SendPlayerConfig();
	}

	if (server && motd[0] != '\0')
		COM_BufAddText(va("sayto %d %s\n", newplayernum, motd));
}

static boolean SV_AddWaitingPlayers(void)
{
	INT32 node, n, newplayer = false;
	XBOXSTATIC UINT8 buf[2];
	UINT8 newplayernum = 0;

	// What is the reason for this? Why can't newplayernum always be 0?
	if (dedicated)
		newplayernum = 1;

	for (node = 0; node < MAXNETNODES; node++)
	{
		// splitscreen can allow 2 player in one node
		for (; nodewaiting[node] > 0; nodewaiting[node]--)
		{
			newplayer = true;

			// search for a free playernum
			// we can't use playeringame since it is not updated here
			for (; newplayernum < MAXPLAYERS; newplayernum++)
			{
				for (n = 0; n < MAXNETNODES; n++)
					if (nodetoplayer[n] == newplayernum || nodetoplayer2[n] == newplayernum)
						break;
				if (n == MAXNETNODES)
					break;
			}

			// should never happen since we check the playernum
			// before accepting the join
			I_Assert(newplayernum < MAXPLAYERS);

			playernode[newplayernum] = (UINT8)node;

			buf[0] = (UINT8)node;
			buf[1] = newplayernum;
			if (playerpernode[node] < 1)
				nodetoplayer[node] = newplayernum;
			else
			{
				nodetoplayer2[node] = newplayernum;
				buf[1] |= 0x80;
			}
			playerpernode[node]++;

			SendNetXCmd(XD_ADDPLAYER, &buf, 2);

			DEBFILE(va("Server added player %d node %d\n", newplayernum, node));
			// use the next free slot (we can't put playeringame[newplayernum] = true here)
			newplayernum++;
		}
	}

	return newplayer;
}

void CL_AddSplitscreenPlayer(void)
{
	if (cl_mode == cl_connected)
		CL_SendJoin();
}

void CL_RemoveSplitscreenPlayer(void)
{
	XBOXSTATIC UINT8 buf[2];

	if (cl_mode != cl_connected)
		return;

	buf[0] = (UINT8)secondarydisplayplayer;
	buf[1] = KICK_MSG_PLAYER_QUIT;
	SendNetXCmd(XD_KICK, &buf, 2);
}

// is there a game running
boolean Playing(void)
{
	return (server && serverrunning) || (!server && cl_mode == cl_connected);
}

boolean SV_SpawnServer(void)
{
	if (demoplayback)
		G_StopDemo(); // reset engine parameter

	if (!serverrunning)
	{
		CONS_Printf("%s", text[STARTINGSERVER]);
		serverrunning = true;
		SV_ResetServer();
		if (netgame && I_NetOpenSocket)
		{
			MSCloseUDPSocket();		// Tidy up before wiping the slate.
			I_NetOpenSocket();
			if (cv_internetserver.value)
				RegisterServer();
		}

		// non dedicated server just connect to itself
		if (!dedicated)
			CL_ConnectToServer(false);
		else doomcom->numslots = 1;
	}

	return SV_AddWaitingPlayers();
}

void SV_StopServer(void)
{
	tic_t i;

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission();
	gamestate = wipegamestate = GS_NULL;

	localtextcmd[0] = 0;
	localtextcmd2[0] = 0;

	for (i = 0; i < BACKUPTICS; i++)
		D_Clearticcmd(i);

	consoleplayer = 0;
	cl_mode = cl_searching;
	maketic = gametic+1;
	neededtic = maketic;
	serverrunning = false;
}

// called at singleplayer start and stopdemo
void SV_StartSinglePlayerServer(void)
{
	server = true;
	netgame = false;
	multiplayer = false;
	gametype = GT_COOP;

	// no more tic the game with this settings!
	SV_StopServer();

	if (splitscreen)
		multiplayer = true;
}

static void SV_SendRefuse(INT32 node, const char *reason)
{
	strcpy(netbuffer->u.serverrefuse.reason, reason);

	netbuffer->packettype = PT_SERVERREFUSE;
	HSendPacket(node, true, 0, strlen(netbuffer->u.serverrefuse.reason) + 1);
	Net_CloseConnection(node);
}

static inline void writeconplayer(cons_pak *con, const size_t i)
{
	size_t j;

	con->playernum = (UINT8)i;

	con->playerstate = (UINT8)players[i].playerstate;
	G_MoveTiccmd(&con->cmd, &players[i].cmd, 1);
	con->viewz = LONG(players[i].viewz);
	con->viewheight = LONG(players[i].viewheight);
	con->deltaviewheight = LONG(players[i].deltaviewheight);
	con->bob = LONG(players[i].bob);
	con->aiming = (angle_t)LONG(players[i].aiming);
	con->awayviewaiming = (angle_t)LONG(players[i].awayviewaiming);
	con->phealth = LONG(players[i].health);
	con->currentweapon = LONG(players[i].currentweapon);
	con->ringweapons = LONG(players[i].ringweapons);
	con->tossstrength = LONG(players[i].tossstrength);

	for (j = 0; j < NUMPOWERS; j++)
		con->powers[j] = LONG(players[i].powers[j]);

	con->pflags = (UINT32)LONG(players[i].pflags);
	con->bonuscount = LONG(players[i].bonuscount);
	con->skincolor = LONG(players[i].skincolor);
	con->skin = LONG(players[i].skin);
	con->score = (UINT32)LONG(players[i].score);
	con->dashspeed = LONG(players[i].dashspeed);
	con->normalspeed = LONG(players[i].normalspeed);
	con->runspeed = LONG(players[i].runspeed);
	con->thrustfactor = LONG(players[i].thrustfactor);
	con->accelstart = LONG(players[i].accelstart);
	con->acceleration = LONG(players[i].acceleration);
	con->charability = LONG(players[i].charability);
	con->charability2 = LONG(players[i].charability2);
	con->charflags = (UINT32)LONG(players[i].charflags);
	con->thokitem = (UINT32)LONG(players[i].thokitem);
	con->spinitem = (UINT32)LONG(players[i].spinitem);
	con->actionspd = LONG(players[i].actionspd);
	con->mindash = LONG(players[i].mindash);
	con->maxdash = LONG(players[i].maxdash);
	con->jumpfactor = LONG(players[i].jumpfactor);
#ifndef TRANSFIX
	con->starttranscolor = LONG(players[i].starttranscolor);
#endif
	con->prefcolor = LONG(players[i].prefcolor);
	con->lives = LONG(players[i].lives);
	con->continues = LONG(players[i].continues);
	con->xtralife = LONG(players[i].xtralife);
	con->speed = LONG(players[i].speed);
	con->jumping = LONG(players[i].jumping);
	con->secondjump =players[i].secondjump;
	con->fly1 = LONG(players[i].fly1);
	con->scoreadd = (UINT32)LONG(players[i].scoreadd);
	con->glidetime = (tic_t)LONG(players[i].glidetime);
	con->climbing = LONG(players[i].climbing);
	con->deadtimer = LONG(players[i].deadtimer);
	con->splish = LONG(players[i].splish);
	con->exiting = (tic_t)LONG(players[i].exiting);
	con->blackow = LONG(players[i].blackow);
	con->homing = players[i].homing;
	con->cmomx = LONG(players[i].cmomx);
	con->cmomy = LONG(players[i].cmomy);
	con->rmomx = LONG(players[i].rmomx);
	con->rmomy = LONG(players[i].rmomy);
	con->numboxes = LONG(players[i].numboxes);
	con->totalring = LONG(players[i].totalring);
	con->realtime = (tic_t)LONG(players[i].realtime);
	con->racescore = (UINT32)LONG(players[i].racescore);
	con->laps = (UINT32)LONG(players[i].laps);
	con->ctfteam = LONG(players[i].ctfteam);
	con->gotflag = (UINT16)SHORT(players[i].gotflag);
	con->dbginfo = LONG(players[i].dbginfo);
	con->emeraldhunt = LONG(players[i].emeraldhunt);
	con->weapondelay = LONG(players[i].weapondelay);
	con->tossdelay = LONG(players[i].tossdelay);
	con->shielddelay = LONG(players[i].shielddelay);
	con->taunttimer = (tic_t)LONG(players[i].taunttimer);
	con->starpostx = LONG(players[i].starpostx);
	con->starposty = LONG(players[i].starposty);
	con->starpostz = LONG(players[i].starpostz);
	con->starpostnum = LONG(players[i].starpostnum);
	con->starposttime = (tic_t)LONG(players[i].starposttime);
	con->starpostangle = (angle_t)LONG(players[i].starpostangle);
	con->starpostbit = (UINT32)LONG(players[i].starpostbit);
	con->angle_pos = (angle_t)LONG(players[i].angle_pos);
	con->old_angle_pos = (angle_t)LONG(players[i].old_angle_pos);
	con->bumpertime = (tic_t)LONG(players[i].bumpertime);
	con->flyangle = LONG(players[i].flyangle);
	con->drilltimer = (tic_t)LONG(players[i].drilltimer);
	con->linkcount = LONG(players[i].linkcount);
	con->linktimer = (tic_t)LONG(players[i].linktimer);
	con->anotherflyangle = LONG(players[i].anotherflyangle);
	con->nightstime = (tic_t)LONG(players[i].nightstime);
	con->drillmeter = LONG(players[i].drillmeter);
	con->drilldelay = players[i].drilldelay;
	con->bonustime = players[i].bonustime;
	con->mare = players[i].mare;
	con->lastsidehit = SHORT(players[i].lastsidehit);
	con->lastlinehit = SHORT(players[i].lastlinehit);
	con->losscount = LONG(players[i].losscount);
	con->onconveyor = LONG(players[i].onconveyor);
	con->spectator = players[i].spectator;
	con->jointime = (tic_t)LONG(players[i].jointime);

	con->hasmo = false;
	//Transfer important mo information if the player has a body.
	//This lets us resync players even if they are dead.
	if (!players[i].mo)
		return;

	con->hasmo = true;
	con->angle = (angle_t)LONG(players[i].mo->angle);
	con->eflags = (UINT32)LONG(players[i].mo->eflags);
	con->flags = LONG(players[i].mo->flags);
	con->flags2 = LONG(players[i].mo->flags2);
	con->friction = LONG(players[i].mo->friction);
	con->health = LONG(players[i].mo->health);
	con->momx = LONG(players[i].mo->momx);
	con->momy = LONG(players[i].mo->momy);
	con->momz = LONG(players[i].mo->momz);
	con->movefactor = LONG(players[i].mo->movefactor);
	con->tics = LONG(players[i].mo->tics);
	con->statenum = (statenum_t)(players[i].mo->state-states); // :(
	con->x = LONG(players[i].mo->x);
	con->y = LONG(players[i].mo->y);
	con->z = LONG(players[i].mo->z);
	con->radius = LONG(players[i].mo->radius);
	con->height = LONG(players[i].mo->height);
	con->scale = (UINT16)SHORT(players[i].mo->scale);
	con->destscale = (UINT16)SHORT(players[i].mo->destscale);
	con->scalespeed = players[i].mo->scalespeed;
}

static inline void readconplayer(cons_pak *con, const INT32 playernum)
{
	size_t i;
	mobj_t *savedmo = players[playernum].mo;

	//We get a packet for each player in game.
	P_SetRandIndex(con->randomseed); // New random index

	//Restore CTF information
	if (gametype == GT_CTF)
	{
		// Remove old flags.
		if (redflag)
		{
			P_SetMobjState(redflag, S_DISS);
			redflag = NULL;
		}
		if (blueflag)
		{
			P_SetMobjState(blueflag, S_DISS);
			blueflag = NULL;
		}

		// Spawn the flags if players aren't carrying them.
		if (con->rflagloose != 2)
		{
			mobj_t *newflag = P_SpawnMobj(con->rflagx << FRACBITS, con->rflagy << FRACBITS, con->rflagz << FRACBITS, MT_REDFLAG);
			newflag->flags |= MF_SPECIAL;
			newflag->flags2 = con->rflags2;
			newflag->fuse = con->rfuse;
			newflag->spawnpoint = rflagpoint;
			redflag = newflag;
		}

		if (con->bflagloose != 2)
		{
			mobj_t *newflag = P_SpawnMobj(con->bflagx << FRACBITS, con->bflagy << FRACBITS, con->bflagz << FRACBITS, MT_BLUEFLAG);
			newflag->flags |= MF_SPECIAL;
			newflag->flags2 = con->bflags2;
			newflag->fuse = con->bfuse;
			newflag->spawnpoint = bflagpoint;
			blueflag = newflag;
		}
	}

	if (!playeringame[playernum])
		return;

	//Tranfer player information.
	players[playernum].playerstate = (playerstate_t)con->playerstate;
	G_MoveTiccmd(&players[playernum].cmd, &con->cmd, 1);
	players[playernum].viewz = LONG(con->viewz);
	players[playernum].viewheight = LONG(con->viewheight);
	players[playernum].deltaviewheight = LONG(con->deltaviewheight);
	players[playernum].bob = LONG(con->bob);
	players[playernum].aiming = (angle_t)LONG(con->aiming);
	players[playernum].awayviewaiming = (angle_t)LONG(con->awayviewaiming);
	players[playernum].health = LONG(con->phealth);
	players[playernum].currentweapon = LONG(con->currentweapon);
	players[playernum].ringweapons = LONG(con->ringweapons);
	players[playernum].tossstrength = LONG(con->tossstrength);

	for (i = 0; i < NUMPOWERS; i++)
		players[playernum].powers[i] = LONG(con->powers[i]);

	players[playernum].pflags = (pflags_t)LONG(con->pflags);
	players[playernum].bonuscount = LONG(con->bonuscount);
	players[playernum].skincolor = LONG(con->skincolor);
	players[playernum].skin = LONG(con->skin);
	players[playernum].score = (UINT32)LONG(con->score);
	players[playernum].dashspeed = LONG(con->dashspeed);
	players[playernum].normalspeed = LONG(con->normalspeed);
	players[playernum].runspeed = LONG(con->runspeed);
	players[playernum].thrustfactor = LONG(con->thrustfactor);
	players[playernum].accelstart = LONG(con->accelstart);
	players[playernum].acceleration = LONG(con->acceleration);
	players[playernum].charability = LONG(con->charability);
	players[playernum].charability2 = LONG(con->charability2);
	players[playernum].charflags = (UINT32)LONG(con->charflags);
	players[playernum].thokitem = (mobjtype_t)LONG(con->thokitem);
	players[playernum].spinitem = (mobjtype_t)LONG(con->spinitem);
	players[playernum].actionspd = LONG(con->actionspd);
	players[playernum].mindash = LONG(con->mindash);
	players[playernum].maxdash = LONG(con->maxdash);
	players[playernum].jumpfactor = LONG(con->jumpfactor);
#ifndef TRANSFIX
	players[playernum].starttranscolor = LONG(con->starttranscolor);
#endif
	players[playernum].prefcolor = LONG(con->prefcolor);
	players[playernum].lives = LONG(con->lives);
	players[playernum].continues = LONG(con->continues);
	players[playernum].xtralife = LONG(con->xtralife);
	players[playernum].speed = LONG(con->speed);
	players[playernum].jumping = LONG(con->jumping);
	players[playernum].secondjump = con->secondjump;
	players[playernum].fly1 = LONG(con->fly1);
	players[playernum].scoreadd = (UINT32)LONG(con->scoreadd);
	players[playernum].glidetime = (tic_t)LONG(con->glidetime);
	players[playernum].climbing = LONG(con->climbing);
	players[playernum].deadtimer = LONG(con->deadtimer);
	players[playernum].splish = LONG(con->splish);
	players[playernum].exiting = (tic_t)LONG(con->exiting);
	players[playernum].blackow = LONG(con->blackow);
	players[playernum].homing = con->homing;
	players[playernum].cmomx = LONG(con->cmomx);
	players[playernum].cmomy = LONG(con->cmomy);
	players[playernum].rmomx = LONG(con->rmomx);
	players[playernum].rmomy = LONG(con->rmomy);
	players[playernum].numboxes = LONG(con->numboxes);
	players[playernum].totalring = LONG(con->totalring);
	players[playernum].realtime = (tic_t)LONG(con->realtime);
	players[playernum].racescore = (UINT32)LONG(con->racescore);
	players[playernum].laps = (UINT32)LONG(con->laps);
	players[playernum].ctfteam = LONG(con->ctfteam);
	players[playernum].gotflag = (UINT16)SHORT(con->gotflag);
	players[playernum].dbginfo = LONG(con->dbginfo);
	players[playernum].emeraldhunt = LONG(con->emeraldhunt);
	players[playernum].weapondelay = LONG(con->weapondelay);
	players[playernum].tossdelay = LONG(con->tossdelay);
	players[playernum].shielddelay = LONG(con->shielddelay);
	players[playernum].taunttimer = (tic_t)LONG(con->taunttimer);
	players[playernum].starpostx = LONG(con->starpostx);
	players[playernum].starposty = LONG(con->starposty);
	players[playernum].starpostz = LONG(con->starpostz);
	players[playernum].starpostnum = LONG(con->starpostnum);
	players[playernum].starposttime = (tic_t)LONG(con->starposttime);
	players[playernum].starpostangle = (angle_t)LONG(con->starpostangle);
	players[playernum].starpostbit = (UINT32)LONG(con->starpostbit);
	players[playernum].angle_pos = (angle_t)LONG(con->angle_pos);
	players[playernum].old_angle_pos = (angle_t)LONG(con->old_angle_pos);
	players[playernum].bumpertime = (tic_t)LONG(con->bumpertime);
	players[playernum].flyangle = LONG(con->flyangle);
	players[playernum].drilltimer = (tic_t)LONG(con->drilltimer);
	players[playernum].linkcount = LONG(con->linkcount);
	players[playernum].linktimer = (tic_t)LONG(con->linktimer);
	players[playernum].anotherflyangle = LONG(con->anotherflyangle);
	players[playernum].nightstime = (tic_t)LONG(con->nightstime);
	players[playernum].drillmeter = LONG(con->drillmeter);
	players[playernum].drilldelay = con->drilldelay;
	players[playernum].bonustime = con->bonustime;
	players[playernum].mare = con->mare;
	players[playernum].lastsidehit = SHORT(con->lastsidehit);
	players[playernum].lastlinehit = SHORT(con->lastlinehit);
	players[playernum].losscount = LONG(con->losscount);
	players[playernum].onconveyor = LONG(con->onconveyor);
	players[playernum].spectator = con->spectator;
	players[playernum].jointime = (tic_t)LONG(con->jointime);

	//...but keep old mo even if it is corrupt or null!
	players[playernum].mo = savedmo;

	//Transfer important mo information if they have a valid mo.
	if (!con->hasmo)
		return;
	//server thinks player has a body.
	//Give them a new body that can be then manipulated by the server's info.
	if (!players[playernum].mo) //client thinks it has no body.
		P_SpawnPlayer(playerstarts[0], playernum);

	//At this point, the player should have a body, whether they were respawned or not.
	P_UnsetThingPosition(players[playernum].mo);
	players[playernum].mo->angle = (angle_t)LONG(con->angle);
	players[playernum].mo->eflags = (UINT32)LONG(con->eflags);
	players[playernum].mo->flags = LONG(con->flags);
	players[playernum].mo->flags2 = LONG(con->flags2);
	players[playernum].mo->friction = LONG(con->friction);
	players[playernum].mo->health = LONG(con->health);
	players[playernum].mo->momx = LONG(con->momx);
	players[playernum].mo->momy = LONG(con->momy);
	players[playernum].mo->momz = LONG(con->momz);
	players[playernum].mo->movefactor = LONG(con->movefactor);
	players[playernum].mo->tics = LONG(con->tics);
	P_SetPlayerMobjState(players[playernum].mo, con->statenum);
	players[playernum].mo->x = LONG(con->x);
	players[playernum].mo->y = LONG(con->y);
	players[playernum].mo->z = LONG(con->z);
	players[playernum].mo->radius = LONG(con->radius);
	players[playernum].mo->height = LONG(con->height);
	// P_SetScale is redundant for this, as all related variables are already restored properly.
	players[playernum].mo->scale = (UINT16)SHORT(con->scale);
	players[playernum].mo->destscale = (UINT16)SHORT(con->destscale);
	players[playernum].mo->scalespeed = con->scalespeed;

	P_SetThingPosition(players[playernum].mo);
}

static inline void handlectfconstuff(cons_pak *con)
{
	if (redflag)
	{
		// Flag is loose
		if (redflag->fuse)
		{
			con->rflagloose = 1;
			con->rflagx = SHORT(redflag->x >> FRACBITS);
			con->rflagy = SHORT(redflag->y >> FRACBITS);
			con->rflagz = SHORT(redflag->z >> FRACBITS);
			con->rflags2 = LONG(redflag->flags2);
			con->rfuse = LONG(redflag->fuse);
		}
		else // flag is at base
		{
			con->rflagloose = 0;
			con->rflagx = SHORT(rflagpoint->x);
			con->rflagy = SHORT(rflagpoint->y);
			con->rflagz = SHORT(rflagpoint->z);
			con->rflags2 = 0;
			con->rfuse = 0;
		}
	}
	else // player has flag
		con->rflagloose = 2;

	if (blueflag)
	{
		// Flag is loose
		if (blueflag->fuse)
		{
			con->bflagloose = 1;
			con->bflagx = SHORT(blueflag->x >> FRACBITS);
			con->bflagy = SHORT(blueflag->y >> FRACBITS);
			con->bflagz = SHORT(blueflag->z >> FRACBITS);
			con->bflags2 = LONG(blueflag->flags2);
			con->bfuse = LONG(blueflag->fuse);
		}
		else // flag is at base
		{
			con->bflagloose = 0;
			con->bflagx = SHORT(bflagpoint->x);
			con->bflagy = SHORT(bflagpoint->y);
			con->bflagz = SHORT(bflagpoint->z);
			con->bflags2 = 0;
			con->bfuse = 0;
		}
	}
	else // player has flag
		con->bflagloose = 2;
}

static void SV_SendConsistency(INT32 node)
{
	INT32 i;

	netbuffer->packettype = PT_CONSISTENCY;
	netbuffer->u.consistency.randomseed = P_GetRandIndex();

	if (gametype == GT_CTF)
		handlectfconstuff(&netbuffer->u.consistency);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			writeconplayer(&netbuffer->u.consistency, i);

			HSendPacket(node, true, 0, (sizeof(cons_pak)));
		}
	}
}

// used at txtcmds received to check packetsize bound
static size_t TotalTextCmdPerTic(tic_t tic)
{
	size_t i, total = 1; // num of textcmds in the tic (ntextcmd byte)

	tic %= BACKUPTICS;

	for (i = 0; i < MAXPLAYERS; i++)
		if ((!i || playeringame[i]) && textcmds[tic][i][0])
			total += 2 + textcmds[tic][i][0]; // "+2" for size and playernum

	return total;
}

/**	\brief GetPackets

  \todo  break this 300 line function into multiple functions
*/
static void GetPackets(void)
{FILESTAMP
	XBOXSTATIC INT32 netconsole;
	XBOXSTATIC SINT8 node;
	XBOXSTATIC tic_t realend,realstart;
	XBOXSTATIC UINT8 *pak, *txtpak, numtxtpak;
	INT32 p = maketic%BACKUPTICS;
FILESTAMP
	while (HGetPacket())
	{
		node = (SINT8)doomcom->remotenode;
		if (netbuffer->packettype == PT_CLIENTJOIN && server)
		{
			if (bannednode && bannednode[node])
				SV_SendRefuse(node, "You have been banned\nfrom the server");
			else if (netbuffer->u.clientcfg.version != VERSION
				|| netbuffer->u.clientcfg.subversion != SUBVERSION)
				SV_SendRefuse(node, va("Different SRB2 versions cannot\nplay a netgame!\n"
				"(server version %d.%d.%d)", VERSION/100, VERSION%100, SUBVERSION));
			else if (!cv_allownewplayer.value && node)
				SV_SendRefuse(node, "The server is not accepting\njoins for the moment");
			else if (D_NumPlayers() >= cv_maxplayers.value)
				SV_SendRefuse(node, va("Maximum players reached: %d", cv_maxplayers.value));
			else if (netgame && netbuffer->u.clientcfg.localplayers > 1)	// Hacked client?
				SV_SendRefuse(node, va("Too many players from\nthis node."));
			else if (netgame && !netbuffer->u.clientcfg.localplayers) // Stealth join?
				SV_SendRefuse(node, va("No players from\nthis node."));
			else
			{
				boolean newnode = false;

				// client authorised to join
				nodewaiting[node] = (UINT8)(netbuffer->u.clientcfg.localplayers - playerpernode[node]);
				if (!nodeingame[node])
				{
					gamestate_t backupstate = gamestate;
					newnode = true;
					SV_AddNode(node);
					if (cv_joinnextround.value && gameaction == ga_nothing)
						G_SetGamestate(GS_WAITINGPLAYERS);
					if (!SV_SendServerConfig(node))
					{
						G_SetGamestate(backupstate);
						ResetNode(node);
						SV_SendRefuse(node, "Server couldn't send info, please try again");
						/// \todo fix this !!!
						//CONS_Printf("Internal Error 5: node %d lost\n",node);
						continue; // restart the while
					}
					G_SetGamestate(backupstate);
					DEBFILE("new node joined\n");
				}
#ifdef JOININGAME
				if (nodewaiting[node])
				{
					if ((gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) && newnode)
					{
						SV_SendSaveGame(node);
						DEBFILE("send savegame\n");
					}
					SV_AddWaitingPlayers();
				}
#endif
			}
			continue;
		} // end of PT_CLIENTJOIN
		if (netbuffer->packettype == PT_SERVERSHUTDOWN && node == servernode
			&& !server && cl_mode != cl_searching)
		{
			M_StartMessage("Server has shutdown\n\nPress Esc", NULL, MM_NOTHING);
			D_QuitNetGame();
			CL_Reset();
			D_StartTitle();
			continue;
		}
		if (netbuffer->packettype == PT_NODETIMEOUT && node == servernode
			&& !server && cl_mode != cl_searching)
		{
			M_StartMessage("Server Timeout\n\nPress Esc", NULL, MM_NOTHING);
			D_QuitNetGame();
			CL_Reset();
			D_StartTitle();
			continue;
		}

		if (netbuffer->packettype == PT_SERVERINFO)
		{
			// compute ping in ms
			const tic_t ticnow = I_GetTime();
			const tic_t ticthen = (tic_t)LONG(netbuffer->u.serverinfo.time);
			const tic_t ticdiff = (ticnow - ticthen)*1000/TICRATE;
			netbuffer->u.serverinfo.time = (tic_t)LONG(ticdiff);
			netbuffer->u.serverinfo.servername[MAXSERVERNAME-1] = 0;

			SL_InsertServer(&netbuffer->u.serverinfo, node);
			continue;
		}

		if (netbuffer->packettype == PT_PLAYERINFO)
			continue; // We do nothing with PLAYERINFO, that's for the MS browser.

		if (!nodeingame[node])
		{
			if (node != servernode)
				DEBFILE(va("Received packet from unknown host %d\n", node));

			// anyone trying to join
			switch (netbuffer->packettype)
			{
				case PT_ASKINFOVIAMS:
					if (server && serverrunning)
					{
						INT32 clientnode = I_NetMakeNode(netbuffer->u.msaskinfo.clientaddr);
						SV_SendServerInfo(clientnode, (tic_t)LONG(netbuffer->u.msaskinfo.time));
						SV_SendPlayerInfo(clientnode); // send extra info
						Net_CloseConnection(clientnode);
						// Don't close connection to MS.
					}
					break;

				case PT_ASKINFO:
					if (server && serverrunning)
					{
						SV_SendServerInfo(node, (tic_t)LONG(netbuffer->u.askinfo.time));
						SV_SendPlayerInfo(node); // send extra info
						Net_CloseConnection(node);
					}
					break;
				case PT_SERVERREFUSE: // negative response of client join request
					if (cl_mode == cl_waitjoinresponse)
					{
						M_StartMessage(va("Server refuses connection\n\nReason:\n%s",
							netbuffer->u.serverrefuse.reason), NULL, MM_NOTHING);
						D_QuitNetGame();
						CL_Reset();
						D_StartTitle();

						// Will be reset by caller. Signals refusal.
						cl_mode = cl_aborted;
					}
					break;
				case PT_SERVERCFG: // positive response of client join request
				{
					INT32 j;
					UINT8 *scp;
					UINT32 playermask = 0;

					/// \note how would this happen? and is it doing the right thing if it does?
					if (cl_mode != cl_waitjoinresponse)
						break;

					if (!server)
					{
						maketic = gametic = neededtic = (tic_t)LONG(netbuffer->u.servercfg.gametic);
						gametype = netbuffer->u.servercfg.gametype;
						modifiedgame = netbuffer->u.servercfg.modifiedgame;
						adminplayer = netbuffer->u.servercfg.adminplayer;
					}

					nodeingame[(UINT8)servernode] = true;
					serverplayer = netbuffer->u.servercfg.serverplayer;
					doomcom->numslots = SHORT(netbuffer->u.servercfg.totalslotnum);
					mynode = netbuffer->u.servercfg.clientnode;
					if (serverplayer >= 0)
						playernode[(UINT8)serverplayer] = servernode;

					CONS_Printf("%s", text[JOINACCEPTED]);
					DEBFILE(va("Server accept join gametic=%u mynode=%d\n", gametic, mynode));

					playermask = LONG(netbuffer->u.servercfg.playerdetected);
					for (j = 0; j < MAXPLAYERS; j++)
						playeringame[j] = (playermask & (1<<j)) != 0;

					scp = netbuffer->u.servercfg.netcvarstates;
					CV_LoadNetVars(&scp);
#ifdef JOININGAME
					if (netbuffer->u.servercfg.gamestate == GS_LEVEL ||
						netbuffer->u.servercfg.gamestate == GS_INTERMISSION)
						cl_mode = cl_downloadsavegame;
					else
#endif
						cl_mode = cl_connected;
					break;
				}
				// handled in d_netfil.c
				case PT_FILEFRAGMENT:
					if (!server)
						Got_Filetxpak();
					break;
				case PT_REQUESTFILE:
					if (server)
						Got_RequestFilePak(node);
					break;
				case PT_NODETIMEOUT:
				case PT_CLIENTQUIT:
					if (server)
						Net_CloseConnection(node);
					break;
				case PT_SERVERTICS:
					// do not remove my own server (we have just get a out of order packet)
					if (node == servernode)
						break;
				case PT_CLIENTCMD:
					break; // this is not an "unknown packet"
				default:
					DEBFILE(va("unknown packet received (%d) from unknown host\n",
						netbuffer->packettype));
					Net_CloseConnection(node);
					break; // ignore it
			} // switch
			continue; //while
		}
		if (dedicated && node == 0) netconsole = 0;
		else netconsole = nodetoplayer[node];
#ifdef PARANOIA
		if (netconsole >= MAXPLAYERS)
			I_Error("bad table nodetoplayer: node %d player %d", doomcom->remotenode, netconsole);
#endif

		switch (netbuffer->packettype)
		{
// -------------------------------------------- SERVER RECEIVE ----------
			case PT_CLIENTCMD:
			case PT_CLIENT2CMD:
			case PT_CLIENTMIS:
			case PT_CLIENT2MIS:
			case PT_NODEKEEPALIVE:
			case PT_NODEKEEPALIVEMIS:
				if (!server)
					break;

				// to save bytes, only the low byte of tic numbers are sent
				// Figure out what the rest of the bytes are
				realstart = ExpandTics(netbuffer->u.clientpak.client_tic);
				realend = ExpandTics(netbuffer->u.clientpak.resendfrom);

				if (netbuffer->packettype == PT_CLIENTMIS || netbuffer->packettype == PT_CLIENT2MIS
					|| netbuffer->packettype == PT_NODEKEEPALIVEMIS
					|| supposedtics[node] < realend)
				{
					supposedtics[node] = realend;
				}
				// discard out of order packet
				if (nettics[node] > realend)
				{
					DEBFILE(va("out of order ticcmd discarded nettics = %u\n", nettics[node]));
					break;
				}

				// update the nettics
				nettics[node] = realend;

				// don't do anything for packets of type NODEKEEPALIVE?
				if (netconsole == -1 || netbuffer->packettype == PT_NODEKEEPALIVE
					|| netbuffer->packettype == PT_NODEKEEPALIVEMIS)
					break;

				// copy ticcmd
				G_MoveTiccmd(&netcmds[maketic%BACKUPTICS][netconsole], &netbuffer->u.clientpak.cmd, 1);

				// check ticcmd for "speed hacks"
				if (netcmds[maketic%BACKUPTICS][netconsole].forwardmove > MAXPLMOVE || netcmds[maketic%BACKUPTICS][netconsole].forwardmove < -MAXPLMOVE
					|| netcmds[maketic%BACKUPTICS][netconsole].sidemove > MAXPLMOVE || netcmds[maketic%BACKUPTICS][netconsole].sidemove < -MAXPLMOVE)
				{
					XBOXSTATIC char buf[2];
					CONS_Printf("Illegal movement value recieved from node %d\n", netconsole);
					//D_Clearticcmd(k);

					buf[0] = (char)netconsole;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				// splitscreen cmd
				if (netbuffer->packettype == PT_CLIENT2CMD && nodetoplayer2[node] >= 0)
					G_MoveTiccmd(&netcmds[maketic%BACKUPTICS][(UINT8)nodetoplayer2[node]],
						&netbuffer->u.client2pak.cmd2, 1);

				// check player consistancy during the level
				// Careful: When a consistency packet is sent, it overwrites the incoming packet containing the ticcmd.
				//          Keep this in mind when changing the code that responds to these packets.
				if (realstart <= gametic && realstart > gametic - BACKUPTICS+1
					&& consistancy[realstart%BACKUPTICS] != SHORT(netbuffer->u.clientpak.consistancy)
					&& gamestate == GS_LEVEL)
				{
					if (cv_consfailprotect.value && playeringame[netconsole] && consfailcount[netconsole] < cv_consfailprotect.value)
					{
						if (!consfailstatus[netconsole])
						{
							if (cv_blamecfail.value)
								CONS_Printf(text[CONSFAILRESTORE], netconsole);

							DEBFILE(va("Restoring player %d (consistency failure) [%update] %d!=%d\n",
								netconsole, realstart, consistancy[realstart%BACKUPTICS],
								SHORT(netbuffer->u.clientpak.consistancy)));

							SV_SendConsistency(netconsole);
							consfailstatus[netconsole] = 1;
							consfailcount[netconsole]++;
						}
						else
						{
							//We don't want to send any more packets than we have to.
							//If the client doesn't resync in a certain time,
							//assume they didn't get the packet. Send another.
							if (consfailstatus[netconsole] < 10)
								consfailstatus[netconsole]++;
							else
								consfailstatus[netconsole] = 0;
						}
					}
					else
					{
						XBOXSTATIC UINT8 buf[3];

						buf[0] = (UINT8)netconsole;
						buf[1] = KICK_MSG_CON_FAIL;
						SendNetXCmd(XD_KICK, &buf, 2);
						DEBFILE(va("player %d kicked (consistency failure) [%u] %d!=%d\n",
							netconsole, realstart, consistancy[realstart%BACKUPTICS],
							SHORT(netbuffer->u.clientpak.consistancy)));
					}
				}
				else
					consfailstatus[netconsole] = 0;

				break;
			case PT_TEXTCMD2: // splitscreen special
				netconsole = nodetoplayer2[node];
			case PT_TEXTCMD:
				if (!server)
					break;

				if (netconsole < 0 || netconsole >= MAXPLAYERS)
					Net_UnAcknowledgPacket(node);
				else
				{
					size_t j;
					tic_t tic = maketic;

					// check if tic that we are making isn't too large else we cannot send it :(
					// doomcom->numslots+1 "+1" since doomcom->numslots can change within this time and sent time
					j = software_MAXPACKETLENGTH
						- (netbuffer->u.textcmd[0]+2+BASESERVERTICSSIZE
						+ (doomcom->numslots+1)*sizeof (ticcmd_t));

					// search a tic that have enougth space in the ticcmd
					while ((TotalTextCmdPerTic(tic) > j || netbuffer->u.textcmd[0]
						+ textcmds[tic%BACKUPTICS][netconsole][0] > MAXTEXTCMD)
						&& tic < firstticstosend + BACKUPTICS)
						tic++;

					if (tic >= firstticstosend + BACKUPTICS)
					{
						DEBFILE(va("GetPacket: Textcmd too long (max %"PRIdS", used %"PRIdS", mak %d, "
							"tosend %u, node %u, player %d)\n", j, TotalTextCmdPerTic(maketic),
							maketic, firstticstosend, node, netconsole));
						Net_UnAcknowledgPacket(node);
						break;
					}
					DEBFILE(va("textcmd put in tic %u at position %d (player %d) ftts %u mk %u\n",
						tic, textcmds[p][netconsole][0]+1, netconsole, firstticstosend, maketic));
					p = tic % BACKUPTICS;
					M_Memcpy(&textcmds[p][netconsole][textcmds[p][netconsole][0]+1],
						netbuffer->u.textcmd+1, netbuffer->u.textcmd[0]);
					textcmds[p][netconsole][0] = (UINT8)(textcmds[p][netconsole][0] + (UINT8)netbuffer->u.textcmd[0]);
				}
				break;
			case PT_NODETIMEOUT:
			case PT_CLIENTQUIT:
				if (!server)
					break;

				// nodeingame will be put false in the execution of kick command
				// this allow to send some packets to the quitting client to have their ack back
				nodewaiting[node] = 0;
				if (netconsole != -1 && playeringame[netconsole])
				{
					XBOXSTATIC UINT8 buf[2];
					buf[0] = (UINT8)netconsole;
					if (netbuffer->packettype == PT_NODETIMEOUT)
						buf[1] = KICK_MSG_TIMEOUT;
					else
						buf[1] = KICK_MSG_PLAYER_QUIT;
					SendNetXCmd(XD_KICK, &buf, 2);
					nodetoplayer[node] = -1;
					if (nodetoplayer2[node] != -1 && nodetoplayer2[node] >= 0
						&& playeringame[(UINT8)nodetoplayer2[node]])
					{
						buf[0] = nodetoplayer2[node];
						SendNetXCmd(XD_KICK, &buf, 2);
						nodetoplayer2[node] = -1;
					}
				}
				Net_CloseConnection(node);
				nodeingame[node] = false;
				break;
// -------------------------------------------- CLIENT RECEIVE ----------
			case PT_SERVERTICS:
				// Only accept PT_SERVERTICS from the server.
				if (node != servernode)
				{
					DEBFILE(va("PT_SERVERTICS recieved from non-host %d\n", node));

					if (server)
					{
						XBOXSTATIC UINT8 buf[2];
						CONS_Printf("PT_SERVERTICS recieved from non-host %d\n", node);

						buf[0] = (UINT8)node;
						buf[1] = KICK_MSG_CON_FAIL;
						SendNetXCmd(XD_KICK, &buf, 2);
					}

					break;
				}

				realstart = ExpandTics(netbuffer->u.serverpak.starttic);
				realend = realstart + netbuffer->u.serverpak.numtics;

				txtpak = (UINT8 *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numslots
					* netbuffer->u.serverpak.numtics];

				if (realend > gametic + BACKUPTICS)
					realend = gametic + BACKUPTICS;
				cl_packetmissed = realstart > neededtic;

				if (realstart <= neededtic && realend > neededtic)
				{
					tic_t i, j;
					pak = (UINT8 *)&netbuffer->u.serverpak.cmds;

					for (i = realstart; i < realend; i++)
					{
						// clear first
						D_Clearticcmd(i);

						// copy the tics
						pak = G_ScpyTiccmd(netcmds[i%BACKUPTICS], pak,
							netbuffer->u.serverpak.numslots*sizeof (ticcmd_t));

						// copy the textcmds
						numtxtpak = *txtpak++;
						for (j = 0; j < numtxtpak; j++)
						{
							INT32 k = *txtpak++; // playernum
							const size_t txtsize = txtpak[0]+1;

							M_Memcpy(textcmds[i%BACKUPTICS][k], txtpak, txtsize);
							txtpak += txtsize;
						}
					}

					neededtic = realend;
				}
				else
					DEBFILE(va("frame not in bound: %u\n", neededtic));
				break;
			case PT_CONSISTENCY:
				// Only accept PT_CONSISTENCY from the server.
				if (node != servernode)
				{
					DEBFILE(va("PT_CONSISTENCY recieved from non-host %d\n", node));

					if (server)
					{
						XBOXSTATIC char buf[2];
						CONS_Printf("PT_CONSISTENCY recieved from non-host %d\n", node);

						buf[0] = (char)node;
						buf[1] = KICK_MSG_CON_FAIL;
						SendNetXCmd(XD_KICK, &buf, 2);
					}

					break;
				}

				readconplayer(&netbuffer->u.consistency, netbuffer->u.consistency.playernum);

				break;
#ifdef NEWPING
			case PT_PING:
				// Only accept PT_PING from the server.
				if (node != servernode)
				{
					DEBFILE(va("PT_PING recieved from non-host %d\n", node));

					if (server)
					{
						XBOXSTATIC char buf[2];
						CONS_Printf("PT_PING recieved from non-host %d\n", node);

						buf[0] = (char)node;
						buf[1] = KICK_MSG_CON_FAIL;
						SendNetXCmd(XD_KICK, &buf, 2);
					}

					break;
				}

				//Update client ping table from the server.
				if (!server)
				{
					INT32 i;
					for (i = 0; i < MAXNETNODES; i++)
						if (playeringame[i])
							playerpingtable[i] = (tic_t)netbuffer->u.pingtable[i];
				}

				break;
#endif
			case PT_SERVERCFG:
				break;
			case PT_FILEFRAGMENT:
				if (!server)
					Got_Filetxpak();
				break;
			default:
				DEBFILE(va("UNKNOWN PACKET TYPE RECEIVED %d from host %d\n",
					netbuffer->packettype, node));
		} // end switch
	} // end while
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
// no more use random generator, because at very first tic isn't yet synchronized
// Note: It is called consistAncy on purpose.
//
static INT16 Consistancy(void)
{
	INT16 ret = 0;
	INT32 i;

	DEBFILE(va("TIC %u ", gametic));
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo)
		{
			DEBFILE(va("p[%d].x = %f ", i, (double)FIXED_TO_FLOAT(players[i].mo->x)));
			ret = (INT16)((ret + players[i].mo->x) & 0xFFFF);
		}
	DEBFILE(va("pos = %d, rnd %d\n", ret, P_GetRandIndex()));
	ret = (INT16)(ret + P_GetRandIndex());

	return ret;
}

// send the client packet to the server
static void CL_SendClientCmd(void)
{
	size_t packetsize = 0;

	netbuffer->packettype = PT_CLIENTCMD;

	if (cl_packetmissed)
		netbuffer->packettype++;
	netbuffer->u.clientpak.resendfrom = (UINT8)(neededtic & UINT8_MAX);
	netbuffer->u.clientpak.client_tic = (UINT8)(gametic & UINT8_MAX);

	if (gamestate == GS_WAITINGPLAYERS)
	{
		// send NODEKEEPALIVE packet
		netbuffer->packettype += 4;
		packetsize = sizeof (clientcmd_pak) - sizeof (ticcmd_t) - sizeof (INT16);
		HSendPacket(servernode, false, 0, packetsize);
	}
	else if (gamestate != GS_NULL)
	{
		G_MoveTiccmd(&netbuffer->u.clientpak.cmd, &localcmds, 1);
		netbuffer->u.clientpak.consistancy = SHORT(consistancy[gametic%BACKUPTICS]);

		// send a special packet with 2 cmd for splitscreen
		if (splitscreen)
		{
			netbuffer->packettype += 2;
			G_MoveTiccmd(&netbuffer->u.client2pak.cmd2, &localcmds2, 1);
			packetsize = sizeof (client2cmd_pak);
		}
		else
			packetsize = sizeof (clientcmd_pak);

		HSendPacket(servernode, false, 0, packetsize);
	}

	if (cl_mode == cl_connected || dedicated)
	{
		// send extra data if needed
		if (localtextcmd[0])
		{
			netbuffer->packettype = PT_TEXTCMD;
			M_Memcpy(netbuffer->u.textcmd,localtextcmd, localtextcmd[0]+1);
			// all extra data have been sended
			if (HSendPacket(servernode, true, 0, localtextcmd[0]+1)) // send can fail...
				localtextcmd[0] = 0;
		}

		// send extra data if needed for player 2 (splitscreen)
		if (localtextcmd2[0])
		{
			netbuffer->packettype = PT_TEXTCMD2;
			M_Memcpy(netbuffer->u.textcmd, localtextcmd2, localtextcmd2[0]+1);
			// all extra data have been sended
			if (HSendPacket(servernode, true, 0, localtextcmd2[0]+1)) // send can fail...
				localtextcmd2[0] = 0;
		}
	}
}

// send the server packet
// send tic from firstticstosend to maketic-1
static void SV_SendTics(void)
{
	tic_t realfirsttic, lasttictosend, i;
	UINT32 n;
	INT32 j;
	size_t packsize;
	UINT8 *bufpos;
	UINT8 *ntextcmd;

	// send to all client but not to me
	// for each node create a packet with x tics and send it
	// x is computed using supposedtics[n], max packet size and maketic
	for (n = 1; n < MAXNETNODES; n++)
		if (nodeingame[n])
		{
			lasttictosend = maketic;

			// assert supposedtics[n]>=nettics[n]
			realfirsttic = supposedtics[n];
			if (realfirsttic >= maketic)
			{
				// well we have sent all tics we will so use extrabandwidth
				// to resent packet that are supposed lost (this is necessary since lost
				// packet detection work when we have received packet with firsttic > neededtic
				// (getpacket servertics case)
				DEBFILE(va("Nothing to send node %u mak=%u sup=%u net=%u \n",
					n, maketic, supposedtics[n], nettics[n]));
				realfirsttic = nettics[n];
				if (realfirsttic >= maketic || (I_GetTime() + n)&3)
					// all tic are ok
					continue;
				DEBFILE(va("Sent %d anyway\n", realfirsttic));
			}
			if (realfirsttic < firstticstosend)
				realfirsttic = firstticstosend;

			// compute the length of the packet and cut it if too large
			packsize = BASESERVERTICSSIZE;
			for (i = realfirsttic; i < lasttictosend; i++)
			{
				packsize += sizeof (ticcmd_t) * doomcom->numslots;
				packsize += TotalTextCmdPerTic(i);

				if (packsize > software_MAXPACKETLENGTH)
				{
					DEBFILE(va("packet too large (%"PRIdS") at tic %d (should be from %d to %d)\n",
						packsize, i, realfirsttic, lasttictosend));
					lasttictosend = i;

					// too bad: too much player have send extradata and there is too
					//          much data in one tic.
					// To avoid it put the data on the next tic. (see getpacket
					// textcmd case) but when numplayer changes the computation can be different
					if (lasttictosend == realfirsttic)
					{
						if (packsize > MAXPACKETLENGTH)
							I_Error("Too many players: can't send %"PRIdS" data for %d players to node %d\n"
							        "Well sorry nobody is perfect....\n",
							        packsize, doomcom->numslots, n);
						else
						{
							lasttictosend++; // send it anyway!
							DEBFILE("sending it anyway\n");
						}
					}
					break;
				}
			}

			// Send the tics
			netbuffer->packettype = PT_SERVERTICS;
			netbuffer->u.serverpak.starttic = (UINT8)realfirsttic;
			netbuffer->u.serverpak.numtics = (UINT8)(lasttictosend - realfirsttic);
			netbuffer->u.serverpak.numslots = (UINT8)SHORT(doomcom->numslots);
			bufpos = (UINT8 *)&netbuffer->u.serverpak.cmds;

			for (i = realfirsttic; i < lasttictosend; i++)
			{
				bufpos = G_DcpyTiccmd(bufpos, netcmds[i%BACKUPTICS], doomcom->numslots * sizeof (ticcmd_t));
			}

			// add textcmds
			for (i = realfirsttic; i < lasttictosend; i++)
			{
				ntextcmd = bufpos++;
				*ntextcmd = 0;
				for (j = 0; j < MAXPLAYERS; j++)
				{
					INT32 size = textcmds[i%BACKUPTICS][j][0];

					if ((!j || playeringame[j]) && size)
					{
						(*ntextcmd)++;
						WRITEUINT8(bufpos, j);
						M_Memcpy(bufpos, textcmds[i%BACKUPTICS][j], size + 1);
						bufpos += size + 1;
					}
				}
			}
			packsize = bufpos - (UINT8 *)&(netbuffer->u);

			HSendPacket(n, false, 0, packsize);
			// when tic are too large, only one tic is sent so don't go backward!
			if (lasttictosend-doomcom->extratics > realfirsttic)
				supposedtics[n] = lasttictosend-doomcom->extratics;
			else
				supposedtics[n] = lasttictosend;
			if (supposedtics[n] < nettics[n]) supposedtics[n] = nettics[n];
		}
	// node 0 is me!
	supposedtics[0] = maketic;
}

//
// TryRunTics
//
static void Local_Maketic(INT32 realtics)
{
	I_OsPolling(); // I_Getevent
	D_ProcessEvents(); // menu responder, cons responder,
	                   // game responder calls HU_Responder, AM_Responder, F_Responder,
	                   // and G_MapEventsToControls
	if (!dedicated) rendergametic = gametic;
	// translate inputs (keyboard/mouse/joystick) into game controls
	G_BuildTiccmd(&localcmds, realtics);
	if (splitscreen)
		G_BuildTiccmd2(&localcmds2, realtics);

	localcmds.angleturn |= TICCMD_RECEIVED;
}

void SV_SpawnPlayer(INT32 playernum, INT32 x, INT32 y, angle_t angle)
{
	// for future copytic use the good x, y, and angle!
	if (server)
	{
		x = y = 0;
		netcmds[maketic%BACKUPTICS][playernum].angleturn = (INT16)((INT16)(angle>>16) | TICCMD_RECEIVED);
	}
}

// create missed tic
static void SV_Maketic(void)
{
	INT32 j;

	for (j = 0; j < MAXNETNODES; j++)
		if (playerpernode[j])
		{
			INT32 player = nodetoplayer[j];
			if ((netcmds[maketic%BACKUPTICS][player].angleturn & TICCMD_RECEIVED) == 0)
			{ // we didn't receive this tic
				INT32 i;

				DEBFILE(va("MISS tic%4d for node %d\n", maketic, j));
#if defined(PARANOIA) && 0
				if (devparm)
					I_OutputMsg("Client Misstic %d\n", maketic);
#endif
				// copy the old tic
				for (i = 0; i < playerpernode[j]; i++, player = nodetoplayer2[j])
				{
					netcmds[maketic%BACKUPTICS][player] = netcmds[(maketic-1)%BACKUPTICS][player];
					netcmds[maketic%BACKUPTICS][player].angleturn &= ~TICCMD_RECEIVED;
				}
			}
		}

	// all tic are now proceed make the next
	maketic++;
}

void TryRunTics(tic_t realtics)
{
	// the machine has lagged but it is not so bad
	if (realtics > TICRATE/7) // FIXME: consistency failure!!
	{
		if (server)
			realtics = 1;
		else
			realtics = TICRATE/7;
	}

	if (singletics)
		realtics = 1;

	if (realtics >= 1)
	{
		COM_BufExecute();
		if (mapchangepending)
			D_MapChange(-1, 0, ultimatemode, 0, 2, false, fromlevelselect); // finish the map change
	}

	NetUpdate();

	if (demoplayback)
	{
		neededtic = gametic + realtics + cv_playdemospeed.value;
		// start a game after a demo
		maketic += realtics;
		firstticstosend = maketic;
		tictoclear = firstticstosend;
	}

	GetPackets();

#ifdef DEBUGFILE
	if (debugfile && (realtics || neededtic > gametic))
	{
		//SoM: 3/30/2000: Need long INT32 in the format string for args 4 & 5.
		//Shut up stupid warning!
		fprintf(debugfile, "------------ Tryruntic: REAL:%d NEED:%d GAME:%d LOAD: %d\n",
			realtics, neededtic, gametic, debugload);
		debugload = 100000;
	}
#endif

	if (neededtic > gametic)
	{
		if (advancedemo)
			D_StartTitle();
		else
			// run the count * tics
			while (neededtic > gametic)
			{
				DEBFILE(va("============ Running tic %d (local %d)\n", gametic, localgametic));

				G_Ticker();
				ExtraDataTicker();
				gametic++;
				// skip paused tic in a demo
				if (demoplayback && paused)
					neededtic++;
				else
					consistancy[gametic%BACKUPTICS] = Consistancy();
			}
	}
}

#ifdef NEWPING
static inline void PingUpdate(void)
{
	INT32 i;
	boolean laggers[MAXPLAYERS];
	UINT8 numlaggers = 0;
	memset(laggers, 0, sizeof(boolean) * MAXPLAYERS);

	netbuffer->packettype = PT_PING;

	//check for ping limit breakage.
	if (cv_maxping.value)
	{
		for (i = 1; i < MAXNETNODES; i++)
		{
			if (playeringame[i] && (realpingtable[i] / pingmeasurecount > (unsigned)cv_maxping.value))
			{
				if (players[i].jointime > 30 * TICRATE)
					laggers[i] = true;
				numlaggers++;
			}
		}

		//kick lagging players... unless everyone but the server's ping sucks.
		//in that case, it is probably the server's fault.
		if (numlaggers < D_NumPlayers() - 1)
		{
			for (i = 1; i < MAXNETNODES; i++)
			{
				if (playeringame[i] && laggers[i])
				{
					XBOXSTATIC char buf[2];

					buf[0] = (char)i;
					buf[1] = KICK_MSG_PING_HIGH;
					SendNetXCmd(XD_KICK, &buf, 2);
				}
			}
		}
	}

	//make the ping packet and clear server data for next one
	for (i = 0; i < MAXNETNODES; i++)
	{
		netbuffer->u.pingtable[i] = realpingtable[i] / pingmeasurecount;
		//server takes a snapshot of the real ping for display.
		//otherwise, pings fluctuate a lot and would be odd to look at.
		playerpingtable[i] = realpingtable[i] / pingmeasurecount;
		realpingtable[i] = 0; //Reset each as we go.
	}

	//send out our ping packets
	for (i = 0; i < MAXNETNODES; i++)
		if (playeringame[i])
			HSendPacket(i, true, 0, sizeof(INT32) * MAXPLAYERS);

	pingmeasurecount = 1; //Reset count
}
#endif

void NetUpdate(void)
{
	static tic_t gametime = 0;
	tic_t nowtime;
	INT32 i;
	INT32 realtics;

	nowtime = I_GetTime();
	realtics = nowtime - gametime;

	if (realtics <= 0) // nothing new to update
		return;
	if (realtics > 5)
	{
		if (server)
			realtics = 1;
		else
			realtics = 5;
	}

	gametime = nowtime;

	if (!(gametime % 255) && netgame && server)
	{
#ifdef NEWPING
		PingUpdate();
#endif
	}

#ifdef NEWPING
	if (server)
	{
		// update node latency values so we can take an average later.
		for (i = 0; i < MAXNETNODES; i++)
			if (playeringame[i])
				realpingtable[i] += G_TicsToMilliseconds(GetLag(i));
		pingmeasurecount++;
	}
#endif

	if (!server)
		maketic = neededtic;

	Local_Maketic(realtics); // make local tic, and call menu?

	if (server && !demoplayback)
		CL_SendClientCmd(); // send it
FILESTAMP
	GetPackets(); // get packet from client or from server
FILESTAMP
	// client send the command after a receive of the server
	// the server send before because in single player is beter

	MasterClient_Ticker(); // acking the master server

	if (!server)
		CL_SendClientCmd(); // send tic cmd
	else
	{
		if (!demoplayback)
		{
			INT32 counts;

			firstticstosend = gametic;
			for (i = 0; i < MAXNETNODES; i++)
				if (nodeingame[i] && nettics[i] < firstticstosend)
					firstticstosend = nettics[i];

			// Don't erase tics not acknowledged
			counts = realtics;

			if (maketic + counts >= firstticstosend + BACKUPTICS)
				counts = firstticstosend+BACKUPTICS-maketic-1;

			for (i = 0; i < counts; i++)
				SV_Maketic(); // create missed tics and increment maketic

			for (; tictoclear < firstticstosend; tictoclear++) // clear only when acknoledged
				D_Clearticcmd(tictoclear);                    // clear the maketic the new tic

			SV_SendTics();

			neededtic = maketic; // the server is a client too
		}
	}
	Net_AckTicker();
	M_Ticker();
	CON_Ticker();
	FiletxTicker();
}

/** Returns the number of players playing.
  * \return Number of players. Can be zero if we're running a ::dedicated
  *         server.
  * \author Graue <graue@oceanbase.org>
  */
INT32 D_NumPlayers(void)
{
	INT32 num = 0, ix;
	for (ix = 0; ix < MAXPLAYERS; ix++)
		if (playeringame[ix])
			num++;
	return num;
}

tic_t GetLag(INT32 node)
{
	return gametic - nettics[node];
}
